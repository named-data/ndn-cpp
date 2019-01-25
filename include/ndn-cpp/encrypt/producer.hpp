/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/producer https://github.com/named-data/ndn-group-encrypt
 * @author: excludeRange from ndn-cxx https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/exclude.cpp
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

#ifndef NDN_PRODUCER_HPP
#define NDN_PRODUCER_HPP

#include <map>
#include "../face.hpp"
#include "../security/key-chain.hpp"
#include "encrypt-error.hpp"
#include "producer-db.hpp"

namespace ndn {

/**
 * A Producer manages content keys used to encrypt a data packet in the
 * group-based encryption protocol.
 * @note This class is an experimental feature. The API may change.
 */
class Producer {
public:
  typedef func_lib::function<
    void(const std::vector<ptr_lib::shared_ptr<Data> >& keys)> OnEncryptedKeys;

  /**
   * Create a Producer to use the given ProducerDb, Face and other values.
   *
   * A producer can produce data with a naming convention:
   *   /{prefix}/SAMPLE/{dataType}/[timestamp]
   *
   * The produced data packet is encrypted with a content key,
   * which is stored in the ProducerDb database.
   *
   * A producer also needs to produce data containing a content key
   * encrypted with E-KEYs. A producer can retrieve E-KEYs through the face,
   * and will re-try for at most repeatAttemps times when E-KEY retrieval fails.
   * @param prefix The producer name prefix. This makes a copy of the Name.
   * @param dataType The dataType portion of the producer name. This makes a
   * copy of the Name.
   * @param face The face used to retrieve keys. This is only a pointer to a
   * Face object which must remain valid for the life of this Producer.
   * @param keyChain The keyChain used to sign data packets. This is only a
   * pointer to a KeyChain object which must remain valid for the life of this
   * Producer.
   * @param database The ProducerDb database for storing keys.
   * @param repeatAttempts (optional) The maximum retry for retrieving keys. If
   * omitted, use 3.
   * @param keyRetrievalLink (optional) The Link object to use in Interests for
   * key retrieval. This makes a copy of the Link object. If the Link object's
   * getDelegations().size() is zero, don't use it. If omitted, don't use a Link
   * object.
   */
  Producer
    (const Name& prefix, const Name& dataType, Face* face, KeyChain* keyChain,
     const ptr_lib::shared_ptr<ProducerDb>& database, int repeatAttempts = 3,
     const Link& keyRetrievalLink = getNO_LINK())
  : impl_(new Impl
      (prefix, dataType, face, keyChain, database, repeatAttempts,
       keyRetrievalLink))
  {
  }

  /**
   * Create the content key corresponding to the timeSlot. This first checks if
   * the content key exists. For an existing content key, this returns the
   * content key name directly. If the key does not exist, this creates one and
   * encrypts it using the corresponding E-KEYs. The encrypted content keys are
   * passed to the onEncryptedKeys callback.
   * @param timeSlot The time slot as milliseconds since Jan 1, 1970 UTC.
   * @param onEncryptedKeys If this creates a content key, then this calls
   * onEncryptedKeys(keys) where keys is a list of encrypted content key Data
   * packets. If onEncryptedKeys is an empty OnEncryptedKeys(), this does not
   * use it.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onError (optional) This calls onError(errorCode, message) for an
   * error. If omitted, use a default callback which does nothing.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @return The content key name.
   */
  Name
  createContentKey
    (MillisecondsSince1970 timeSlot, const OnEncryptedKeys& onEncryptedKeys,
     const EncryptError::OnError& onError = defaultOnError)
  {
    return impl_->createContentKey(timeSlot, onEncryptedKeys, onError);
  }

  /**
   * Encrypt the given content with the content key that covers timeSlot, and
   * update the data packet with the encrypted content and an appropriate data
   * name.
   * @param data An empty Data object which is updated.
   * @param timeSlot The time slot as milliseconds since Jan 1, 1970 UTC.
   * @param content The content to encrypt.
   * @param onError (optional) This calls onError(errorCode, message) for an
   * error. If omitted, use a default callback which does nothing.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   */
  void
  produce
    (Data& data, MillisecondsSince1970 timeSlot, const Blob& content,
     const EncryptError::OnError& onError = defaultOnError)
  {
    impl_->produce(data, timeSlot, content, onError);
  }

  /**
   * The default OnError callback which does nothing.
   */
  static void
  defaultOnError(EncryptError::ErrorCode errorCode, const std::string& message);

private:
  /**
   * Producer::Impl does the work of Producer. It is a separate class so that
   * Producer can create an instance in a shared_ptr to use in callbacks.
   */
  class Impl : public ptr_lib::enable_shared_from_this<Impl> {
  public:
    /**
     * Create a new Impl, which should belong to a shared_ptr. See the Producer
     * constructor for parameter documentation.
     */
    Impl
      (const Name& prefix, const Name& dataType, Face* face, KeyChain* keyChain,
       const ptr_lib::shared_ptr<ProducerDb>& database, int repeatAttempts,
       const Link& keyRetrievalLink);

    Name
    createContentKey
      (MillisecondsSince1970 timeSlot, const OnEncryptedKeys& onEncryptedKeys,
       const EncryptError::OnError& onError);

    void
    produce
      (Data& data, MillisecondsSince1970 timeSlot, const Blob& content,
       const EncryptError::OnError& onError);

  private:
    class KeyInfo {
    public:
      KeyInfo() : beginTimeSlot(0), endTimeSlot(0) {}

      MillisecondsSince1970 beginTimeSlot;
      MillisecondsSince1970 endTimeSlot;
      Blob keyBits;
    };

    class KeyRequest {
    public:
      KeyRequest(int interests)
      {
        interestCount = interests;
      }

      int interestCount;
      std::map<Name, int> repeatAttempts;
      std::vector<ptr_lib::shared_ptr<Data> > encryptedKeys;
    };

    /**
     * Round timeSlot to the nearest whole hour, so that we can store content
     * keys uniformly (by start of the hour).
     * @param timeSlot The time slot as milliseconds since Jan 1, 1970 UTC.
     * @return The start of the hour as milliseconds since Jan 1, 1970 UTC.
     */
    static MillisecondsSince1970
    getRoundedTimeSlot(MillisecondsSince1970 timeSlot);

    /**
     * Send an interest with the given name through the face with callbacks to
     * handleCoveringKey, handleTimeout and handleNetworkNack.
     * @param interest The interest to send.
     * @param timeSlot The time slot, passed to handleCoveringKey, handleTimeout
     * and handleNetworkNack.
     * @param onEncryptedKeys The OnEncryptedKeys callback, passed to
     * handleCoveringKey, handleTimeout and handleNetworkNack.
     */
    void
    sendKeyInterest
      (const Interest& interest, MillisecondsSince1970 timeSlot,
       const OnEncryptedKeys& onEncryptedKeys,
       const EncryptError::OnError& onError);

    /**
     * This is called from an expressInterest timeout to update the state of
     * keyRequest. Re-express the interest if the number of retrials is less
     * than the max limit.
     * @param interest The timed-out interest.
     * @param timeSlot The time slot as milliseconds since Jan 1, 1970 UTC.
     * @param onEncryptedKeys When there are no more interests to process, this
     * calls onEncryptedKeys(keys) where keys is a list of encrypted content key
     * Data packets. If onEncryptedKeys is an empty OnEncryptedKeys(), this does
     * not use it.
     */
    void
    handleTimeout
      (const ptr_lib::shared_ptr<const Interest>& interest,
       MillisecondsSince1970 timeSlot, const OnEncryptedKeys& onEncryptedKeys,
       const EncryptError::OnError& onError);

    /**
     * This is called from an expressInterest OnNetworkNack to handle a network
     * Nack for the E-KEY requested through the Interest. Decrease the
     * outstanding E-KEY interest count for the C-KEY corresponding to the
     * timeSlot.
     * @param interest The interest given to expressInterest.
     * @param networkNack The returned NetworkNack (unused).
     * @param timeSlot The time slot as milliseconds since Jan 1, 1970 UTC.
     * @param onEncryptedKeys When there are no more interests to process, this
     * calls onEncryptedKeys(keys) where keys is a list of encrypted content key
     * Data packets. If onEncryptedKeys is an empty OnEncryptedKeys(), this does
     * not use it.
     */
    void
    handleNetworkNack
      (const ptr_lib::shared_ptr<const Interest>& interest,
       const ptr_lib::shared_ptr<NetworkNack>& networkNack,
       MillisecondsSince1970 timeSlot,
       const OnEncryptedKeys& onEncryptedKeys,
       const EncryptError::OnError& onError);

    /**
     * Decrease the count of outstanding E-KEY interests for the C-KEY for
     * timeCount. If the count decreases to 0, invoke onEncryptedKeys.
     * @param keyRequest The KeyRequest with the interestCount to update.
     * @param timeCount The time count for indexing keyRequests_.
     * @param onEncryptedKeys When there are no more interests to process, this
     * calls onEncryptedKeys(keys) where keys is a list of encrypted content key
     * Data packets. If onEncryptedKeys is an empty OnEncryptedKeys(), this does
     * not use it.
     */
    void
    updateKeyRequest
      (const ptr_lib::shared_ptr<KeyRequest>& keyRequest,
       MillisecondsSince1970 timeCount, const OnEncryptedKeys& onEncryptedKeys);

    /**
     * This is called from an expressInterest OnData to check that the
     * encryption key contained in data fits the timeSlot. This sends a refined
     * interest if required.
     * @param interest The interest given to expressInterest.
     * @param data The fetched Data packet.
     * @param timeSlot The time slot as milliseconds since Jan 1, 1970 UTC.
     * @param onEncryptedKeys When there are no more interests to process, this
     * calls onEncryptedKeys(keys) where keys is a list of encrypted content key
     * Data packets. If onEncryptedKeys is an empty OnEncryptedKeys(), this does
     * not use it.
     */
    void
    handleCoveringKey
      (const ptr_lib::shared_ptr<const Interest>& interest,
       const ptr_lib::shared_ptr<Data>& data, MillisecondsSince1970 timeSlot,
       const OnEncryptedKeys& onEncryptedKeys,
       const EncryptError::OnError& onError);

    /**
     * Get the content key from the database_ and encrypt it for the timeSlot
     * using encryptionKey.
     * @param encryptionKey The encryption key value.
     * @param eKeyName The key name for the EncryptedContent.
     * @param timeSlot The time slot as milliseconds since Jan 1, 1970 UTC.
     * @param onEncryptedKeys When there are no more interests to process, this
     * calls onEncryptedKeys(keys) where keys is a list of encrypted content key
     * Data packets. If onEncryptedKeys is an empty OnEncryptedKeys(), this does
     * not use it.
     * @return True if encryption succeeds, otherwise false.
     */
    bool
    encryptContentKey
      (const Blob& encryptionKey, const Name& eKeyName,
       MillisecondsSince1970 timeSlot, const OnEncryptedKeys& onEncryptedKeys,
       const EncryptError::OnError& onError);

    // TODO: Move this to be the main representation inside the Exclude object.
    class ExcludeEntry {
    public:
      ExcludeEntry(const Name::Component& component, bool anyFollowsComponent)
      : component_(component), anyFollowsComponent_(anyFollowsComponent)
      {
      }

      Name::Component component_;
      bool anyFollowsComponent_;
    };

    /**
     * Create a list of ExcludeEntry from the Exclude object.
     * @param exclude The Exclude object to read.
     * @param entries Clear and set to a list of ExcludeEntry.
     */
    static void
    getExcludeEntries(const Exclude& exclude, std::vector<ExcludeEntry>& entries);

    /**
     * Set the Exclude object from the list of ExcludeEntry.
     * @param exclude The Exclude object to update.
     * @param entries The list of ExcludeEntry.
     */
    static void
    setExcludeEntries(Exclude& exclude, const std::vector<ExcludeEntry>& entries);

    /**
     * Get the latest entry in the list whose component_ is less than or equal
     * to component.
     * @param entries The list of ExcludeEntry.
     * @param component The component to compare.
     * @return The index of the found entry, or -1 if not found.
     */
    static int
    findEntryBeforeOrAt
      (const std::vector<ExcludeEntry>& entries,
       const Name::Component& component);

    /**
     * Exclude all components in the range beginning at "from".
     * @param exclude The Exclude object to update.
     * @param from The first component in the exclude range.
     */
    static void
    excludeAfter(Exclude& exclude, const Name::Component& from);

    /**
     * Exclude all components in the range ending at "to".
     * @param exclude The Exclude object to update.
     * @param to The last component in the exclude range.
     */
    static void
    excludeBefore(Exclude& exclude, const Name::Component& to)
    {
      excludeRange(exclude, Name::Component(), to);
    }

    /**
     * Exclude all components in the range beginning at "from" and ending at "to".
     * @param exclude The Exclude object to update.
     * @param from The first component in the exclude range.
     * @param to The last component in the exclude range.
     */
    static void
    excludeRange
      (Exclude& exclude, const Name::Component& from, const Name::Component& to);

    Face* face_;
    Name namespace_;
    KeyChain* keyChain_;
    std::map<Name, ptr_lib::shared_ptr<KeyInfo> > eKeyInfo_;
    std::map<MillisecondsSince1970, ptr_lib::shared_ptr<KeyRequest> > keyRequests_;
    ptr_lib::shared_ptr<ProducerDb> database_;
    int maxRepeatAttempts_;

    Link keyRetrievalLink_;

    static const int START_TIME_STAMP_INDEX = -2;
    static const int END_TIME_STAMP_INDEX = -1;
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
