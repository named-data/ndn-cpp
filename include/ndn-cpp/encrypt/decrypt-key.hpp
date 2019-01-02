/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/decrypt-key https://github.com/named-data/ndn-group-encrypt
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

#ifndef NDN_DECRYPT_KEY_HPP
#define NDN_DECRYPT_KEY_HPP

#include "../util/blob.hpp"

/**
 * A DecryptKey supplies the key for decrypt.
 * @note This class is an experimental feature. The API may change.
 */
namespace ndn {

class DecryptKey {
public:
  /**
   * Create a DecryptKey with the given key value.
   * @param keyBits The key value.
   */
  DecryptKey(const Blob& keyBits)
  : keyBits_(keyBits)
  {}

  /**
   * Get the key value.
   * @return The key value.
   */
  const Blob&
  getKeyBits() const { return keyBits_; }

private:
  Blob keyBits_;
};

}

#endif
