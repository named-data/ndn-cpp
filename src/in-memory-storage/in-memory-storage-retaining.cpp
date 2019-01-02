/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2018-2019 Regents of the University of California.
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

#include <ndn-cpp/in-memory-storage/in-memory-storage-retaining.hpp>

using namespace std;

namespace ndn {

ptr_lib::shared_ptr<Data>
InMemoryStorageRetaining::find(const Interest& interest)
{
  for (map<Name, ptr_lib::shared_ptr<Data> >::iterator i = cache_.begin();
       i != cache_.end(); ++i) {
    // Debug: Check selectors, especially CanBePrefix.
    if (interest.getName().isPrefixOf(i->first))
      return i->second;
  }

  return ptr_lib::shared_ptr<Data>();
}

}
