/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
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

#include <ndn-cpp/util/logging.hpp>
#include <ndn-cpp-tools/usersync/prefix-discovery.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

INIT_LOGGER("ndntools.PrefixDiscovery");

namespace ndntools {

void
PrefixDiscovery::Impl::express()
{
  if (!isEnabled_)
    return;

  face_->expressInterest
    (Name("/localhop/nfd/rib/routable-prefixes"),
     bind(&PrefixDiscovery::Impl::onData, shared_from_this(), _1, _2),
     bind(&PrefixDiscovery::Impl::onTimeout, shared_from_this(), _1));
}

void
PrefixDiscovery::Impl::onData
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data)
{
  ptr_lib::shared_ptr<vector<Name> > responsePrefixes(new vector<Name>());
  bool isDecoded = false;
  try {
    // Decode a sequence of Names.
    size_t offset = 0;
    while (offset < data->getContent().size()) {
      Name name;
      name.wireDecode
        (data->getContent().buf() + offset, data->getContent().size() - offset);
      responsePrefixes->push_back(name);

      // Re-encode the decoded Name to increment the offset.
      offset += name.wireEncode().size();
    }

    isDecoded = true;
  } catch (const std::exception& ex) {
    _LOG_ERROR("PrefixDiscovery: Error decoding Name in response: " << ex.what());
  } catch (...) {
    _LOG_ERROR("PrefixDiscovery: Error decoding Name in response");
  }

  if (isDecoded) {
    // Successfully decoded. Check for changes.
    bool prefixesChanged = false;
    if (responsePrefixes->size() != prefixes_.size())
      prefixesChanged = true;
    else {
      for (size_t i = 0; i < responsePrefixes->size(); ++i) {
        if (!prefixes_[i].equals((*responsePrefixes)[i])) {
          prefixesChanged = true;
          break;
        }
      }
    }

    if (prefixesChanged) {
      // This is a new prefix list. Save a copy and call the application callback.
      prefixes_.clear();
      for (size_t i = 0; i < responsePrefixes->size(); ++i)
        prefixes_.push_back((*responsePrefixes)[i]);

      try {
        onPrefixes_(responsePrefixes);
      } catch (const std::exception& ex) {
        _LOG_ERROR("PrefixDiscovery: Error in onPrefixes: " << ex.what());
      } catch (...) {
        _LOG_ERROR("PrefixDiscovery: Error in onPrefixes");
      }
    }
  }

  face_->callLater
    (periodMilliseconds_,
     bind(&PrefixDiscovery::Impl::express, shared_from_this()));
}

void
PrefixDiscovery::Impl::onTimeout
  (const ptr_lib::shared_ptr<const Interest>& interest)
{
  _LOG_DEBUG("PrefixDiscovery: Time out for routable prefix discovery interest " <<
     interest->getName().toUri());

  face_->callLater
    (periodMilliseconds_,
     bind(&PrefixDiscovery::Impl::express, shared_from_this()));
}

}
