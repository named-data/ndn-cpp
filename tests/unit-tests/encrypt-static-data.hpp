/**
 * Copyright (C) 2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt unit tests
 * https://github.com/named-data/name-based-access-control/blob/new/tests/tests/static-data.hpp
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

#ifndef NDN_ENCRYPT_STATIC_DATA_HPP
#define NDN_ENCRYPT_STATIC_DATA_HPP

#include <ndn-cpp/util/blob.hpp>

/**
 * EncryptStaticData has static data blocks used by TestEncryptorV2, etc.
 */
class EncryptStaticData {
public:
  static const EncryptStaticData& get()
  {
    if (!instance_)
      instance_ = new EncryptStaticData();

    return *instance_;
  }

  ndn::Blob userIdentity;
  std::vector<ndn::Blob> managerPackets;
  std::vector<ndn::Blob> encryptedBlobs;
  std::vector<ndn::Blob> encryptorPackets;

private:
  EncryptStaticData();

  static EncryptStaticData *instance_;
};

#endif
