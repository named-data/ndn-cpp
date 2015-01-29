/**
 * Copyright (C) 2013-2015 Regents of the University of California.
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

// The KeyType integer is used by the Sqlite key storage, so don't change them.
enum KeyType {
  KEY_TYPE_RSA = 0,
  // KEY_TYPE_DSA,
  KEY_TYPE_AES = 1,
  // KEY_TYPE_DES,
  // KEY_TYPE_RC4,
  // KEY_TYPE_RC2
  KEY_TYPE_ECDSA = 2
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
