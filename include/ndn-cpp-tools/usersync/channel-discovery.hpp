/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
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

#ifndef NDN_CHANNEL_DISCOVERY_HPP
#define NDN_CHANNEL_DISCOVERY_HPP

#include <map>
#include <ndn-cpp/sync/chrono-sync2013.hpp>

namespace ndntools {

/**
 * The ChannelDiscovery class uses ChronoSync to listen to the application's
 * "discovery" channel for announcements of other channels. It maintains a list
 * of the current channels created by each user and informs the application when
 * a channel is added or removed.
 * @note The support for ChannelDiscovery is experimental and the API is not finalized.
 */
class ChannelDiscovery {
public:
  enum ErrorCode {
    REGISTER_FAILED = 1,
    INTEREST_TIMEOUT = 2
  };

  typedef ndn::func_lib::function<void
    (ChannelDiscovery& channelDiscovery,
     const ndn::ptr_lib::shared_ptr<ndn::Name>& userPrefix)> OnReceivedChannelList;

  typedef ndn::func_lib::function<void
    (ErrorCode errorCode, const std::string& message)> OnError;

  /**
   * Create a ChannelDiscovery with the given parameters and begin listening for
   * channel announcements.
   * @param applicationDataPrefix The name prefix for the Data packet from this
   * user with the list of channels created by this user. For example,
   * /ndn/edu/ucla/remap/peter/flume/discovery .
   * @param channelListFilePath The file path to store the Data packet from this
   * user with the list of channels created by this user. This is used for
   * persistent storage between sessions. This will create the file if it
   * doesn't exist.
   * @param applicationBroadcastPrefix The name prefix for ChonoSync broadcast
   * interests and sync updates. For example, /ndn/broadcast/flume/discovery .
   * @param face The Face for calling registerPrefix and expressInterest. The
   * Face object must remain valid for the life of this ChannelDiscovery object.
   * @param keyChain To sign a data packet containing a sync state message, this
   * calls keyChain.sign(data, certificateName).
   * @param certificateName The certificate name of the key to use for signing a
   * data packet containing a sync state message.
   * @param syncLifetime The interest lifetime in milliseconds for sending
   * sync interests, for example 5000.0.
   * @param onReceivedChannelList On receiving a channel list from another user,
   * this calls onReceivedChannelList(channelDiscovery, userPrefix) where
   * channelDiscovery is this ChannelDiscovery object and userPrefix is the Name
   * of the user who updated the channel list. To get the new list, call
   * channelDiscovery.getChannelList(*userPrefix).
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onError Call onError(errorCode, message) for timeout or an error
   * processing segments.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   */
  ChannelDiscovery
    (const ndn::Name& applicationDataPrefix,
     const std::string& channelListFilePath,
     const ndn::Name& applicationBroadcastPrefix, ndn::Face& face,
     ndn::KeyChain& keyChain, const ndn::Name& certificateName,
     ndn::Milliseconds syncLifetime,
     const OnReceivedChannelList& onReceivedChannelList, const OnError& onError)
  : impl_(new Impl
      (*this, applicationDataPrefix, channelListFilePath, face, keyChain,
       certificateName, syncLifetime, onReceivedChannelList, onError))
  {
    impl_->initialize(applicationBroadcastPrefix);
  }

  /**
   * Add the channel to this user's list of channels and publish the new list.
   * @param channel The new channel name. If already in the list of channels,
   * this does nothing.
   */
  void
  addChannel(const std::string& channel)
  {
    impl_->addChannel(channel);
  }

  /**
   * Remove the channel from this user's list of channels and publish the new
   * list.
   * @param channel The name of the channel to remove. If already removed from
   * the list of channels, this does nothing.
   */
  void
  removeChannel(const std::string& channel)
  {
    impl_->removeChannel(channel);
  }

  /**
   * Get the applicationDataPrefix given to the constructor.
   * @return The applicationDataPrefix. You should not change this object.
   */
  const ndn::Name&
  getApplicationDataPrefix() { return impl_->getApplicationDataPrefix(); }

  /**
   * Get the list of prefixes for other users, which can be used in
   * getChannelList(userPrefix).
   * @return An unsorted copy of the list of user prefix Names. This does not
   * contain this user's getApplicationDataPrefix().
   */
  ndn::ptr_lib::shared_ptr<std::vector<ndn::Name>>
  getOtherUserPrefixes()
  {
    return impl_->getOtherUserPrefixes();
  }

  /**
   * Get the channel list for the user. To get the channel list for this user,
   * call getChannelList(getApplicationDataPrefix()).
   * @param userPrefix The user's application data prefix.
   * @return A sorted copy of the user's channel list, or null if the userPrefix
   * does not exist.
   */
  ndn::ptr_lib::shared_ptr<std::vector<std::string>>
  getChannelList(const ndn::Name& userPrefix)
  {
    return impl_->getChannelList(userPrefix);
  }

  // TODO: Implement shutdown.

private:
  /**
   * ChannelDiscovery::Impl does the work of ChannelDiscovery. It is a separate
   * class so that ChannelDiscovery can create an instance in a shared_ptr to
   * use in callbacks.
   */
  class Impl : public ndn::ptr_lib::enable_shared_from_this<Impl> {
  public:
    /**
     * Create a new Impl, which should belong to a shared_ptr. Then you must
     * call initialize().  See the ChannelDiscovery constructor for parameter
     * documentation.
     */
    Impl
      (ChannelDiscovery& parent, const ndn::Name& applicationDataPrefix,
       const std::string& channelListFilePath, ndn::Face& face,
       ndn::KeyChain& keyChain, const ndn::Name& certificateName,
       ndn::Milliseconds syncLifetime,
       const OnReceivedChannelList& onReceivedChannelList, const OnError& onError);

    /**
     * This is called by the ChannelDiscovery constructor after creating this
     * Impl object. Register to receive Interests under the
     * applicationDataPrefix_ and proceed with onRegisterApplicationPrefixSuccess.
     * @param applicationBroadcastPrefix From the ChannelDiscovery constructor.
     */
    void
    initialize(const ndn::Name& applicationBroadcastPrefix);

    void
    addChannel(const std::string& channel);

    void
    removeChannel(const std::string& channel);

    const ndn::Name&
    getApplicationDataPrefix() { return applicationDataPrefix_; }

    ndn::ptr_lib::shared_ptr<std::vector<ndn::Name>>
    getOtherUserPrefixes();

    ndn::ptr_lib::shared_ptr<std::vector<std::string>>
    getChannelList(const ndn::Name& userPrefix);

    /**
     * This is called on success after initialize registers the
     * applicationDataPrefix.
     * @param prefix The prefix given to registerPrefix.
     * @param registeredPrefixId The prefix from registerPrefix.
     * @param applicationBroadcastPrefix From the ChannelDiscovery constructor.
     * @param previousSequenceNumber The value computed in initialize
     */
    void
    onRegisterApplicationPrefixSuccess
      (const ndn::ptr_lib::shared_ptr<const ndn::Name>& prefix,
       uint64_t registeredPrefixId, const ndn::Name& applicationBroadcastPrefix,
       int previousSequenceNumber);

    /**
     * This is called when ChronoSync2013 is initialized. Do nothing.
     */
    void
    onInitialized();

    /** This is called when ChronoSync reports a new sequence number from
     * another user. Express an interest for the user's channel list.
     */
    void
    onReceivedSyncState
      (const std::vector<ndn::ChronoSync2013::SyncState>& syncStates,
       bool isRecovery);

    /**
     * This is called on receiving the other user's channel list.
     */
    void
    onData
      (const ndn::ptr_lib::shared_ptr<const ndn::Interest>& interest,
       const ndn::ptr_lib::shared_ptr<ndn::Data>& data);

    /** This is called when receiving an Interest for applicationDataPrefix_,
     */
    void
    onApplicationInterest
      (const ndn::ptr_lib::shared_ptr<const ndn::Name>& prefix,
       const ndn::ptr_lib::shared_ptr<const ndn::Interest>& interest,
       ndn::Face& face, uint64_t interestFilterId,
       const ndn::ptr_lib::shared_ptr<const ndn::InterestFilter>& filter);

    /**
     * This is called when the ChronoSync2013 constructor fails to register the
     * prefix.
     * @param prefix The Name prefix from registerPrefix.
     */
    void
    onRegisterFailed(const ndn::ptr_lib::shared_ptr<const ndn::Name>& prefix);

    /**
     * This is called when an expressed interest for another user's channel list
     * times out.
     * @param interest The Interest given to expressInterest.
     */
    void
    onTimeout(const ndn::ptr_lib::shared_ptr<const ndn::Interest>& interest);

    /**
     * Use ChronoSync publishNextSequenceNo to get the next sequence number, use
     * makeChannelListData to create the Data packet containing myChannelList_,
     * save it as channelListData_ and also write it to channelListFilePath_.
     */
    void
    publishChannelListData();

    /**
     * Make a new Data packet where the name is
     * <applicationDataPrefix_>/<sessionNo_>/<sequenceNumber> and the content is
     * myChannelList_.
     * @param sequenceNo The sequence number for the Data name.
     * @return The new Data packet.
     */
    ndn::ptr_lib::shared_ptr<ndn::Data>
    makeChannelListData(int sequenceNo);

    /**
     * Split the new-line separated blob and URI decode each line, adding to the
     * list.
     * @param blob The Blob with the list, for example the Data packet content.
     * @param channelList This adds each channel to the list. This does not
     * first clear the list.
     */
    static void
    parseChannelList
      (const ndn::Blob& blob, std::vector<std::string>& channelList);

    /**
     * Read the filePath, decode as base64 and return the Blob.
     * @param filePath The file path to read.
     * @return The decoded Blob. If the file does not exist, the Blob isNull.
     */
    static ndn::Blob
    readBase64Blob(const std::string& filePath);

    /**
     * Encode the blob in base64 and write to the filePath.
     * @param blob The blob to write.
     * @param filePath The file path to write to.
     */
    static void
    writeBase64Blob(const ndn::Blob& blob, const std::string& filePath);

  private:
    ChannelDiscovery& parent_;
    ndn::Name applicationDataPrefix_;
    int sessionNo_;
    std::string channelListFilePath_;
    ndn::Face& face_;
    ndn::KeyChain& keyChain_;
    ndn::Name certificateName_;
    ndn::Milliseconds syncLifetime_;
    OnReceivedChannelList onReceivedChannelList_;
    OnError onError_;
    ndn::ptr_lib::shared_ptr<ndn::ChronoSync2013> sync_;
    std::vector<std::string> myChannelList_;
    // The key is the user's application prefix. The value is the list of channel names.
    std::map<std::string, std::vector<std::string>> otherUserChannelList_;
    ndn::ptr_lib::shared_ptr<ndn::Data> channelListData_;
    bool enabled_;
    bool isInitialized_;
    bool needToPublish_;
  };

  ndn::ptr_lib::shared_ptr<Impl> impl_;
};

}

#endif
