/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2016 Regents of the University of California.
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

#ifndef NDN_MEMORY_CONTENT_CACHE_HPP
#define NDN_MEMORY_CONTENT_CACHE_HPP

#include <map>
#include <deque>
#include "../face.hpp"

namespace ndn {

/**
 * A MemoryContentCache holds a set of Data packets and answers an Interest to
 * return the correct Data packet. The cache is periodically cleaned up to
 * remove each stale Data packet based on its FreshnessPeriod (if it has one).
 * @note This class is an experimental feature.  See the API docs for more detail at
 * http://named-data.net/doc/ndn-ccl-api/memory-content-cache.html .
 */
class MemoryContentCache {
public:
  /**
   * Create a new MemoryContentCache to use the given Face.
   * @param face The Face to use to call registerPrefix and setInterestFilter,
   * and which will call this object's OnInterest callback.
   * @param cleanupIntervalMilliseconds (optional) The interval in milliseconds
   * between each check to clean up stale content in the cache. If omitted,
   * use a default of 1000 milliseconds. If this is a large number, then
   * effectively the stale content will not be removed from the cache.
   */
  MemoryContentCache
    (Face* face, Milliseconds cleanupIntervalMilliseconds = 1000.0);

  /**
   * A PendingInterest holds an interest which onInterest received but could
   * not satisfy. When we add a new data packet to the cache, we will also check
   * if it satisfies a pending interest.
   */
  class PendingInterest {
  public:
    /**
     * Create a new PendingInterest and set the timeoutTime_ based on the
     * current time and the interest lifetime.
     * @param interest A shared_ptr for the interest.
     * @param face The face from the onInterest callback. If the
     * interest is satisfied later by a new data packet, we will send the data
     * packet to the face.
     */
    PendingInterest
      (const ptr_lib::shared_ptr<const Interest>& interest, Face& face);

    /**
     * Return the interest given to the constructor. You must not modify this
     * object - if you need to modify it then make a copy.
     */
    const ptr_lib::shared_ptr<const Interest>&
    getInterest() const { return interest_; }

    /**
     * Return the time when this pending interest entry was created (the time
     * when the unsatisfied interest arrived and was added to the pending
     * interest table). The interest timeout is based on this value.
     * @return The timeout period start time in milliseconds since 1/1/1970,
     * as returned by ndn_getNowMilliseconds.
     */
    MillisecondsSince1970
    getTimeoutPeriodStart() const { return timeoutPeriodStart_; }

    /**
     * Return the face given to the constructor.
     */
    Face&
    getFace() const { return face_; }

    /**
     * Check if this interest is timed out.
     * @param nowMilliseconds The current time in milliseconds from
     * ndn_getNowMilliseconds.
     * @return true if this interest timed out, otherwise false.
     */
    bool
    isTimedOut(MillisecondsSince1970 nowMilliseconds) const
    {
      return timeoutTimeMilliseconds_ >= 0.0 &&
             nowMilliseconds >= timeoutTimeMilliseconds_;
    }

  private:
    ptr_lib::shared_ptr<const Interest> interest_;
    Face& face_;
    MillisecondsSince1970 timeoutPeriodStart_;
    MillisecondsSince1970 timeoutTimeMilliseconds_; /**< The time when the
      * interest times out in milliseconds according to ndn_getNowMilliseconds,
      * or -1 for no timeout. */
  };

  /**
   * Call registerPrefix on the Face given to the constructor so that this
   * MemoryContentCache will answer interests whose name has the prefix.
   * Alternatively, if the Face's registerPrefix has already been called, then
   * you can call this object's setInterestFilter.
   * @param prefix The Name for the prefix to register. This copies the Name.
   * @param onRegisterFailed A function object to call if failed to retrieve the
   * connected hub’s ID or failed to register the prefix. This calls
   * onRegisterFailed(prefix) where prefix is the prefix given to registerPrefix.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onDataNotFound (optional) If a data packet for an interest is not
   * found in the cache, this forwards the interest by calling
   * onDataNotFound(prefix, interest, face, interestFilterId, filter).
   * Your callback can find the Data packet for the interest and call
   * face.putData(data).  If your callback cannot find the Data packet, it can
   * optionally call storePendingInterest(interest, face) to store the pending
   * interest in this object to be satisfied by a later call to add(data). If
   * you want to automatically store all pending interests, you can simply use
   * getStorePendingInterest() for onDataNotFound. If onDataNotFound is an empty
   * OnInterest(), this does not use it. This copies the function object, so you
   * may need to use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param flags (optional) See Face::registerPrefix.
   * @param wireFormat (optional) See Face::registerPrefix.
   */
  void
  registerPrefix
    (const Name& prefix, const OnRegisterFailed& onRegisterFailed,
     const OnInterestCallback& onDataNotFound = OnInterestCallback(),
     const ForwardingFlags& flags = ForwardingFlags(),
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    registerPrefix
      (prefix, onRegisterFailed, OnRegisterSuccess(), onDataNotFound, flags,
       wireFormat);
  }

  /**
   * Call registerPrefix on the Face given to the constructor so that this
   * MemoryContentCache will answer interests whose name has the prefix.
   * Alternatively, if the Face's registerPrefix has already been called, then
   * you can call this object's setInterestFilter.
   * @param prefix The Name for the prefix to register. This copies the Name.
   * @param onRegisterFailed A function object to call if failed to retrieve the
   * connected hub’s ID or failed to register the prefix. This calls
   * onRegisterFailed(prefix) where prefix is the prefix given to registerPrefix.
   * @param onRegisterSuccess (optional) A function object to call registerPrefix
   * receives a success message from the forwarder. This calls
   * onRegisterSuccess(prefix, registeredPrefixId) where  prefix and
   * registeredPrefixId are the values given to registerPrefix. If
   * onRegisterSuccess is an empty OnRegisterSuccess(), this does not use it.
   * @param onDataNotFound (optional) If a data packet for an interest is not
   * found in the cache, this forwards the interest by calling
   * onDataNotFound(prefix, interest, face, interestFilterId, filter).
   * Your callback can find the Data packet for the interest and call
   * face.putData(data).  If your callback cannot find the Data packet, it can
   * optionally call storePendingInterest(interest, face) to store the pending
   * interest in this object to be satisfied by a later call to add(data). If
   * you want to automatically store all pending interests, you can simply use
   * getStorePendingInterest() for onDataNotFound. If onDataNotFound is an empty
   * OnInterest(), this does not use it. This copies the function object, so you
   * may need to use func_lib::ref() as appropriate.
   * @param flags (optional) See Face::registerPrefix.
   * @param wireFormat (optional) See Face::registerPrefix.
   */
  void
  registerPrefix
    (const Name& prefix, const OnRegisterFailed& onRegisterFailed,
     const OnRegisterSuccess& onRegisterSuccess,
     const OnInterestCallback& onDataNotFound = OnInterestCallback(),
     const ForwardingFlags& flags = ForwardingFlags(),
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    onDataNotFoundForPrefix_[prefix.toUri()] = onDataNotFound;
    // TODO: After we remove the registerPrefix with the deprecated OnInterest,
    // we can remove the explicit cast to OnInterestCallback (needed for boost).
    uint64_t registeredPrefixId = face_->registerPrefix
      (prefix, (const OnInterestCallback&)func_lib::ref(*this), onRegisterFailed,
       onRegisterSuccess, flags, wireFormat);
    // Remember the registeredPrefixId so unregisterAll can remove it.
    registeredPrefixIdList_.push_back(registeredPrefixId);
  }

  /**
   * Call setInterestFilter on the Face given to the constructor so that this
   * MemoryContentCache will answer interests whose name matches the filter.
   * @param filter The InterestFilter with a prefix and optional regex filter
   * used to match the name of an incoming Interest. This makes a copy of filter.
   * @param onDataNotFound (optional) If a data packet for an interest is not
   * found in the cache, this forwards the interest by calling
   * onDataNotFound(prefix, interest, face, interestFilterId, filter).
   * Your callback can find the Data packet for the interest and call
   * face.putData(data).  Note: If you call setInterestFilter multiple times where
   * filter.getPrefix() is the same, it is undetermined which onDataNotFound
   * will be called. If your callback cannot find the Data packet, it can
   * optionally call storePendingInterest(interest, face) to store the pending
   * interest in this object to be satisfied by a later call to add(data). If
   * you want to automatically store all pending interests, you can simply use
   * getStorePendingInterest() for onDataNotFound. If onDataNotFound is an empty
   * OnInterest(), this does not use it. This copies the function object, so you
   * may need to use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   */
  void
  setInterestFilter
    (const InterestFilter& filter,
     const OnInterestCallback& onDataNotFound = OnInterestCallback())
  {
    onDataNotFoundForPrefix_[filter.getPrefix().toUri()] = onDataNotFound;
    uint64_t interestFilterId = face_->setInterestFilter
      (filter, func_lib::ref(*this));
    // Remember the interestFilterId so unregisterAll can remove it.
    interestFilterIdList_.push_back(interestFilterId);
  }

  /**
   * Call setInterestFilter on the Face given to the constructor so that this
   * MemoryContentCache will answer interests whose name has the prefix.
   * @param prefix The Name prefix used to match the name of an incoming
   * Interest. This copies the Name.
   * @param onDataNotFound (optional) If a data packet for an interest is not
   * found in the cache, this forwards the interest by calling
   * onDataNotFound(prefix, interest, face, interestFilterId, filter).
   * Your callback can find the Data packet for the interest and call
   * face.putData(data).  If your callback cannot find the Data packet, it can
   * optionally call storePendingInterest(interest, face) to store the pending
   * interest in this object to be satisfied by a later call to add(data). If
   * you want to automatically store all pending interests, you can simply use
   * getStorePendingInterest() for onDataNotFound. If onDataNotFound is an empty
   * OnInterest(), this does not use it. This copies the function object, so you
   * may need to use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   */
  void
  setInterestFilter
    (const Name& prefix,
     const OnInterestCallback& onDataNotFound = OnInterestCallback())
  {
    onDataNotFoundForPrefix_[prefix.toUri()] = onDataNotFound;
    uint64_t interestFilterId = face_->setInterestFilter
      (prefix, func_lib::ref(*this));
    // Remember the interestFilterId so unregisterAll can remove it.
    interestFilterIdList_.push_back(interestFilterId);
  }

  /**
   * Call Face.unsetInterestFilter and Face.removeRegisteredPrefix for all the
   * prefixes given to the setInterestFilter and registerPrefix method on this
   * MemoryContentCache object so that it will not receive interests any more.
   * You can call this if you want to "shut down" this MemoryContentCache while
   * your application is still running.
   */
  void
  unregisterAll();

  /**
   * Add the Data packet to the cache so that it is available to use to
   * answer interests. If data.getMetaInfo().getFreshnessPeriod() is not
   * negative, set the staleness time to now plus
   * data.getMetaInfo().getFreshnessPeriod(), which is checked during cleanup to
   * remove stale content. This also checks if cleanupIntervalMilliseconds
   * milliseconds have passed and removes stale content from the cache. After
   * removing stale content, remove timed-out pending interests from
   * storePendingInterest(), then if the added Data packet satisfies any
   * interest, send it through the transport and remove the interest from the
   * pending interest table.
   * Because this modifies the internal tables, you should call this on the same
   * thread as processEvents, which can also modify the tables.
   * @param data The Data packet object to put in the cache. This copies the
   * fields from the object.
   */
  void
  add(const Data& data);

  /**
   * Store an interest from an OnInterest callback in the internal pending
   * interest table (normally because there is no Data packet available yet to
   * satisfy the interest). add(data) will check if the added Data packet
   * satisfies any pending interest and send it through the face.
   * Because this modifies the internal tables, you should call this on the same
   * thread as processEvents, which can also modify the tables.
   * @param interest The Interest for which we don't have a Data packet yet. You
   * should not modify the interest after calling this.
   * @param face The Face with the connection which received the
   * interest. This comes from the onInterest callback.
   */
  void
  storePendingInterest
    (const ptr_lib::shared_ptr<const Interest>& interest, Face& face);

  /**
   * Return a callback to use for onDataNotFound in registerPrefix which simply
   * calls storePendingInterest() to store the interest that doesn't match a
   * Data packet. add(data) will check if the added Data packet satisfies any
   * pending interest and send it.
   * @return A callback to use for onDataNotFound in registerPrefix().
   */
  const OnInterestCallback&
  getStorePendingInterest()
  {
    return storePendingInterestCallback_;
  }

  /**
   * Remove timed-out pending interests, then for each pending interest which
   * matches according to Interest.matchesName(name), append the PendingInterest
   * entry to the given pendingInterests list.
   * Because this modifies the internal tables, you should call this on the same
   * thread as processEvents, which can also modify the tables.
   * @param name The name to check.
   * @param pendingInterests The vector to receive the matching PendingInterest
   * objects. This first clears the list before adding objects. You should not
   * modify the PendingInterest objects.
   */
  void
  getPendingInterestsForName
    (const Name& name,
     std::vector<ptr_lib::shared_ptr<const PendingInterest> >& pendingInterests);

  /**
   * This is the OnInterest callback which is called when the library receives
   * an interest whose name has the prefix given to registerPrefix. First check
   * if cleanupIntervalMilliseconds milliseconds have passed and remove stale
   * content from the cache. Then search the cache for the Data packet, matching
   * any interest selectors including ChildSelector, and send the Data packet
   * to the transport. If no matching Data packet is in the cache, call
   * the callback in onDataNotFoundForPrefix_ (if defined).
   */
  void
  operator()
    (const ptr_lib::shared_ptr<const Name>& prefix,
     const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
     uint64_t interestFilterId,
     const ptr_lib::shared_ptr<const InterestFilter>& filter);

private:
  /**
   * Content is a private class to hold the name and encoding for each entry
   * in the cache. This base class is for a Data packet without a
   * FreshnessPeriod.
   */
  class Content {
  public:
    /**
     * Create a new Content entry to hold data's name and wire encoding.
     * @param data The Data packet whose name and wire encoding are copied.
     */
    Content(const Data& data)
    // wireEncode returns the cached encoding if available.
    : name_(data.getName()), dataEncoding_(data.wireEncode())
    {}

    const Name&
    getName() const { return name_; }

    const Blob&
    getDataEncoding() const { return dataEncoding_; }

  private:
    Name name_;
    Blob dataEncoding_;
  };

  /**
   * StaleTimeContent extends Content to include the staleTimeMilliseconds
   * for when this entry should be cleaned up from the cache.
   */
  class StaleTimeContent : public Content {
  public:
    /**
     * Create a new StaleTimeContent to hold data's name and wire encoding
     * as well as the staleTimeMilliseconds which is now plus
     * data.getMetaInfo().getFreshnessPeriod().
     * @param data The Data packet whose name and wire encoding are copied.
     */
    StaleTimeContent(const Data& data);

    /**
     * Check if this content is stale.
     * @param nowMilliseconds The current time in milliseconds from
     * ndn_getNowMilliseconds.
     * @return True if this content is stale, otherwise false.
     */
    bool
    isStale(MillisecondsSince1970 nowMilliseconds) const
    {
      return staleTimeMilliseconds_ <= nowMilliseconds;
    }

    /**
     * Compare shared_ptrs to Content based only on staleTimeMilliseconds_.
     */
    class Compare {
    public:
      bool
      operator()
        (const ptr_lib::shared_ptr<const StaleTimeContent>& x,
         const ptr_lib::shared_ptr<const StaleTimeContent>& y) const
      {
        return x->staleTimeMilliseconds_ < y->staleTimeMilliseconds_;
      }
    };

  private:
    MillisecondsSince1970 staleTimeMilliseconds_; /**< The time when the content
      becomse stale in milliseconds according to ndn_getNowMilliseconds */
  };

  /**
   * Check if now is greater than nextCleanupTime_ and, if so, remove stale
   * content from staleTimeCache_ and reset nextCleanupTime_ based on
   * cleanupIntervalMilliseconds_. Since add(Data) does a sorted insert into
   * staleTimeCache_, the check for stale data is quick and does not require
   * searching the entire staleTimeCache_.
   */
  void
  doCleanup();

  /**
   * This is a private method to return for setting storePendingInterestCallback_.
   * We need a separate method because the arguments are different from the main
   * storePendingInterest.
   */
  void
  storePendingInterestCallback
    (const ptr_lib::shared_ptr<const Name>& prefix,
     const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
     uint64_t interestFilterId,
     const ptr_lib::shared_ptr<const InterestFilter>& filter)
  {
    storePendingInterest(interest, face);
  }

  Face* face_;
  Milliseconds cleanupIntervalMilliseconds_;
  MillisecondsSince1970 nextCleanupTime_;
  std::map<std::string, OnInterestCallback> onDataNotFoundForPrefix_; /**< The map key is the prefix.toUri() */
  std::vector<uint64_t> interestFilterIdList_;
  std::vector<uint64_t> registeredPrefixIdList_;
  std::vector<ptr_lib::shared_ptr<const Content> > noStaleTimeCache_;
  // Use a deque so we can efficiently remove from the front.
  std::deque<ptr_lib::shared_ptr<const StaleTimeContent> > staleTimeCache_;
  StaleTimeContent::Compare contentCompare_;
  Name::Component emptyComponent_;
  std::vector<ptr_lib::shared_ptr<const PendingInterest> > pendingInterestTable_;
  OnInterestCallback storePendingInterestCallback_;
};

}

#endif
