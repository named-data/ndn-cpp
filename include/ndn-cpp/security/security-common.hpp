/**
 * Copyright (C) 2013-2014 Regents of the University of California.
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#ifndef NDN_SECURITY_COMMON_HPP
#define NDN_SECURITY_COMMON_HPP

namespace ndn {

enum KeyType {
  KEY_TYPE_RSA,
  // KEY_TYPE_DSA,
  KEY_TYPE_AES,
  // KEY_TYPE_DES,
  // KEY_TYPE_RC4,
  // KEY_TYPE_RC2
  KEY_TYPE_EC
};

enum KeyClass {
  KEY_CLASS_PUBLIC,
  KEY_CLASS_PRIVATE,
  KEY_CLASS_SYMMETRIC
};
  
enum DigestAlgorithm {
  // DIGEST_ALGORITHM_MD2,
  // DIGEST_ALGORITHM_MD5,
  // DIGEST_ALGORITHM_SHA1,
  DIGEST_ALGORITHM_SHA256
};

enum EncryptMode {
  ENCRYPT_MODE_DEFAULT,
  ENCRYPT_MODE_CFB_AES,
  // ENCRYPT_MODE_CBC_AES
};

}

#endif
