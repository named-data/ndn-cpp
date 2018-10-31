/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the NAC library https://github.com/named-data/name-based-access-control/blob/new/src/encryptor.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version, with the additional exemption that
 * compiling, linking, and/or using OpenSSL is allowed.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU Lesser General Public License is in the file COPYING.
 */

#include <stdexcept>
#include <sstream>
#include <ndn-cpp/util/logging.hpp>
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include <ndn-cpp/lite/security/rsa-public-key-lite.hpp>
#include <ndn-cpp/lite/encrypt/algo/aes-algorithm-lite.hpp>
#include <ndn-cpp/encrypt/encrypted-content.hpp>
#include <ndn-cpp/encrypt/encryptor-v2.hpp>

using namespace std;
using namespace ndn::func_lib;

INIT_LOGGER("ndn.EncryptorV2");

namespace ndn {

EncryptorV2::Impl::Impl
  (const Name& accessPrefix, const Name& ckPrefix,
   const SigningInfo& ckDataSigningInfo, const EncryptError::OnError& onError,
   Validator* validator, KeyChain* keyChain, Face* face)
: accessPrefix_(accessPrefix), ckPrefix_(ckPrefix),
  ckDataSigningInfo_(ckDataSigningInfo), isKekRetrievalInProgress_(false),
  onError_(onError), keyChain_(keyChain), face_(face),
  kekPendingInterestId_(0)
{
  regenerateCk();

  // Prepare the callbacks. We make a shared_ptr object since it needs to
  // exist after we call expressInterest and return.
  class Callbacks : public ptr_lib::enable_shared_from_this<Callbacks> {
  public:
    Callbacks(EncryptorV2::Impl* parent)
    : parent_(parent)
    {}

    void
    onInterest
      (const ptr_lib::shared_ptr<const Name>& prefix,
       const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
       uint64_t interestFilterId,
       const ptr_lib::shared_ptr<const InterestFilter>& filter)
    {
      ptr_lib::shared_ptr<Data> data = parent_->storage_.find(*interest);
      if (data) {
        _LOG_TRACE("Serving " << data.getName() << " from InMemoryStorage");
        try {
          face.putData(*data);
        } catch (const std::exception& ex) {
          _LOG_ERROR("Error in Face.putData: " << ex.what());
        }
      }
      else {
        _LOG_TRACE("Didn't find CK data for " << interest.getName());
        // TODO: Send NACK?
      }
    }

    void
    onRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix)
    {
      _LOG_ERROR("Failed to register prefix: " << prefix);
    }

    EncryptorV2::Impl* parent_;
  };

  ptr_lib::shared_ptr<Callbacks> callbacks = ptr_lib::make_shared<Callbacks>(this);
  ckRegisteredPrefixId_ = face_->registerPrefix
    (Name(ckPrefix).append(getNAME_COMPONENT_CK()),
     bind(&Callbacks::onInterest, callbacks, _1, _2, _3, _4, _5),
     bind(&Callbacks::onRegisterFailed, callbacks, _1));
}

void
EncryptorV2::Impl::shutdown()
{
  face_->unsetInterestFilter(ckRegisteredPrefixId_);
  if (kekPendingInterestId_ > 0)
    face_->removePendingInterest(kekPendingInterestId_);
}

ptr_lib::shared_ptr<EncryptedContent>
EncryptorV2::Impl::encrypt(const uint8_t* plainData, size_t plainDataLength)
{
  // Generate the initial vector.
  uint8_t initialVector[AES_IV_SIZE];
  ndn_Error error;
  if ((error = CryptoLite::generateRandomBytes
       (initialVector, sizeof(initialVector))))
    throw runtime_error(ndn_getErrorString(error));

  // Add room for the padding.
  ptr_lib::shared_ptr<vector<uint8_t> > encryptedData
    (new vector<uint8_t>(plainDataLength + ndn_AES_BLOCK_LENGTH));
  size_t encryptedDataLength;
  if ((error = AesAlgorithmLite::encrypt256Cbc
       (ckBits_, sizeof(ckBits_), initialVector, sizeof(initialVector),
        plainData, plainDataLength, &encryptedData->front(), encryptedDataLength)))
    throw runtime_error(string("AesAlgorithm: ") + ndn_getErrorString(error));
  encryptedData->resize(encryptedDataLength);

  ptr_lib::shared_ptr<EncryptedContent> content =
    ptr_lib::make_shared<EncryptedContent>();
  content->setInitialVector(Blob(initialVector, sizeof(initialVector)));
  content->setPayload(Blob(encryptedData, false));
  content->setKeyLocatorName(ckName_);

  return content;
}

void
EncryptorV2::Impl::regenerateCk()
{
  // TODO: Ensure that the CK Data packet for the old CK is published when the
  // CK is updated before the KEK is fetched.

  ckName_ = Name(ckPrefix_);
  ckName_.append(getNAME_COMPONENT_CK());
  // The version is the ID of the CK.
  ckName_.appendVersion((uint64_t)ndn_getNowMilliseconds());

  _LOG_TRACE("Generating new CK: " + ckName_.toUri());
  ndn_Error error;
  if ((error = CryptoLite::generateRandomBytes(ckBits_, sizeof(ckBits_))))
    throw runtime_error(ndn_getErrorString(error));

  // One implication: If the CK is updated before the KEK is fetched, then
  // the KDK for the old CK will not be published.
  if (!kekData_)
    retryFetchingKek();
  else
    makeAndPublishCkData(onError_);
}

void
EncryptorV2::Impl::retryFetchingKek()
{
  if (isKekRetrievalInProgress_)
    return;

  _LOG_TRACE("Retrying fetching of the KEK");
  isKekRetrievalInProgress_ = true;

  // Prepare the callbacks. We make a shared_ptr object since it needs to
  // exist after we call expressInterest and return.
  class Callbacks : public ptr_lib::enable_shared_from_this<Callbacks> {
  public:
    Callbacks(EncryptorV2::Impl* parent)
    : parent_(parent)
    {}

    void
    onReady()
    {
      _LOG_TRACE("The KEK was retrieved and published");
      parent_->isKekRetrievalInProgress_ = false;
    }

    void
    onError(EncryptError::ErrorCode errorCode, const string& message)
    {
      _LOG_TRACE("Failed to retrieve KEK: " + message);
      parent_->isKekRetrievalInProgress_ = false;
      parent_->onError_(errorCode, message);
    }

    EncryptorV2::Impl* parent_;
  };

  ptr_lib::shared_ptr<Callbacks> callbacks = ptr_lib::make_shared<Callbacks>(this);
  fetchKekAndPublishCkData
    (bind(&Callbacks::onReady, callbacks),
     bind(&Callbacks::onError, callbacks, _1, _2),
     N_RETRIES);
}

void
EncryptorV2::Impl::fetchKekAndPublishCkData
  (const Face::Callback& onReady, const EncryptError::OnError& onError,
   int nTriesLeft)
{
  _LOG_TRACE("Fetching KEK: " <<
             Name(accessPrefix_).append(getNAME_COMPONENT_KEK()));

  if (kekPendingInterestId_ > 0) {
    onError(EncryptError::ErrorCode::General,
      "fetchKekAndPublishCkData: There is already a kekPendingInterestId_");
    return;
  }

  // Prepare the callbacks. We make a shared_ptr object since it needs to
  // exist after we call expressInterest and return.
  class Callbacks : public ptr_lib::enable_shared_from_this<Callbacks> {
  public:
    Callbacks
      (EncryptorV2::Impl* parent, const Face::Callback& onReady,
       const EncryptError::OnError& onError, int nTriesLeft)
    : parent_(parent), onReady_(onReady), onError_(onError),
      nTriesLeft_(nTriesLeft)
    {}

    void
    onData
      (const ptr_lib::shared_ptr<const Interest>& interest,
       const ptr_lib::shared_ptr<Data>& kekData)
    {
      parent_->kekPendingInterestId_ = 0;
      // TODO: Verify if the key is legitimate.
      parent_->kekData_ = kekData;
      if (parent_->makeAndPublishCkData(onError_))
        onReady_();
      // Otherwise, failure has already been reported.
    }

    void
    onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
    {
      parent_->kekPendingInterestId_ = 0;
      if (nTriesLeft_ > 1)
        parent_->fetchKekAndPublishCkData(onReady_, onError_, nTriesLeft_ - 1);
      else {
        onError_(EncryptError::ErrorCode::KekRetrievalTimeout,
          "Retrieval of KEK [" + interest->getName().toUri() + "] timed out");
        _LOG_TRACE("Scheduling retry after all timeouts");
        parent_->face_->callLater
          (RETRY_DELAY_KEK_RETRIEVAL_MS,
           bind(&EncryptorV2::Impl::retryFetchingKek, parent_));
      }
    }

    void
    onNetworkNack
      (const ptr_lib::shared_ptr<const Interest>& interest,
       const ptr_lib::shared_ptr<NetworkNack>& networkNack)
    {
      parent_->kekPendingInterestId_ = 0;
      if (nTriesLeft_ > 1) {
        parent_->face_->callLater
          (RETRY_DELAY_AFTER_NACK_MS,
           bind(&EncryptorV2::Impl::fetchKekAndPublishCkData, parent_,
                onReady_, onError_, nTriesLeft_ - 1));
      }
      else {
        ostringstream message;
        message <<  "Retrieval of KEK [" << interest->getName().toUri() <<
          "] failed. Got NACK (" << networkNack->getReason() << ")";
        onError_(EncryptError::ErrorCode::KekRetrievalFailure, message.str());
        _LOG_TRACE("Scheduling retry from NACK");
        parent_->face_->callLater
          (RETRY_DELAY_KEK_RETRIEVAL_MS,
           bind(&EncryptorV2::Impl::retryFetchingKek, parent_));
      }
    }

    EncryptorV2::Impl* parent_;
    Face::Callback onReady_;
    EncryptError::OnError onError_;
    int nTriesLeft_;
  };

  try {
    ptr_lib::shared_ptr<Callbacks> callbacks = ptr_lib::make_shared<Callbacks>
      (this, onReady, onError, nTriesLeft);
    kekPendingInterestId_ = face_->expressInterest
      (Interest(Name(accessPrefix_).append(getNAME_COMPONENT_KEK()))
               .setMustBeFresh(true)
               .setCanBePrefix(true),
       bind(&Callbacks::onData, callbacks, _1, _2),
       bind(&Callbacks::onTimeout, callbacks, _1),
       bind(&Callbacks::onNetworkNack, callbacks, _1, _2));
  } catch (const std::exception& ex) {
    onError(EncryptError::ErrorCode::General,
            string("expressInterest error: ") + ex.what());
  }
}

bool
EncryptorV2::Impl::makeAndPublishCkData(const EncryptError::OnError& onError)
{
  try {
    RsaPublicKeyLite kek;
    if (kek.decode(kekData_->getContent()) != NDN_ERROR_success)
      throw runtime_error("RsaAlgorithm: Error decoding public key");

    // TODO: use RSA_size, etc. to get the proper size of the output buffer.
    ptr_lib::shared_ptr<vector<uint8_t> > encryptedData(new vector<uint8_t>(1000));
    size_t encryptedDataLength;
    ndn_Error error;
    if ((error = kek.encrypt
         (ckBits_, sizeof(ckBits_), ndn_EncryptAlgorithmType_RsaOaep,
          &encryptedData->front(), encryptedDataLength)))
      throw runtime_error("RsaAlgorithm: Error encrypting with public key");
    encryptedData->resize(encryptedDataLength);
    EncryptedContent content;
    content.setPayload(Blob(encryptedData, false));

    Data ckData
      (Name(ckName_).append(getNAME_COMPONENT_ENCRYPTED_BY())
       .append(kekData_->getName()));
    ckData.setContent(content.wireEncodeV2());
    // FreshnessPeriod can serve as a soft access control for revoking access.
    ckData.getMetaInfo().setFreshnessPeriod(DEFAULT_CK_FRESHNESS_PERIOD_MS);
    keyChain_->sign(ckData, ckDataSigningInfo_);
    storage_.insert(ckData);

    _LOG_TRACE("Publishing CK data: " << ckData.getName());
    return true;
  } catch (const std::exception& ex) {
    onError(EncryptError::ErrorCode::EncryptionFailure,
      "Failed to encrypt generated CK with KEK " + kekData_->getName().toUri());
    return false;
  }
}

EncryptorV2::Values* EncryptorV2::values_ = 0;

}
