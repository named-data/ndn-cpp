/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015 Regents of the University of California.
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
  getKeyType() const
  {
    return keyType_;
  }

protected:
  explicit
  KeyParams(KeyType keyType)
  : keyType_(keyType)
  {
  }

private:
  KeyType keyType_;
};

class RsaKeyParams : public KeyParams {
public:
  explicit
  RsaKeyParams(uint32_t size = RsaKeyParams::getDefaultSize())
  : KeyParams(RsaKeyParams::getType()), size_(size)
  {
  }

  uint32_t
  getKeySize() const
  {
    return size_;
  }

private:
  static uint32_t
  getDefaultSize() { return 2048; }

  static KeyType
  getType() { return KEY_TYPE_RSA; }

  uint32_t size_;
};

class EcdsaKeyParams : public KeyParams {
public:
  explicit
  EcdsaKeyParams(uint32_t size = EcdsaKeyParams::getDefaultSize())
  : KeyParams(EcdsaKeyParams::getType()), size_(size)
  {
  }

  uint32_t
  getKeySize() const
  {
    return size_;
  }

private:
  static uint32_t
  getDefaultSize() { return 256; }

  static KeyType
  getType() { return KEY_TYPE_ECDSA; }

  uint32_t size_;
};

}

#endif
