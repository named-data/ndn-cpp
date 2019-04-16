/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2019 Regents of the University of California.
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
    (Face* face, Milliseconds cleanupIntervalMilliseconds = 1000.0)
  : impl_(new Impl(face, cleanupIntervalMilliseconds))
  {
    impl_->initialize();
  }

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
      return nowMilliseconds >= timeoutTimeMilliseconds_;
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
   * Content is a private class to hold the name and encoding for each entry
   * in the cache.
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

    /**
     * Get the content Name.
     * @return The content Name.
     */
    const Name&
    getName() const { return name_; }

    /**
     * Get the encoded content Data packet.
     * @return The encoded content Blob.
     */
    const Blob&
    getDataEncoding() const { return dataEncoding_; }

  private:
    Name name_;
    Blob dataEncoding_;
  };

  typedef std::vector<ptr_lib::shared_ptr<const Content> > ContentList;

  /**
   * An OnContentRemoved function object is called when content is removed from
   * the cache. See setOnContentRemoved().
   */
  typedef func_lib::function<void
    (const ptr_lib::shared_ptr<ContentList>& contentList)> OnContentRemoved;

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
   * OnInterestCallback(), this does not use it. This copies the function object, so you
   * may need to use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param registrationOptions (optional) See Face::registerPrefix.
   * @param wireFormat (optional) See Face::registerPrefix.
   */
  void
  registerPrefix
    (const Name& prefix, const OnRegisterFailed& onRegisterFailed,
     const OnInterestCallback& onDataNotFound = OnInterestCallback(),
     const RegistrationOptions& registrationOptions = RegistrationOptions(),
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    registerPrefix
      (prefix, onRegisterFailed, OnRegisterSuccess(), onDataNotFound, registrationOptions,
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
   * OnInterestCallback(), this does not use it. This copies the function object, so you
   * may need to use func_lib::ref() as appropriate.
   * @param registrationOptions (optional) See Face::registerPrefix.
   * @param wireFormat (optional) See Face::registerPrefix.
   */
  void
  registerPrefix
    (const Name& prefix, const OnRegisterFailed& onRegisterFailed,
     const OnRegisterSuccess& onRegisterSuccess,
     const OnInterestCallback& onDataNotFound = OnInterestCallback(),
     const RegistrationOptions& registrationOptions = RegistrationOptions(),
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    impl_->registerPrefix
      (prefix, onRegisterFailed, onRegisterSuccess, onDataNotFound, registrationOptions,
       wireFormat);
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
   * OnInterestCallback(), this does not use it. This copies the function object, so you
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
    impl_->setInterestFilter(filter, onDataNotFound);
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
   * OnInterestCallback(), this does not use it. This copies the function object, so you
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
    impl_->setInterestFilter(prefix, onDataNotFound);
  }

  /**
   * Call Face.unsetInterestFilter and Face.removeRegisteredPrefix for all the
   * prefixes given to the setInterestFilter and registerPrefix method on this
   * MemoryContentCache object so that it will not receive interests any more.
   * You can call this if you want to "shut down" this MemoryContentCache while
   * your application is still running.
   */
  void
  unregisterAll() { impl_->unregisterAll(); }

  /**
   * Add the Data packet to the cache so that it is available to use to
   * answer interests. If data.getMetaInfo().getFreshnessPeriod() is not
   * negative, set the staleness time to now plus the maximum of
   * data.getMetaInfo().getFreshnessPeriod() and minimumCacheLifetime, which is
   * checked during cleanup to remove stale content.
   * This also checks if cleanupIntervalMilliseconds
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
  add(const Data& data) { impl_->add(data); }

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
    (const ptr_lib::shared_ptr<const Interest>& interest, Face& face)
  {
    impl_->storePendingInterest(interest, face);
  }

  /**
   * Return a callback to use for onDataNotFound in registerPrefix which simply
   * calls storePendingInterest() to store the interest that doesn't match a
   * Data packet. add(data) will check if the added Data packet satisfies any
   * pending interest and send it.
   * @return A callback to use for onDataNotFound in registerPrefix().
   */
  const OnInterestCallback&
  getStorePendingInterest() { return impl_->getStorePendingInterest(); }

  /**
   * Remove timed-out pending interests, then for each pending interest which
   * matches according to Interest.matchesName(name), append the PendingInterest
   * entry to the given pendingInterests list. (To get interests with a given
   * prefix, see getPendingInterestsWithPrefix().)
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
     std::vector<ptr_lib::shared_ptr<const PendingInterest> >& pendingInterests)
  {
    impl_->getPendingInterestsForName(name, pendingInterests);
  }

  /**
   * Remove timed-out pending interests, then for each pending interest which
   * matches according to prefix.isPrefixOf(interest.getName()), append the
   * PendingInterest entry to the given pendingInterests list. Note that
   * interest selectors are ignored. (To get interests which would match a
   * given data packet name, see getPendingInterestsForName().)
   * Because this modifies the internal tables, you should call this on the same
   * thread as processEvents, which can also modify the tables.
   * @param prefix The prefix of the interest names to match.
   * @param pendingInterests The vector to receive the matching PendingInterest
   * objects. This first clears the list before adding objects. You should not
   * modify the PendingInterest objects.
   */
  void
  getPendingInterestsWithPrefix
    (const Name& prefix,
     std::vector<ptr_lib::shared_ptr<const PendingInterest> >& pendingInterests)
  {
    impl_->getPendingInterestsWithPrefix(prefix, pendingInterests);
  }

  /**
   * Set the OnContentRemoved callback to call when stale content is removed
   * from the cache during cleanup. Note: Because onContentRemoved is called
   * while processing incoming Interests, it should return quickly to allow the
   * Interest to be processed quickly.
   * @param onContentRemoved This calls onContentRemoved(contentList) where
   * contentList is the list of MemoryContentCache::Content objects that were
   * removed, where each Content has the Name and encoding of the removed
   * content.
   */
  void
  setOnContentRemoved(const OnContentRemoved& onContentRemoved)
  {
    impl_->setOnContentRemoved(onContentRemoved);
  }

  /**
   * Get the minimum lifetime before removing stale content from the cache.
   * @return The minimum cache lifetime in milliseconds.
   */
  Milliseconds
  getMinimumCacheLifetime() { return impl_->getMinimumCacheLifetime(); }

  /**
   * Set the minimum lifetime before removing stale content from the cache which
   * can keep content in the cache longer than the lifetime defined in the meta
   * info. This can be useful for matching interests where MustBeFresh is false.
   * The default minimum cache lifetime is zero, meaning that content is removed
   * when its lifetime expires.
   * @param minimumCacheLifetime The minimum cache lifetime in milliseconds.
   */
  void
  setMinimumCacheLifetime(Milliseconds minimumCacheLifetime)
  {
    impl_->setMinimumCacheLifetime(minimumCacheLifetime);
  }

private:
  /**
   * MemoryContentCache::Impl does the work of MemoryContentCache. It is a
   * separate class so that MemoryContentCache can create an instance in a
   * shared_ptr to use in callbacks.
   */
  class Impl : public ptr_lib::enable_shared_from_this<Impl> {
  public:
    /**
     * Create a new Impl, which should belong to a shared_ptr. Then you must
     * call initialize().  See the MemoryContentCache constructor for parameter
     * documentation.
     */
    Impl(Face* face, Milliseconds cleanupIntervalMilliseconds);

    /**
     * Complete the work of the constructor. This is needed because we can't
     * call shared_from_this() in the constructor.
     */
    void
    initialize();

    void
    registerPrefix
      (const Name& prefix, const OnRegisterFailed& onRegisterFailed,
       const OnRegisterSuccess& onRegisterSuccess,
       const OnInterestCallback& onDataNotFound,
       const RegistrationOptions& registrationOptions, WireFormat& wireFormat);

    void
    setInterestFilter
      (const InterestFilter& filter, const OnInterestCallback& onDataNotFound);

    void
    setInterestFilter
      (const Name& prefix, const OnInterestCallback& onDataNotFound);

    void
    unregisterAll();

    void
    add(const Data& data);

    void
    storePendingInterest
      (const ptr_lib::shared_ptr<const Interest>& interest, Face& face);

    const OnInterestCallback&
    getStorePendingInterest() { return storePendingInterestCallback_; }

    void
    getPendingInterestsForName
      (const Name& name,
       std::vector<ptr_lib::shared_ptr<const PendingInterest> >& pendingInterests);

    void
    getPendingInterestsWithPrefix
      (const Name& prefix,
       std::vector<ptr_lib::shared_ptr<const PendingInterest> >& pendingInterests);

    void
    setOnContentRemoved(const OnContentRemoved& onContentRemoved)
    {
      onContentRemoved_ = onContentRemoved;
    }

    Milliseconds
    getMinimumCacheLifetime() { return minimumCacheLifetime_; }

    void
    setMinimumCacheLifetime(Milliseconds minimumCacheLifetime)
    {
      minimumCacheLifetime_ = minimumCacheLifetime;
    }

    /**
     * This is the OnInterestCallback which is called when the library receives
     * an interest whose name has the prefix given to registerPrefix. First
     * check if cleanupIntervalMilliseconds milliseconds have passed and remove
     * stale content from the cache. Then search the cache for the Data packet,
     * matching any interest selectors including ChildSelector, and send the
     * Data packet to the transport. If no matching Data packet is in the cache,
     * call the callback in onDataNotFoundForPrefix_ (if defined).
     */
    void
    onInterest
      (const ptr_lib::shared_ptr<const Name>& prefix,
       const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
       uint64_t interestFilterId,
       const ptr_lib::shared_ptr<const InterestFilter>& filter);

  private:
    /**
     * StaleTimeContent extends Content to include the cacheRemovalTimeMilliseconds_
     * for when this entry should be cleaned up from the cache.
     */
    class StaleTimeContent : public Content {
    public:
      /**
       * Create a new StaleTimeContent to hold data's name and wire encoding
       * as well as the cacheRemovalTimeMilliseconds_ which is now plus the
       * maximum of data.getMetaInfo().getFreshnessPeriod() and the
       * minimumCacheLifetime.
       * @param data The Data packet whose name and wire encoding are copied.
       * @param nowMilliseconds The current time in milliseconds from
       * ndn_getNowMilliseconds.
       * @param minimumCacheLifetime The minimum cache lifetime in milliseconds.
       */
      StaleTimeContent
        (const Data& data, MillisecondsSince1970 nowMilliseconds,
         Milliseconds minimumCacheLifetime);

      /**
       * Check if this content is stale and should be removed from the cache,
       * according to the content freshness period and the minimumCacheLifetime.
       * @param nowMilliseconds The current time in milliseconds from
       * ndn_getNowMilliseconds.
       * @return True if this content should be removed, otherwise false.
       */
      bool
      isPastRemovalTime(MillisecondsSince1970 nowMilliseconds) const
      {
        return cacheRemovalTimeMilliseconds_ <= nowMilliseconds;
      }

      /**
       * Check if the content is still fresh according to its freshness period
       * (independent of when to remove from the cache).
       * @param nowMilliseconds The current time in milliseconds from
       * ndn_getNowMilliseconds.
       * @return True if the content is still fresh, otherwise false.
       */
      bool
      isFresh(MillisecondsSince1970 nowMilliseconds) const
      {
        return freshnessExpiryTimeMilliseconds_ > nowMilliseconds;
      }

      /**
       * Compare shared_ptrs to Content based only on cacheRemovalTimeMilliseconds_.
       */
      class Compare {
      public:
        bool
        operator()
          (const ptr_lib::shared_ptr<const StaleTimeContent>& x,
           const ptr_lib::shared_ptr<const StaleTimeContent>& y) const
        {
          return x->cacheRemovalTimeMilliseconds_ < y->cacheRemovalTimeMilliseconds_;
        }
      };

    private:
      MillisecondsSince1970 cacheRemovalTimeMilliseconds_; /**< The time when the content
        becomes stale and should be removed from the cache in milliseconds
        according to ndn_getNowMilliseconds */
      MillisecondsSince1970 freshnessExpiryTimeMilliseconds_; /**< The time when
        the freshness period of the content expires (independent of when to
        remove from the cache) in milliseconds according to ndn_getNowMilliseconds */
    };

    /**
     * Check if now is greater than nextCleanupTime_ and, if so, remove stale
     * content from staleTimeCache_ and reset nextCleanupTime_ based on
     * cleanupIntervalMilliseconds_. Since add(Data) does a sorted insert into
     * staleTimeCache_, the check for stale data is quick and does not require
     * searching the entire staleTimeCache_. If onContentRemoved_ is defined,
     * this calls onContentRemoved_(content) for the removed content.
     * @param nowMilliseconds The current time in milliseconds from
     * ndn_getNowMilliseconds.
     */
    void
    doCleanup(MillisecondsSince1970 nowMilliseconds);

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
    OnContentRemoved onContentRemoved_;
    bool isDoingCleanup_;
    Milliseconds minimumCacheLifetime_;
  };

  ptr_lib::shared_ptr<Impl> impl_;
};

}

#endif
