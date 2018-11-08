/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the NAC library https://github.com/named-data/name-based-access-control/blob/new/src/decryptor.cpp
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

#include <sstream>
#include <ndn-cpp/util/logging.hpp>
#include <ndn-cpp/lite/encrypt/algo/aes-algorithm-lite.hpp>
#include <ndn-cpp/encrypt/encryptor-v2.hpp>
#include <ndn-cpp/encrypt/decryptor-v2.hpp>

using namespace std;
using namespace ndn::func_lib;

INIT_LOGGER("ndn.DecryptorV2");

namespace ndn {

DecryptorV2::Impl::Impl
  (PibKey* credentialsKey, Validator* validator, KeyChain* keyChain,
   Face* face)
: credentialsKey_(credentialsKey),
  // validator_(validator),
  face_(face),
  keyChain_(keyChain),
  internalKeyChain_("pib-memory:", "tpm-memory:")
{
}

void
DecryptorV2::Impl::shutdown()
{
  for (size_t iContentKey = 0; iContentKey < contentKeys_.size(); ++iContentKey) {
    ContentKey& contentKey = *contentKeys_[0];

    if (contentKey.pendingInterest > 0) {
      face_->removePendingInterest(contentKey.pendingInterest);
      contentKey.pendingInterest = 0;

      for (size_t iPendingDecrypt = 0; 
           iPendingDecrypt < contentKey.pendingDecrypts.size();
           ++iPendingDecrypt) {
        ContentKey::PendingDecrypt& pendingDecrypt = 
          *contentKey.pendingDecrypts[iPendingDecrypt];
        pendingDecrypt.onError
          (EncryptError::ErrorCode::CkRetrievalFailure,
           "Canceling pending decrypt as ContentKey is being destroyed");
      }

      // Clear is not really necessary, but just in case.
      contentKey.pendingDecrypts.clear();
    }
  }
}

void
DecryptorV2::Impl::decrypt
  (const ptr_lib::shared_ptr<EncryptedContent>& encryptedContent,
   const DecryptSuccessCallback& onSuccess,
   const EncryptError::OnError& onError)
{
  if (encryptedContent->getKeyLocator().getType() != ndn_KeyLocatorType_KEYNAME) {
    _LOG_TRACE
      ("Missing required KeyLocator in the supplied EncryptedContent block");
    onError(EncryptError::ErrorCode::MissingRequiredKeyLocator,
      "Missing required KeyLocator in the supplied EncryptedContent block");
    return;
  }

  if (!encryptedContent->hasInitialVector()) {
    _LOG_TRACE
      ("Missing required initial vector in the supplied EncryptedContent block");
    onError(EncryptError::ErrorCode::MissingRequiredInitialVector,
      "Missing required initial vector in the supplied EncryptedContent block");
    return;
  }

  Name ckName = encryptedContent->getKeyLocatorName();
  ptr_lib::shared_ptr<ContentKey> contentKey;
  bool isNew = (contentKeys_.find(ckName) == contentKeys_.end());
  if (isNew) {
    contentKey = ptr_lib::make_shared<ContentKey>();
    contentKeys_[ckName] = contentKey;
  }
  else
    contentKey = contentKeys_[ckName];

  if (contentKey->isRetrieved)
    doDecrypt(*encryptedContent, contentKey->bits, onSuccess, onError);
  else {
    _LOG_TRACE
      ("CK " << ckName <<
       " not yet available, so adding to the pending decrypt queue");
    contentKey->pendingDecrypts.push_back
      (ptr_lib::make_shared<ContentKey::PendingDecrypt>
       (encryptedContent, onSuccess, onError));
  }

  if (isNew)
    fetchCk(ckName, contentKey, onError, EncryptorV2::N_RETRIES);
}

void
DecryptorV2::Impl::fetchCk
  (const Name& ckName, const ptr_lib::shared_ptr<ContentKey>& contentKey,
   const EncryptError::OnError& onError, int nTriesLeft)
{
  // The full name of the CK is
  //
  // <whatever-prefix>/CK/<ck-id>  /ENCRYPTED-BY /<kek-prefix>/KEK/<key-id>
  // \                          /                \                        /
  //  -----------  -------------                  -----------  -----------
  //             \/                                          \/
  //   from the encrypted data          unknown (name in retrieved CK is used to determine KDK)

  _LOG_TRACE("Fetching CK " << ckName);

  // Prepare the callbacks.
  class Callbacks : public ptr_lib::enable_shared_from_this<Callbacks> {
  public:
    Callbacks
      (const ptr_lib::shared_ptr<Impl>& parent, const Name& ckName,
       const ptr_lib::shared_ptr<ContentKey>& contentKey,
       const EncryptError::OnError& onError, int nTriesLeft)
    : parent_(parent), ckName_(ckName), contentKey_(contentKey),
      onError_(onError), nTriesLeft_(nTriesLeft)
    {}

    void
    onData
      (const ptr_lib::shared_ptr<const Interest>& ckInterest,
       const ptr_lib::shared_ptr<Data>& ckData)
    {
      try {
        contentKey_->pendingInterest = 0;
        // TODO: Verify that the key is legitimate.
        Name kdkPrefix;
        Name kdkIdentityName;
        Name kdkKeyName;
        if (!extractKdkInfoFromCkName
            (ckData->getName(), ckInterest->getName(), onError_, kdkPrefix,
             kdkIdentityName, kdkKeyName))
          // The error has already been reported.
          return;

        // Check if the KDK already exists.
        ptr_lib::shared_ptr<PibIdentity> kdkIdentity;
        try {
          kdkIdentity = parent_->internalKeyChain_.getPib().getIdentity
            (kdkIdentityName);
        } catch (const Pib::Error& ex) {
        }
        if (kdkIdentity) {
          ptr_lib::shared_ptr<PibKey> kdkKey;
          try {
            kdkKey = kdkIdentity->getKey(kdkKeyName);
          } catch (const Pib::Error& ex) {
          }
          if (kdkKey) {
            // The KDK was already fetched and imported.
            _LOG_TRACE("KDK " << kdkKeyName <<
              " already exists, so directly using it to decrypt the CK");
            parent_->decryptCkAndProcessPendingDecrypts
              (*contentKey_, *ckData, kdkKeyName, onError_);
            return;
          }
        }

        parent_->fetchKdk
          (contentKey_, kdkPrefix, ckData, onError_, EncryptorV2::N_RETRIES);
      } catch (const std::exception& ex) {
        onError_(EncryptError::ErrorCode::General,
          string("Error in fetchCk onData: ") + ex.what());
      }
    }

    void
    onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
    {
      contentKey_->pendingInterest = 0;
      if (nTriesLeft_ > 1)
        parent_->fetchCk(ckName_, contentKey_, onError_, nTriesLeft_ - 1);
      else
        onError_(EncryptError::ErrorCode::CkRetrievalTimeout,
          "Retrieval of CK [" + interest->getName().toUri() + "] timed out");
    }

    void
    onNetworkNack
      (const ptr_lib::shared_ptr<const Interest>& interest,
       const ptr_lib::shared_ptr<NetworkNack>& networkNack)
    {
      contentKey_->pendingInterest = 0;
      ostringstream message;
      message << "Retrieval of CK [" << interest->getName().toUri() <<
        "] failed. Got NACK (" << networkNack->getReason() << ")";
      onError_(EncryptError::ErrorCode::CkRetrievalFailure, message.str());
    }

    ptr_lib::shared_ptr<Impl> parent_;
    Name ckName_;
    ptr_lib::shared_ptr<ContentKey> contentKey_;
    EncryptError::OnError onError_;
    int nTriesLeft_;
  };

  try {
    // We make a shared_ptr object since it needs to exist after we return, and
    // pass shared_from_this() to keep a pointer to this Impl.
    ptr_lib::shared_ptr<Callbacks> callbacks = ptr_lib::make_shared<Callbacks>
      (shared_from_this(), ckName, contentKey, onError, nTriesLeft);
    contentKey->pendingInterest = face_->expressInterest
      (Interest(ckName).setMustBeFresh(false).setCanBePrefix(true),
       bind(&Callbacks::onData, callbacks, _1, _2),
       bind(&Callbacks::onTimeout, callbacks, _1),
       bind(&Callbacks::onNetworkNack, callbacks, _1, _2));
  } catch (const std::exception& ex) {
    onError(EncryptError::ErrorCode::General,
            string("expressInterest error: ") + ex.what());
  }
}

void
DecryptorV2::Impl::fetchKdk
  (const ptr_lib::shared_ptr<ContentKey>& contentKey, const Name& kdkPrefix,
   const ptr_lib::shared_ptr<Data>& ckData,
   const EncryptError::OnError& onError, int nTriesLeft)
{
  // <kdk-prefix>/KDK/<kdk-id>    /ENCRYPTED-BY  /<credential-identity>/KEY/<key-id>
  // \                          /                \                                /
  //  -----------  -------------                  ---------------  ---------------
  //             \/                                              \/
  //     from the CK data                                from configuration

  Name kdkName(kdkPrefix);
  kdkName
    .append(EncryptorV2::getNAME_COMPONENT_ENCRYPTED_BY())
    .append(credentialsKey_->getName());

  _LOG_TRACE("Fetching KDK " << kdkName);

  // Prepare the callbacks.
  class Callbacks : public ptr_lib::enable_shared_from_this<Callbacks> {
  public:
    Callbacks
      (const ptr_lib::shared_ptr<Impl>& parent,
       const ptr_lib::shared_ptr<ContentKey>& contentKey, const Name& kdkPrefix,
       const ptr_lib::shared_ptr<Data>& ckData,
       const EncryptError::OnError& onError, int nTriesLeft)
    : parent_(parent), contentKey_(contentKey), kdkPrefix_(kdkPrefix),
      ckData_(ckData), onError_(onError), nTriesLeft_(nTriesLeft)
    {}

    void
    onData
      (const ptr_lib::shared_ptr<const Interest>& interest,
       const ptr_lib::shared_ptr<Data>& kdkData)
    {
      contentKey_->pendingInterest = 0;
      // TODO: Verify that the key is legitimate.

      bool isOk = parent_->decryptAndImportKdk(*kdkData, onError_);
      if (!isOk)
        return;
      // This way of getting the kdkKeyName is a bit hacky.
      Name kdkKeyName = kdkPrefix_.getPrefix(-2)
        .append("KEY").append(kdkPrefix_.get(-1));
      parent_->decryptCkAndProcessPendingDecrypts
        (*contentKey_, *ckData_, kdkKeyName, onError_);
    }

    void
    onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
    {
      contentKey_->pendingInterest = 0;
      if (nTriesLeft_ > 1)
        parent_->fetchKdk
          (contentKey_, kdkPrefix_, ckData_, onError_, nTriesLeft_ - 1);
      else
        onError_(EncryptError::ErrorCode::KdkRetrievalTimeout,
          "Retrieval of KDK [" + interest->getName().toUri() + "] timed out");
    }

    void
    onNetworkNack
      (const ptr_lib::shared_ptr<const Interest>& interest,
       const ptr_lib::shared_ptr<NetworkNack>& networkNack)
    {
      contentKey_->pendingInterest = 0;
      ostringstream message;
      message << "Retrieval of KDK [" << interest->getName().toUri() <<
        "] failed. Got NACK (" << networkNack->getReason() << ")";
      onError_(EncryptError::ErrorCode::KdkRetrievalFailure, message.str());
    }

    ptr_lib::shared_ptr<Impl> parent_;
    ptr_lib::shared_ptr<ContentKey> contentKey_;
    Name kdkPrefix_;
    ptr_lib::shared_ptr<Data> ckData_;
    EncryptError::OnError onError_;
    int nTriesLeft_;
  };

  try {
    // We make a shared_ptr object since it needs to exist after we return, and
    // pass shared_from_this() to keep a pointer to this Impl.
    ptr_lib::shared_ptr<Callbacks> callbacks = ptr_lib::make_shared<Callbacks>
      (shared_from_this(), contentKey, kdkPrefix, ckData, onError, nTriesLeft);
    contentKey->pendingInterest = face_->expressInterest
      (Interest(kdkName).setMustBeFresh(true).setCanBePrefix(false),
       bind(&Callbacks::onData, callbacks, _1, _2),
       bind(&Callbacks::onTimeout, callbacks, _1),
       bind(&Callbacks::onNetworkNack, callbacks, _1, _2));
  } catch (const std::exception& ex) {
    onError(EncryptError::ErrorCode::General,
            string("expressInterest error: ") + ex.what());
  }
}

bool
DecryptorV2::Impl::decryptAndImportKdk
  (const Data& kdkData, const EncryptError::OnError& onError)
{
  try {
    _LOG_TRACE("Decrypting and importing KDK " << kdkData.getName());
    EncryptedContent encryptedContent;
    encryptedContent.wireDecodeV2(kdkData.getContent());

    SafeBag safeBag(encryptedContent.getPayload());
    Blob secret = keyChain_->getTpm().decrypt
      (encryptedContent.getPayloadKey().buf(),
       encryptedContent.getPayloadKey().size(), credentialsKey_->getName());
    if (secret.isNull()) {
      onError(EncryptError::ErrorCode::TpmKeyNotFound,
         "Could not decrypt secret, " + credentialsKey_->getName().toUri() +
         " not found in TPM");
      return false;
    }

    internalKeyChain_.importSafeBag(safeBag, secret.buf(), secret.size());
    return true;
  } catch (const std::exception& ex) {
    // This can be EncodingException, Pib.Error, Tpm.Error, or a bunch of
    // other runtime-derived errors.
    onError(EncryptError::ErrorCode::DecryptionFailure,
       "Failed to decrypt KDK [" + kdkData.getName().toUri() + "]: " + ex.what());
    return false;
  }
}

void
DecryptorV2::Impl::decryptCkAndProcessPendingDecrypts
  (ContentKey& contentKey, const Data& ckData, const Name& kdkKeyName,
   const EncryptError::OnError& onError)
{
  _LOG_TRACE("Decrypting CK data " << ckData.getName());

  EncryptedContent content;
  try {
    content.wireDecodeV2(ckData.getContent());
  } catch (const std::exception& ex) {
    onError(EncryptError::ErrorCode::InvalidEncryptedFormat,
      string("Error decrypting EncryptedContent: ") + ex.what());
    return;
  }

  Blob ckBits;
  try {
    ckBits = internalKeyChain_.getTpm().decrypt
      (content.getPayload().buf(), content.getPayload().size(), kdkKeyName);
  } catch (const std::exception& ex) {
    // We don't expect this from the in-memory KeyChain.
    onError(EncryptError::ErrorCode::DecryptionFailure,
      string("Error decrypting the CK EncryptedContent ") + ex.what());
    return;
  }

  if (ckBits.isNull()) {
    onError(EncryptError::ErrorCode::TpmKeyNotFound,
      "Could not decrypt secret, " + kdkKeyName.toUri() + " not found in TPM");
    return;
  }

  contentKey.bits = ckBits;
  contentKey.isRetrieved = true;

  for (size_t i = 0; i < contentKey.pendingDecrypts.size(); ++i) {
    ContentKey::PendingDecrypt& pendingDecrypt = *contentKey.pendingDecrypts[i];
    // TODO: If this calls onError, should we quit?
    doDecrypt
      (*pendingDecrypt.encryptedContent, contentKey.bits,
       pendingDecrypt.onSuccess, pendingDecrypt.onError);
  }

  contentKey.pendingDecrypts.clear();
}

void
DecryptorV2::Impl::doDecrypt
  (const EncryptedContent& content, const Blob& ckBits,
   const DecryptSuccessCallback& onSuccess,
   const EncryptError::OnError& onError)
{
  if (!content.hasInitialVector()) {
    onError(EncryptError::ErrorCode::MissingRequiredInitialVector,
      "Expecting Initial Vector in the encrypted content, but it is not present");
    return;
  }

  ndn_Error error;
  ptr_lib::shared_ptr<vector<uint8_t> > plainData
    (new vector<uint8_t>(content.getPayload().size()));
  size_t plainDataLength;
  if ((error = AesAlgorithmLite::decrypt256Cbc
       (ckBits, content.getInitialVector(), content.getPayload(),
        &plainData->front(), plainDataLength))) {
    onError(EncryptError::ErrorCode::DecryptionFailure,
      "Decryption error in doDecrypt: " + string(ndn_getErrorString(error)));
    return;
  }
  plainData->resize(plainDataLength);

  try {
    onSuccess(Blob(plainData, false));
  } catch (const std::exception& ex) {
    _LOG_ERROR("Error in onSuccess: " << ex.what());
  } catch (...) {
    _LOG_ERROR("Error in onSuccess.");
  }
}

Name
DecryptorV2::Impl::convertKekNameToKdkPrefix
  (const Name& kekName, const EncryptError::OnError& onError)
{
  if (kekName.size() < 2 ||
      !kekName.get(-2).equals(EncryptorV2::getNAME_COMPONENT_KEK())) {
    onError(EncryptError::ErrorCode::KekInvalidName,
      "Invalid KEK name [" + kekName.toUri() + "]");
    return Name();
  }

  return kekName.getPrefix(-2)
    .append(EncryptorV2::getNAME_COMPONENT_KDK()).append(kekName.get(-1));
}

bool
DecryptorV2::Impl::extractKdkInfoFromCkName
  (const Name& ckDataName, const Name& ckName,
   const EncryptError::OnError& onError, Name& kdkPrefix,
   Name& kdkIdentityName, Name& kdkKeyId)
{
  // <full-ck-name-with-id> | /ENCRYPTED-BY/<kek-prefix>/NAC/KEK/<key-id>

  if (ckDataName.size() < ckName.size() + 1 ||
      !ckDataName.getPrefix(ckName.size()).equals(ckName) ||
      !ckDataName.get(ckName.size()).equals
        (EncryptorV2::getNAME_COMPONENT_ENCRYPTED_BY())) {
    onError(EncryptError::ErrorCode::CkInvalidName,
      "Invalid CK name [" + ckDataName.toUri() + "]");
    return false;
  }

  Name kekName = ckDataName.getSubName(ckName.size() + 1);
  kdkPrefix = convertKekNameToKdkPrefix(kekName, onError);
  if (kdkPrefix.size() == 0)
    // The error has already been reported.
    return false;

  kdkIdentityName = kekName.getPrefix(-2);
  kdkKeyId = kekName.getPrefix(-2).append("KEY").append(kekName.get(-1));
  return true;
}

}
