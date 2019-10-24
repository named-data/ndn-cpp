/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2019 Regents of the University of California.
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

#ifndef NDN_CONSUMER_HPP
#define NDN_CONSUMER_HPP

#include <map>
#include "../data.hpp"
#include "../face.hpp"
#include "../security/key-chain.hpp"
#include "encrypt-error.hpp"
#include "encrypted-content.hpp"
#include "consumer-db.hpp"

// Give friend access to the tests.
class TestConsumer_DecryptContent_Test;

namespace ndn {

/**
 * A Consumer manages fetched group keys used to decrypt a data packet in the
 * group-based encryption protocol.
 * @note This class is an experimental feature. The API may change.
 */
class Consumer {
public:
  /**
   * Create a Consumer to use the given ConsumerDb, Face and other values.
   * @param face The face used for data packet and key fetching. This is only a
   * pointer to a Face object which must remain valid for the life of this
   * Consumer.
   * @param keyChain The keyChain used to verify data packets. This is only a
   * pointer to a KeyChain object which must remain valid for the life of this
   * Consumer.
   * @param groupName The reading group name that the consumer belongs to.
   * This makes a copy of the Name.
   * @param consumerName The identity of the consumer. This makes a copy of the
   * Name.
   * @param database The ConsumerDb database for storing decryption keys.
   * @param cKeyLink (optional) The Link object to use in Interests for C-KEY
   * retrieval. This makes a copy of the Link object. If the Link object's
   * getDelegations().size() is zero, don't use it. If omitted, don't use a Link
   * object.
   * @param dKeyLink (optional) The Link object to use in Interests for D-KEY
   * retrieval. This makes a copy of the Link object. If the Link object's
   * getDelegations().size() is zero, don't use it. If omitted, don't use a Link
   * object.
   */
  Consumer
    (Face* face, KeyChain* keyChain, const Name& groupName,
     const Name& consumerName, const ptr_lib::shared_ptr<ConsumerDb>& database,
     const Link& cKeyLink = getNO_LINK(), const Link& dKeyLink = getNO_LINK())
  : impl_(new Impl
      (face, keyChain, groupName, consumerName, database, cKeyLink, dKeyLink))
  {
  }

  typedef func_lib::function<void
    (const ptr_lib::shared_ptr<Data>& contentData,
     const Blob& result)> OnConsumeComplete;

  typedef func_lib::function<void(const Blob& decryptedBlob)> OnPlainText;

  /**
   * Express an Interest to fetch the content packet with contentName, and
   * decrypt it, fetching keys as needed.
   * @param contentName The name of the content packet.
   * @param onConsumeComplete When the content packet is fetched and decrypted,
   * this calls onConsumeComplete(contentData, result) where contentData is the
   * fetched Data packet and result is the decrypted plain text Blob.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onError This calls onError(errorCode, message) for an error.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param link (optional) The Link object to use in Interests for data
   * retrieval. This makes a copy of the Link object. If the Link object's
   * getDelegations().size() is zero, don't use it. If omitted, don't use a Link
   * object.
   */
  void
  consume
    (const Name& contentName, const OnConsumeComplete& onConsumeComplete,
     const EncryptError::OnError& onError, const Link& link = getNO_LINK())
  {
    impl_->consume(contentName, onConsumeComplete, onError, link);
  }

  /**
   * Set the group name.
   * @param groupName The reading group name that the consumer belongs to.
   * This makes a copy of the Name.
   */
  void
  setGroup(const Name& groupName) { impl_->setGroup(groupName); }

  /**
   * Add a new decryption key with keyName and keyBlob to the database.
   * @param keyName The key name.
   * @param keyBlob The encoded key.
   * @throws ConsumerDb::Error if a key with the same keyName already exists in
   * the database, or other database error.
   * @throws runtime_error if the consumer name is not a prefix of the key name.
   */
  void
  addDecryptionKey(const Name& keyName, const Blob& keyBlob)
  {
    impl_->addDecryptionKey(keyName, keyBlob);
  }

  /**
   * A utility method to decrypt the data packet, retrieving the C-KEY Data
   * from the network if necessary. An application normally does not call this
   * but calls the higher-level consume method.
   * @param data The data packet. This does not verify the packet.
   * @param onPlainText When the data packet is decrypted, this calls
   * onPlainText(decryptedBlob) with the decrypted blob.
   * @param onError This calls onError(errorCode, message) for an error.
   */
  void
  decryptContent
    (const Data& data, const OnPlainText& onPlainText,
     const EncryptError::OnError& onError)
  {
    impl_-> decryptContent(data, onPlainText, onError);
  }

private:
  // Give friend access to the tests.
  friend class ::TestConsumer_DecryptContent_Test;

  /**
   * Consumer::Impl does the work of Consumer. It is a separate class so that
   * Consumer can create an instance in a shared_ptr to use in callbacks.
   */
  class Impl : public ptr_lib::enable_shared_from_this<Impl> {
  public:
    /**
     * Create a new Impl, which should belong to a shared_ptr. See the Consumer
     * constructor for parameter documentation.
     */
    Impl
      (Face* face, KeyChain* keyChain, const Name& groupName,
       const Name& consumerName, const ptr_lib::shared_ptr<ConsumerDb>& database,
       const Link& cKeyLink, const Link& dKeyLink);

    void
    consume
      (const Name& contentName, const OnConsumeComplete& onConsumeComplete,
       const EncryptError::OnError& onError, const Link& link);

    void
    setGroup(const Name& groupName) { groupName_ = groupName; }

    void
    addDecryptionKey(const Name& keyName, const Blob& keyBlob);

    void
    decryptContent
      (const Data& data, const OnPlainText& onPlainText,
       const EncryptError::OnError& onError);

  private:
    // Give friend access to the tests.
    friend class ::TestConsumer_DecryptContent_Test;

    /**
     * Decode encryptedBlob as an EncryptedContent and decrypt using keyBits.
     * @param encryptedBlob The encoded EncryptedContent to decrypt.
     * @param keyBits The key value.
     * @param onPlainText When encryptedBlob is decrypted, this calls
     * onPlainText(decryptedBlob) with the decrypted blob.
     * @param onError This calls onError(errorCode, message) for an error.
     */
    static void
    decrypt
      (const Blob& encryptedBlob, const Blob& keyBits,
       const OnPlainText& onPlainText, const EncryptError::OnError& onError);

    /**
     * Decrypt encryptedContent using keyBits.
     * @param encryptedContent The EncryptedContent to decrypt.
     * @param keyBits The key value.
     * @param onPlainText When encryptedBlob is decrypted, this calls
     * onPlainText(decryptedBlob) with the decrypted blob.
     * @param onError This calls onError(errorCode, message) for an error.
     */
    static void
    decryptEncryptedContent
      (const EncryptedContent& encryptedContent, const Blob& keyBits,
       const OnPlainText& onPlainText, const EncryptError::OnError& onError);

    /**
     * Decrypt cKeyData.
     * @param cKeyData The C-KEY data packet.
     * @param onPlainText When the data packet is decrypted, this calls
     * onPlainText(decryptedBlob) with the decrypted blob.
     * @param onError This calls onError(errorCode, message) for an error.
     */
    void
    decryptCKey
      (const Data& cKeyData, const OnPlainText& onPlainText,
       const EncryptError::OnError& onError);

    /**
     * Decrypt dKeyData.
     * @param dKeyData The D-KEY data packet.
     * @param onPlainText When the data packet is decrypted, this calls
     * onPlainText(decryptedBlob) with the decrypted blob.
     * @param onError This calls onError.onError(errorCode, message) for an error.
     */
    void
    decryptDKey
      (const Data& dKeyData, const OnPlainText& onPlainText,
       const EncryptError::OnError& onError);

    /**
     * Get the encoded blob of the decryption key with decryptionKeyName from the
     * database.
     * @param decryptionKeyName The key name.
     * @return A Blob with the encoded key, or an isNull Blob if cannot find the
     * key with decryptionKeyName.
     * @throws ConsumerDb.Error for a database error.
     */
    Blob
    getDecryptionKey(const Name& decryptionKeyName)
    {
      return database_->getKey(decryptionKeyName);
    }

    /**
     * Express the interest, call verifyData for the fetched Data packet and
     * call onVerified if verify succeeds. If verify fails, call
     * onError(EncryptError::ErrorCode::Validation, "verifyData failed").
     * If the interest times out, re-express nRetrials times. If the interest
     * times out nRetrials times, or for a network Nack, call
     * onError(EncryptError::ErrorCode::DataRetrievalFailure, interest.getName().toUri()).
     * @param interest The Interest to express.
     * @param nRetrials The number of retrials left after a timeout.
     * @param link The Link object to use in the Interest. This does not make a
     * copy of the Link object. If the Link object's getDelegations().size() is
     * zero, don't use it.
     * @param onVerified When the fetched Data packet validation succeeds, this
     * calls onVerified(data).
     * @param onError This calls onError(errorCode, message) for an error.
     */
    void
    sendInterest
      (const ptr_lib::shared_ptr<const Interest>& interest, int nRetrials,
       const ptr_lib::shared_ptr<Link>& link, const OnVerified& onVerified,
       const EncryptError::OnError& onError);

    /**
     * Call onError(EncryptError::ErrorCode::Validation, "verifyData failed")
     * within a try block to log exceptions that it throws.
     */
    static void
    onValidationFailed
      (const ptr_lib::shared_ptr<Data>& data, const std::string& reason,
       const EncryptError::OnError& onError);

    ptr_lib::shared_ptr<ConsumerDb> database_;
    KeyChain* keyChain_;
    Face* face_;
    Name groupName_;
    Name consumerName_;

    const ptr_lib::shared_ptr<Link> cKeyLink_;
    // The map key is the C-KEY name. The value is the encoded key Blob.
    std::map<Name, Blob> cKeyMap_;
    const ptr_lib::shared_ptr<Link> dKeyLink_;
    // The map key is the D-KEY name. The value is the encoded key Blob.
    std::map<Name, Blob> dKeyMap_;
  };

  /**
   * Get the static NoLink object, creating it if needed. We do this explicitly
   * because some C++ environments don't handle static constructors well.
   * @return The static NoLink object.
   */
  static Link&
  getNO_LINK()
  {
    if (!noLink_)
      noLink_ = new Link();

    return *noLink_;
  }

  ptr_lib::shared_ptr<Impl> impl_;
  static Link* noLink_;
};

}

#endif
