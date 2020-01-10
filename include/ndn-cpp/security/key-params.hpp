/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015-2020 Regents of the University of California.
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

#ifndef NDN_KEY_PARAMS_HPP
#define NDN_KEY_PARAMS_HPP

#include <stdint.h>
#include "../name.hpp"
#include "security-common.hpp"

namespace ndn {

/**
 * KeyParams is a base class for key parameters. Its subclasses are used to
 * store parameters for key generation.
 */
class KeyParams {
public:
  virtual
  ~KeyParams()
  {
  }

  KeyType
  getKeyType() const { return keyType_; }

  KeyIdType
  getKeyIdType() const { return keyIdType_; }

  void
  setKeyId(const Name::Component& keyId) { keyId_ = keyId; }

  const Name::Component&
  getKeyId() const { return keyId_; }

protected:
  /**
   * Create a key generation parameter.
   * @param keyType The type for the created key.
   * @param keyIdType The method for how the key id should be generated, which
   * must not be KEY_ID_TYPE_USER_SPECIFIED.
   * @throws std::runtime_error if keyIdType is KEY_ID_TYPE_USER_SPECIFIED.
   */
  KeyParams(KeyType keyType, KeyIdType keyIdType);

  /**
   * Create a key generation parameter.
   * @param keyType The type for the created key.
   * @param keyId The user-specified key ID. This sets the keyIdType to
   * KEY_ID_TYPE_USER_SPECIFIED. keyId must not be empty.
   * @throws std::runtime_error if keyId is empty.
   */
  KeyParams(KeyType keyType, const Name::Component& keyId);

private:
  KeyType keyType_;
  KeyIdType keyIdType_;
  Name::Component keyId_;
};

class RsaKeyParams : public KeyParams {
public:
  RsaKeyParams
    (const Name::Component& keyId,
     uint32_t size = RsaKeyParams::getDefaultSize())
  : KeyParams(RsaKeyParams::getType(), keyId),
    size_(size)
  {
  }

  RsaKeyParams
    (uint32_t size = RsaKeyParams::getDefaultSize(),
     KeyIdType keyIdType = KEY_ID_TYPE_RANDOM)
  : KeyParams(RsaKeyParams::getType(), keyIdType),
    size_(size)
  {
  }

  uint32_t
  getKeySize() const
  {
    return size_;
  }

  static uint32_t
  getDefaultSize() { return 2048; }

  static KeyType
  getType() { return KEY_TYPE_RSA; }

private:
  uint32_t size_;
};

class EcKeyParams : public KeyParams {
public:
  EcKeyParams
    (const Name::Component& keyId,
     uint32_t size = EcKeyParams::getDefaultSize())
  : KeyParams(EcKeyParams::getType(), keyId),
    size_(size)
  {
  }

  EcKeyParams
    (uint32_t size = EcKeyParams::getDefaultSize(),
     KeyIdType keyIdType = KEY_ID_TYPE_RANDOM)
  : KeyParams(EcKeyParams::getType(), keyIdType),
    size_(size)
  {
  }

  uint32_t
  getKeySize() const
  {
    return size_;
  }

  static uint32_t
  getDefaultSize() { return 256; }

  static KeyType
  getType() { return KEY_TYPE_EC; }

private:
  uint32_t size_;
};


/**
 * @deprecated Use EcKeyParams
 */
class EcdsaKeyParams : public EcKeyParams {
public:
  EcdsaKeyParams
    (const Name::Component& keyId,
     uint32_t size = EcKeyParams::getDefaultSize())
  : EcKeyParams(keyId, size)
  {
  }

  EcdsaKeyParams
    (uint32_t size = EcKeyParams::getDefaultSize(),
     KeyIdType keyIdType = KEY_ID_TYPE_RANDOM)
  : EcKeyParams(size, keyIdType)
  {
  }
};

class AesKeyParams : public KeyParams {
public:
  AesKeyParams
    (const Name::Component& keyId,
     uint32_t size = AesKeyParams::getDefaultSize())
  : KeyParams(AesKeyParams::getType(), keyId),
    size_(size)
  {
  }

  AesKeyParams
    (uint32_t size = AesKeyParams::getDefaultSize(),
     KeyIdType keyIdType = KEY_ID_TYPE_RANDOM)
  : KeyParams(AesKeyParams::getType(), keyIdType),
    size_(size)
  {
  }

  uint32_t
  getKeySize() const
  {
    return size_;
  }

  static uint32_t
  getDefaultSize() { return 64; }

  static KeyType
  getType() { return KEY_TYPE_AES; }

private:
  uint32_t size_;
};

}

#endif
