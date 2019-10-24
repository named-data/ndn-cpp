/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2018-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the NAC library https://github.com/named-data/name-based-access-control/blob/new/src/encryptor.hpp
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

#ifndef NDN_ENCRYPTOR_V2_HPP
#define NDN_ENCRYPTOR_V2_HPP

#include "../security/key-chain.hpp"
#include "../security/v2/validator.hpp"
#include "../in-memory-storage/in-memory-storage-retaining.hpp"
#include "encrypted-content.hpp"
#include "encrypt-error.hpp"

// Give friend access to the tests.
class TestEncryptorV2_EncryptAndPublishCk_Test;
class TestEncryptorV2_EnumerateDataFromInMemoryStorage_Test;

namespace ndn {

/**
 * EncryptorV2 encrypts the requested content for name-based access control (NAC)
 * using security v2. For the meaning of "KEK", etc. see:
 * https://github.com/named-data/name-based-access-control/blob/new/docs/spec.rst
 */
class EncryptorV2 {
public:
  /**
   * Create an EncryptorV2 with the given parameters. This uses the face to
   * register to receive Interests for the prefix {ckPrefix}/CK.
   * @param accessPrefix The NAC prefix to fetch the Key Encryption Key (KEK)
   * (e.g., /access/prefix/NAC/data/subset). This copies the Name.
   * @param ckPrefix The prefix under which Content Keys (CK) will be generated.
   * (Each will have a unique version appended.) This copies the Name.
   * @param ckDataSigningInfo The SigningInfo parameters to sign the Content Key
   * (CK) Data packet. This copies the SigningInfo.
   * @param onError On failure to create the CK data (failed to fetch the KEK,
   * failed to encrypt with the KEK, etc.), this calls
   * onError(errorCode, message) where errorCode is from the
   * EncryptError::ErrorCode enum, and message is an error string. The encrypt
   * method will continue trying to retrieve the KEK until success (with each
   * attempt separated by RETRY_DELAY_KEK_RETRIEVAL_MS) and onError may be
   * called multiple times.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param validator The validation policy to ensure correctness of the KEK.
   * @param keyChain The KeyChain used to sign Data packets.
   * @param face The Face that will be used to fetch the KEK and publish CK data.
   */
  EncryptorV2
    (const Name& accessPrefix, const Name& ckPrefix,
     const SigningInfo& ckDataSigningInfo, const EncryptError::OnError& onError,
     Validator* validator, KeyChain* keyChain, Face* face)
  : impl_(new Impl
          (accessPrefix, ckPrefix, ckDataSigningInfo, onError, validator, 
           keyChain, face))
  {
    impl_->initialize();
  }

  void
  shutdown() { impl_->shutdown(); }

  /**
   * Encrypt the plainData using the existing Content Key (CK) and return a new
   * EncryptedContent.
   * @param plainData The data to encrypt.
   * @param plainDataLength The length of plainData.
   * @return The new EncryptedContent.
   */
  ptr_lib::shared_ptr<EncryptedContent>
  encrypt(const uint8_t* plainData, size_t plainDataLength)
  {
    return impl_->encrypt(plainData, plainDataLength);
  }

  /**
   * Encrypt the plainData using the existing Content Key (CK) and return a new
   * EncryptedContent.
   * @param plainData The data to encrypt.
   * @return The new EncryptedContent.
   */
  ptr_lib::shared_ptr<EncryptedContent>
  encrypt(const Blob& plainData)
  {
    return encrypt(plainData.buf(), plainData.size());
  }

  /**
   * Create a new Content Key (CK) and publish the corresponding CK Data packet.
   * This uses the onError given to the constructor to report errors.
   */
  void
  regenerateCk() { impl_->regenerateCk(); }

  /**
   * Get the number of packets stored in in-memory storage.
   * @return The number of packets.
   */
  size_t
  size() { return impl_->size(); }

  static const Name::Component&
  getNAME_COMPONENT_ENCRYPTED_BY() { return getValues().NAME_COMPONENT_ENCRYPTED_BY; }

  static const Name::Component&
  getNAME_COMPONENT_NAC() { return getValues().NAME_COMPONENT_NAC; }

  static const Name::Component&
  getNAME_COMPONENT_KEK() { return getValues().NAME_COMPONENT_KEK; }

  static const Name::Component&
  getNAME_COMPONENT_KDK() { return getValues().NAME_COMPONENT_KDK; }

  static const Name::Component&
  getNAME_COMPONENT_CK() { return getValues().NAME_COMPONENT_CK; }

  static const uint64_t RETRY_DELAY_AFTER_NACK_MS = 1000;
  static const uint64_t RETRY_DELAY_KEK_RETRIEVAL_MS = 60 * 1000;

  static const int AES_KEY_SIZE = 32;
  static const int AES_IV_SIZE = 16;
  static const int N_RETRIES = 3;

private:
  // Give friend access to the tests.
  friend class ::TestEncryptorV2_EncryptAndPublishCk_Test;
  friend class ::TestEncryptorV2_EnumerateDataFromInMemoryStorage_Test;

  /**
   * Values holds values used by the static member values_.
   */
  class Values {
  public:
    Values()
    : NAME_COMPONENT_ENCRYPTED_BY("ENCRYPTED-BY"),
      NAME_COMPONENT_NAC("NAC"),
      NAME_COMPONENT_KEK("KEK"),
      NAME_COMPONENT_KDK("KDK"),
      NAME_COMPONENT_CK("CK")
    {}

    Name::Component NAME_COMPONENT_ENCRYPTED_BY;
    Name::Component NAME_COMPONENT_NAC;
    Name::Component NAME_COMPONENT_KEK;
    Name::Component NAME_COMPONENT_KDK;
    Name::Component NAME_COMPONENT_CK;
  };

  /**
   * Get the static Values object, creating it if needed. We do this explicitly
   * because some C++ environments don't handle static constructors well.
   * @return The static Values object.
   */
  static Values&
  getValues()
  {
    if (!values_)
      values_ = new Values();

    return *values_;
  }

  static Values* values_;

  /**
   * EncryptorV2::Impl does the work of EncryptorV2. It is a separate class so
   * that EncryptorV2 can create an instance in a shared_ptr to use in callbacks.
   */
  class Impl : public ptr_lib::enable_shared_from_this<Impl> {
  public:
    /**
     * Create a new Impl, which should belong to a shared_ptr. Then you must 
     * call initialize(). See the EncryptorV2 constructor for parameter
     * documentation.
     */
    Impl
      (const Name& accessPrefix, const Name& ckPrefix,
       const SigningInfo& ckDataSigningInfo, const EncryptError::OnError& onError,
       Validator* validator, KeyChain* keyChain, Face* face)
    : accessPrefix_(accessPrefix), ckPrefix_(ckPrefix),
      ckDataSigningInfo_(ckDataSigningInfo), isKekRetrievalInProgress_(false),
      onError_(onError), keyChain_(keyChain), face_(face),
      kekPendingInterestId_(0)
    {}

    /**
     * Complete the work of the constructor. This is needed because we can't
     * call shared_from_this() in the constructor.
     */
    void
    initialize();

    void
    shutdown();

    ptr_lib::shared_ptr<EncryptedContent>
    encrypt(const uint8_t* plainData, size_t plainDataLength);

    /**
     * Create a new Content Key (CK) and publish the corresponding CK Data
     * packet. This uses the onError given to the constructor to report errors.
     */
    void
    regenerateCk();

    size_t
    size() { return storage_.size(); }

  private:
    // Give friend access to the tests.
    friend class ::TestEncryptorV2_EncryptAndPublishCk_Test;
    friend class ::TestEncryptorV2_EnumerateDataFromInMemoryStorage_Test;

    void
    retryFetchingKek();

    /**
     * Create an Interest for <access-prefix>/KEK to retrieve the
     * <access-prefix>/KEK/<key-id> KEK Data packet, and set kekData_.
     * @param onReady When the KEK is retrieved and published, this calls
     * onReady().
     * @param onError On failure, this calls onError(errorCode, message) where
     * errorCode is from the EncryptError::ErrorCode enum, and message is an
     * error string.
     * @param nTriesLeft The number of retries for expressInterest timeouts.
     */
    void
    fetchKekAndPublishCkData
      (const Face::Callback& onReady, const EncryptError::OnError& onError,
       int nTriesLeft);

    /**
     * Make a CK Data packet for ckName_ encrypted by the KEK in kekData_ and
     * insert it in the storage_.
     * @param onError On failure, this calls onError(errorCode, message) where
     * errorCode is from the EncryptError::ErrorCode enum, and message is an
     * error string.
     * @return True on success, else false.
     */
    bool
    makeAndPublishCkData(const EncryptError::OnError& onError);

    Name accessPrefix_;
    Name ckPrefix_;
    Name ckName_;
    uint8_t ckBits_[AES_KEY_SIZE];
    SigningInfo ckDataSigningInfo_;

    bool isKekRetrievalInProgress_;
    ptr_lib::shared_ptr<Data> kekData_;
    EncryptError::OnError onError_;

    // Storage for encrypted CKs.
    InMemoryStorageRetaining storage_;
    uint64_t ckRegisteredPrefixId_;
    uint64_t kekPendingInterestId_;

    KeyChain* keyChain_;
    Face* face_;

    static const uint64_t DEFAULT_CK_FRESHNESS_PERIOD_MS = 3600 * 1000;
  };

  ptr_lib::shared_ptr<Impl> impl_;
};

}

#endif
