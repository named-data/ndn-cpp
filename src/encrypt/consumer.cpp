/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/consumer https://github.com/named-data/ndn-group-encrypt
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
#include <ndn-cpp/util/logging.hpp>
#include <ndn-cpp/encrypt/algo/aes-algorithm.hpp>
#include <ndn-cpp/encrypt/algo/rsa-algorithm.hpp>
#include <ndn-cpp/encrypt/algo/encryptor.hpp>
#include <ndn-cpp/encrypt/consumer.hpp>

using namespace std;
using namespace ndn::func_lib;

INIT_LOGGER("ndn.Consumer");

namespace ndn {

Consumer::Impl::Impl
  (Face* face, KeyChain* keyChain, const Name& groupName,
   const Name& consumerName, const ptr_lib::shared_ptr<ConsumerDb>& database,
   const Link& cKeyLink, const Link& dKeyLink)
: face_(face),
  keyChain_(keyChain),
  groupName_(groupName),
  consumerName_(consumerName),
  database_(database),
  cKeyLink_(new Link(cKeyLink)),
  dKeyLink_(new Link(dKeyLink))
{
}

void
Consumer::Impl::consume
  (const Name& contentName, const OnConsumeComplete& onConsumeComplete,
   const EncryptError::OnError& onError, const Link& link)
{
  ptr_lib::shared_ptr<const Interest> interest(new Interest(contentName));

  // Prepare the callbacks. We make a shared_ptr object since it needs to
  // exist after we call expressInterest and return.
  class Callbacks : public ptr_lib::enable_shared_from_this<Callbacks> {
  public:
    Callbacks
      (Consumer::Impl* parent, const OnConsumeComplete& onConsumeComplete,
       const EncryptError::OnError& onError)
    : parent_(parent), onConsumeComplete_(onConsumeComplete), onError_(onError)
    {}

    void
    onContentVerified(const ptr_lib::shared_ptr<Data>& validData)
    {
      // Save this for calling onConsumeComplete.
      contentData_ = validData;

      parent_->decryptContent
        (*validData,
         bind(&Callbacks::onContentPlainText, shared_from_this(), _1),
         onError_);
    }

    void
    onContentPlainText(const Blob& plainText)
    {
      try {
        onConsumeComplete_(contentData_, plainText);
      } catch (const std::exception& ex) {
        _LOG_ERROR("Error in onConsumeComplete: " << ex.what());
      } catch (...) {
        _LOG_ERROR("Error in onConsumeComplete.");
      }
    }

    Consumer::Impl* parent_;
    OnConsumeComplete onConsumeComplete_;
    EncryptError::OnError onError_;
    ptr_lib::shared_ptr<Data> contentData_;
  };

  ptr_lib::shared_ptr<Callbacks> callbacks(new Callbacks
    (this, onConsumeComplete, onError));
  // Copy the Link object since the passed link may become invalid.
  sendInterest
    (interest, 1, ptr_lib::make_shared<Link>(link),
     bind(&Callbacks::onContentVerified, callbacks, _1), onError);
}

void
Consumer::Impl::addDecryptionKey(const Name& keyName, const Blob& keyBlob)
{
  if (!(consumerName_.match(keyName)))
    throw runtime_error
      ("addDecryptionKey: The consumer name must be a prefix of the key name");

  database_->addKey(keyName, keyBlob);
}

void
Consumer::Impl::decrypt
  (const Blob& encryptedBlob, const Blob& keyBits,
   const OnPlainText& onPlainText, const EncryptError::OnError& onError)
{
  EncryptedContent encryptedContent;
  try {
    encryptedContent.wireDecode(encryptedBlob);
  } catch (const std::exception& ex) {
    try {
      onError(EncryptError::ErrorCode::InvalidEncryptedFormat, ex.what());
    } catch (const std::exception& ex) {
      _LOG_ERROR("Error in onError: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Error in onError.");
    }
    return;
  }

  decryptEncryptedContent(encryptedContent, keyBits, onPlainText, onError);
}

void
Consumer::Impl::decryptEncryptedContent
  (const EncryptedContent& encryptedContent, const Blob& keyBits,
   const OnPlainText& onPlainText, const EncryptError::OnError& onError)
{
  Blob payload = encryptedContent.getPayload();

  if (encryptedContent.getAlgorithmType() == ndn_EncryptAlgorithmType_AesCbc) {
    // Prepare the parameters.
    EncryptParams decryptParams(ndn_EncryptAlgorithmType_AesCbc);
    decryptParams.setInitialVector(encryptedContent.getInitialVector());

    // Decrypt the content.
    Blob content;
    try {
      content = AesAlgorithm::decrypt(keyBits, payload, decryptParams);
    } catch (const std::exception& ex) {
      try {
        onError(EncryptError::ErrorCode::InvalidEncryptedFormat, ex.what());
      } catch (const std::exception& ex) {
        _LOG_ERROR("Error in onError: " << ex.what());
      } catch (...) {
        _LOG_ERROR("Error in onError.");
      }
      return;
    }
    try {
      onPlainText(content);
    } catch (const std::exception& ex) {
      _LOG_ERROR("Error in onPlainText: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Error in onPlainText.");
    }
  }
  else if (encryptedContent.getAlgorithmType() == ndn_EncryptAlgorithmType_RsaOaep) {
    // Prepare the parameters.
    EncryptParams decryptParams(ndn_EncryptAlgorithmType_RsaOaep);

    // Decrypt the content.
    Blob content;
    try {
      content = RsaAlgorithm::decrypt(keyBits, payload, decryptParams);
    } catch (const std::exception& ex) {
      try {
        onError(EncryptError::ErrorCode::InvalidEncryptedFormat, ex.what());
      } catch (const std::exception& ex) {
        _LOG_ERROR("Error in onError: " << ex.what());
      } catch (...) {
        _LOG_ERROR("Error in onError.");
      }
      return;
    }
    try {
      onPlainText(content);
    } catch (const std::exception& ex) {
      _LOG_ERROR("Error in onPlainText: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Error in onPlainText.");
    }
  }
  else {
    try {
      onError
        (EncryptError::ErrorCode::UnsupportedEncryptionScheme,
         "UnsupportedEncryptionScheme");
    } catch (const std::exception& ex) {
      _LOG_ERROR("Error in onError: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Error in onError.");
    }
  }
}

void
Consumer::Impl::decryptContent
  (const Data& data, const OnPlainText& onPlainText,
   const EncryptError::OnError& onError)
{
  // Get the encrypted content.
  // Make this a shared_ptr so we can pass it in callbacks.
  ptr_lib::shared_ptr<EncryptedContent> dataEncryptedContent
    (new EncryptedContent());
  try {
    dataEncryptedContent->wireDecode(data.getContent());
  } catch (const std::exception& ex) {
    try {
      onError(EncryptError::ErrorCode::InvalidEncryptedFormat, ex.what());
    } catch (const std::exception& ex) {
      _LOG_ERROR("Error in onError: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Error in onError.");
    }
    return;
  }
  const Name& cKeyName = dataEncryptedContent->getKeyLocator().getKeyName();

  // Check if the content key is already in the store.
  if (cKeyMap_.find(cKeyName) != cKeyMap_.end())
    decryptEncryptedContent
      (*dataEncryptedContent, cKeyMap_[cKeyName], onPlainText, onError);
  else {
    // Retrieve the C-KEY Data from the network.
    Name interestName(cKeyName);
    interestName.append(Encryptor::getNAME_COMPONENT_FOR()).append(groupName_);
    ptr_lib::shared_ptr<const Interest> interest(new Interest(interestName));

    // Prepare the callbacks. We make a shared_ptr object since it needs to
    // exist after we call expressInterest and return.
    class Callbacks : public ptr_lib::enable_shared_from_this<Callbacks> {
    public:
      Callbacks
        (Consumer::Impl* parent,
         const ptr_lib::shared_ptr<EncryptedContent>& dataEncryptedContent,
         const Name& cKeyName, const OnPlainText& onPlainText,
         const EncryptError::OnError& onError)
      : parent_(parent), dataEncryptedContent_(dataEncryptedContent),
        cKeyName_(cKeyName), onPlainText_(onPlainText), onError_(onError)
      {}

      void
      onCKeyVerified(const ptr_lib::shared_ptr<Data>& validCKeyData)
      {
        parent_->decryptCKey
          (*validCKeyData,
           bind(&Callbacks::onCKeyPlainText, shared_from_this(), _1),
           onError_);
      }

      void
      onCKeyPlainText(const Blob& cKeyBits)
      {
        parent_->cKeyMap_[cKeyName_] = cKeyBits;
        parent_->decryptEncryptedContent
          (*dataEncryptedContent_, cKeyBits, onPlainText_, onError_);
      }

      Consumer::Impl* parent_;
      ptr_lib::shared_ptr<EncryptedContent> dataEncryptedContent_;
      Name cKeyName_;
      OnPlainText onPlainText_;
      EncryptError::OnError onError_;
    };

    ptr_lib::shared_ptr<Callbacks> callbacks(new Callbacks
      (this, dataEncryptedContent, cKeyName, onPlainText, onError));
    sendInterest
      (interest, 1, cKeyLink_, bind(&Callbacks::onCKeyVerified, callbacks, _1),
       onError);
  }
}

void
Consumer::Impl::decryptCKey
  (const Data& cKeyData, const OnPlainText& onPlainText, 
   const EncryptError::OnError& onError)
{
  // Get the encrypted content.
  Blob cKeyContent = cKeyData.getContent();
  // Make this a shared_ptr so we can pass it in callbacks.
  ptr_lib::shared_ptr<EncryptedContent> cKeyEncryptedContent
    (new EncryptedContent());
  try {
    cKeyEncryptedContent->wireDecode(cKeyContent);
  } catch (const std::exception& ex) {
    try {
      onError(EncryptError::ErrorCode::InvalidEncryptedFormat, ex.what());
    } catch (const std::exception& ex) {
      _LOG_ERROR("Error in onError: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Error in onError.");
    }
    return;
  }
  const Name& eKeyName = cKeyEncryptedContent->getKeyLocator().getKeyName();
  Name dKeyName = eKeyName.getPrefix(-3);
  dKeyName.append(Encryptor::getNAME_COMPONENT_D_KEY())
    .append(eKeyName.getSubName(-2));

  // Check if the decryption key is already in the store.
  if (dKeyMap_.find(dKeyName) != dKeyMap_.end())
    decryptEncryptedContent
      (*cKeyEncryptedContent, dKeyMap_[dKeyName], onPlainText, onError);
  else {
    // Get the D-Key Data.
    Name interestName(dKeyName);
    interestName.append(Encryptor::getNAME_COMPONENT_FOR()).append(consumerName_);
    ptr_lib::shared_ptr<const Interest> interest(new Interest(interestName));

    // Prepare the callbacks. We make a shared_ptr object since it needs to
    // exist after we call expressInterest and return.
    class Callbacks : public ptr_lib::enable_shared_from_this<Callbacks> {
    public:
      Callbacks
        (Consumer::Impl* parent, 
         const ptr_lib::shared_ptr<EncryptedContent>& cKeyEncryptedContent,
         const Name& dKeyName, const OnPlainText& onPlainText,
         const EncryptError::OnError& onError)
      : parent_(parent), cKeyEncryptedContent_(cKeyEncryptedContent),
        dKeyName_(dKeyName), onPlainText_(onPlainText), onError_(onError)
      {}

      void
      onDKeyVerified(const ptr_lib::shared_ptr<Data>& validDKeyData)
      {
        parent_->decryptDKey
          (*validDKeyData,
           bind(&Callbacks::onDKeyPlainText, shared_from_this(), _1),
           onError_);
      }

      void
      onDKeyPlainText(const Blob& dKeyBits)
      {
        parent_->dKeyMap_[dKeyName_] = dKeyBits;
        parent_->decryptEncryptedContent
          (*cKeyEncryptedContent_, dKeyBits, onPlainText_, onError_);
      }

      Consumer::Impl* parent_;
      ptr_lib::shared_ptr<EncryptedContent> cKeyEncryptedContent_;
      Name dKeyName_;
      OnPlainText onPlainText_;
      EncryptError::OnError onError_;
    };

    ptr_lib::shared_ptr<Callbacks> callbacks(new Callbacks
      (this, cKeyEncryptedContent, dKeyName, onPlainText, onError));
    sendInterest
      (interest, 1, dKeyLink_, bind(&Callbacks::onDKeyVerified, callbacks, _1),
       onError);
  }
}

void
Consumer::Impl::decryptDKey
  (const Data& dKeyData, const OnPlainText& onPlainText,
   const EncryptError::OnError& onError)
{
  // Get the encrypted content.
  Blob dataContent = dKeyData.getContent();

  // Process the nonce.
  // dataContent is a sequence of the two EncryptedContent.
  EncryptedContent encryptedNonce;
  try {
    encryptedNonce.wireDecode(dataContent);
  } catch (const std::exception& ex) {
    try {
      onError(EncryptError::ErrorCode::InvalidEncryptedFormat, ex.what());
    } catch (const std::exception& ex) {
      _LOG_ERROR("Error in onError: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Error in onError.");
    }
    return;
  }
  Name consumerKeyName = encryptedNonce.getKeyLocator().getKeyName();

  // Get consumer decryption key.
  Blob consumerKeyBlob;
  try {
    consumerKeyBlob = getDecryptionKey(consumerKeyName);
  } catch (const std::exception& ex) {
    try {
      onError(EncryptError::ErrorCode::NoDecryptKey, ex.what());
    } catch (const std::exception& ex) {
      _LOG_ERROR("Error in onError: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Error in onError.");
    }
    return;
  }
  if (consumerKeyBlob.size() == 0) {
    try {
      onError(EncryptError::ErrorCode::NoDecryptKey,
        "The desired consumer decryption key in not in the database");
    } catch (const std::exception& ex) {
      _LOG_ERROR("Error in onError: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Error in onError.");
    }
    return;
  }

  // Process the D-KEY.
  // Use the size of encryptedNonce to find the start of encryptedPayload.
  size_t encryptedNonceSize = encryptedNonce.wireEncode().size();
  EncryptedContent encryptedPayload;
  Blob encryptedPayloadBlob
    (dataContent.buf() + encryptedNonceSize,
     dataContent.size() - encryptedNonceSize);
  if (encryptedPayloadBlob.size() == 0) {
    try {
      onError(EncryptError::ErrorCode::InvalidEncryptedFormat,
        "The data packet does not satisfy the D-KEY packet format");
    } catch (const std::exception& ex) {
      _LOG_ERROR("Error in onError: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Error in onError.");
    }
    return;
  }

  // Decrypt the D-KEY.
  decryptEncryptedContent
    (encryptedNonce, consumerKeyBlob,
     bind(&Consumer::Impl::decrypt, encryptedPayloadBlob, _1, onPlainText, onError),
     onError);
}

void
Consumer::Impl::sendInterest
  (const ptr_lib::shared_ptr<const Interest>& interest, int nRetrials,
   const ptr_lib::shared_ptr<Link>& link, const OnVerified& onVerified,
   const EncryptError::OnError& onError)
{
  // Prepare the callbacks. We make a shared_ptr object since it needs to
  // exist after we call expressInterest and return.
  class Callbacks : public ptr_lib::enable_shared_from_this<Callbacks> {
  public:
    Callbacks
      (Consumer::Impl* parent, int nRetrials,
       const ptr_lib::shared_ptr<Link>& link, const OnVerified& onVerified,
       const EncryptError::OnError& onError)
    : parent_(parent), nRetrials_(nRetrials), link_(link),
      onVerified_(onVerified), onError_(onError)
    {}

    void
    onData
      (const ptr_lib::shared_ptr<const Interest>& contentInterest,
       const ptr_lib::shared_ptr<Data>& contentData)
    {
      // The Interest has no selectors, so assume the library correctly
      // matched with the Data name before calling onData.

      try {
        parent_->keyChain_->verifyData
          (contentData, onVerified_,
           // Cast to disambiguate from the deprecated OnVerifyFailed.
           (const OnDataValidationFailed)bind
             (&Impl::onValidationFailed, _1, _2, onError_));
      } catch (const std::exception& ex) {
        try {
          onError_(EncryptError::ErrorCode::General,
                  string("verifyData error: ") + ex.what());
        } catch (const std::exception& ex) {
          _LOG_ERROR("Error in onError: " << ex.what());
        } catch (...) {
          _LOG_ERROR("Error in onError.");
        }
      }
    }

    void
    onNetworkNack
      (const ptr_lib::shared_ptr<const Interest>& interest,
       const ptr_lib::shared_ptr<NetworkNack>& networkNack)
    {
      // We have run out of options. Report a retrieval failure.
      try {
        onError_(EncryptError::ErrorCode::DataRetrievalFailure,
                 interest->getName().toUri());
      } catch (const std::exception& ex) {
        _LOG_ERROR("Error in onError: " << ex.what());
      } catch (...) {
        _LOG_ERROR("Error in onError.");
      }
    }

    void
    onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
    {
      if (nRetrials_ > 0)
        parent_->sendInterest
          (interest, nRetrials_ - 1, link_, onVerified_, onError_);
      else
        onNetworkNack(interest, ptr_lib::make_shared<NetworkNack>());
    }

    Consumer::Impl* parent_;
    int nRetrials_;
    const ptr_lib::shared_ptr<Link> link_;
    const OnVerified onVerified_;
    EncryptError::OnError onError_;
  };

  ptr_lib::shared_ptr<Interest> interestWithLink;
  const Interest* request;
  if (link->getDelegations().size() == 0)
    // We can use the supplied interest without copying.
    request = interest.get();
  else {
    // Copy the supplied interest and add the Link.
    interestWithLink.reset(new Interest(*interest));
    // This will use a cached encoding if available.
    interestWithLink->setLinkWireEncoding(link->wireEncode());

    request = interestWithLink.get();
  }

  ptr_lib::shared_ptr<Callbacks> callbacks(new Callbacks
    (this, nRetrials, link, onVerified, onError));
  try {
    face_->expressInterest
      (*request, bind(&Callbacks::onData, callbacks, _1, _2),
       bind(&Callbacks::onTimeout, callbacks, _1),
       bind(&Callbacks::onNetworkNack, callbacks, _1, _2));
  } catch (const std::exception& ex) {
    try {
      onError(EncryptError::ErrorCode::General,
              string("expressInterest error: ") + ex.what());
    } catch (const std::exception& ex) {
      _LOG_ERROR("Error in onError: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Error in onError.");
    }
    return;
  }
}

void
Consumer::Impl::onValidationFailed
  (const ptr_lib::shared_ptr<Data>& data, const string& reason,
   const EncryptError::OnError& onError)
{
  try {
    onError
      (EncryptError::ErrorCode::Validation,
       "verifyData failed. Reason: " + reason);
  } catch (const std::exception& ex) {
    _LOG_ERROR("Error in onError: " << ex.what());
  } catch (...) {
    _LOG_ERROR("Error in onError.");
  }
}

Link* Consumer::noLink_ = 0;

}
