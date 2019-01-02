/**
 * Copyright (C) 2013-2019 Regents of the University of California.
 * @author: Yingdi Yu <yingdi@cs.ucla.edu>
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

#ifndef NDN_SECURITY_COMMON_HPP
#define NDN_SECURITY_COMMON_HPP

namespace ndn {

/**
 * The KeyIdType enum represents the type of a KeyId component in a key name.
 */
enum KeyIdType {
  /**
   * USER_SPECIFIED: A user-specified key ID. It is the user's responsibility to
   * ensure the uniqueness of key names.
   */
  KEY_ID_TYPE_USER_SPECIFIED = 0,
  /**
   * SHA256: The SHA256 hash of the public key as the key id. This KeyId type
   * guarantees the uniqueness of key names.
   */
  KEY_ID_TYPE_SHA256 = 1,
  /**
   * RANDOM: A 64-bit random number as the key id. This KeyId provides rough
   * uniqueness of key names.
   */
  KEY_ID_TYPE_RANDOM = 2
};

// The KeyType integer is used by the Sqlite key storage, so don't change them.
// Make these the same as ndn-cxx in case the storage file is shared.
enum KeyType {
  KEY_TYPE_RSA = 0,
  KEY_TYPE_EC = 1,
  /**
   * @deprecated Use KEY_TYPE_EC .
   */
  KEY_TYPE_ECDSA = 1,
  KEY_TYPE_AES   = 128,
};

enum KeyClass {
  KEY_CLASS_PUBLIC,
  KEY_CLASS_PRIVATE,
  KEY_CLASS_SYMMETRIC
};

enum DigestAlgorithm {
  DIGEST_ALGORITHM_SHA256
};

}

#endif
