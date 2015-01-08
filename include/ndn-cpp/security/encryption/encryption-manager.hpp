/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Yingdi Yu <yingdi@cs.ucla.edu>
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

#ifndef NDN_ENCRYPTION_MANAGER_HPP
#define NDN_ENCRYPTION_MANAGER_HPP

#include "../../name.hpp"
#include "../security-common.hpp"

namespace ndn {

class EncryptionManager {
public:
  virtual ~EncryptionManager() {}

  virtual void
  createSymmetricKey(const Name& keyName, KeyType keyType, const Name& signkeyName = Name(), bool isSymmetric = true) = 0;

  virtual Blob
  encrypt(const Name& keyName, const uint8_t* data, size_t dataLength, bool useSymmetric = false,
          EncryptMode encryptMode = ENCRYPT_MODE_DEFAULT) = 0;

  virtual Blob
  decrypt(const Name& keyName, const uint8_t* data, size_t dataLength, bool useSymmetric = false,
          EncryptMode encryptMode = ENCRYPT_MODE_DEFAULT) = 0;
};

}

#endif
