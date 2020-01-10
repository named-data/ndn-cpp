/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2018-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/ims/in-memory-storage-persistent.hpp
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

#ifndef NDN_IN_MEMORY_STORAGE_RETAINING_HPP
#define NDN_IN_MEMORY_STORAGE_RETAINING_HPP

#include <map>
#include "../interest.hpp"
#include "../data.hpp"

// Give friend access to the tests.
class TestEncryptorV2_EnumerateDataFromInMemoryStorage_Test;
class TestAccessManagerV2_EnumerateDataFromInMemoryStorage_Test;

namespace ndn {

/**
 * InMemoryStorageRetaining provides an application cache with in-memory
 * storage, of which no eviction policy will be employed. Entries will only be
 * evicted by explicit application control.
 * Note: In ndn-cxx, this class is called InMemoryStoragePersistent, but
 * "persistent" misleadingly sounds like persistent on-disk storage.
 */
class InMemoryStorageRetaining {
public:
  /**
   * Insert a Data packet. If a Data packet with the same name, including the
   * implicit digest, already exists, replace it.
   * @param data The packet to insert, which is copied.
   */
  void
  insert(const Data& data)
  {
    cache_[*data.getFullName()] = ptr_lib::make_shared<Data>(data);
  }

  /**
   * Find the best match Data for a Name.
   * @param name The Name of the Data packet to find.
   * @return The best match if any, otherwise null. You should not modify the
   * returned object. If you need to modify it then you must make a copy.
   */
  ptr_lib::shared_ptr<Data>
  find(const Name& name);

  /**
   * Find the best match Data for an Interest.
   * @param interest The Interest with the Name of the Data packet to find.
   * @return The best match if any, otherwise null. You should not modify the
   * returned object. If you need to modify it then you must make a copy.
   */
  ptr_lib::shared_ptr<Data>
  find(const Interest& interest)
  {
    // Debug: Check selectors, especially CanBePrefix.
    return find(interest.getName());
  }

  /**
   * Remove matching entries by prefix.
   * @param prefix The prefix Name of the entries to remove.
   */
  void
  remove(const Name& prefix);

  /**
   * Get the number of packets stored in the in-memory storage.
   * @return The number of packets.
   */
  size_t
  size() { return cache_.size(); }

  /**
   * Get the the storage cache, which should only be used for testing.
   * @return The storage cache.
   */
  const std::map<Name, ptr_lib::shared_ptr<Data> >&
  getCache_() { return cache_; }

private:
  // Give friend access to the tests.
  friend class ::TestEncryptorV2_EnumerateDataFromInMemoryStorage_Test;
  friend class ::TestAccessManagerV2_EnumerateDataFromInMemoryStorage_Test;

  std::map<Name, ptr_lib::shared_ptr<Data> > cache_;
};

}

#endif
