/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/encryptor https://github.com/named-data/ndn-group-encrypt
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

#ifndef NDN_ENCRYPTOR_HPP
#define NDN_ENCRYPTOR_HPP

#include "../../data.hpp"
#include "encrypt-params.hpp"

namespace ndn {

/**
 * Encryptor has static constants and utility methods for encryption, such as
 * encryptData.
 */
class Encryptor {
public:
  /**
   * Prepare an encrypted data packet by encrypting the payload using the key
   * according to the params. In addition, this prepares the encoded
   * EncryptedContent with the encryption result using keyName and params. The
   * encoding is set as the content of the data packet. If params defines an
   * asymmetric encryption algorithm and the payload is larger than the maximum
   * plaintext size, this encrypts the payload with a symmetric key that is
   * asymmetrically encrypted and provided as a nonce in the content of the data
   * packet. The packet's /{dataName}/ is updated to be
   * /{dataName}/FOR/{keyName}
   * @param data The data packet which is updated.
   * @param payload The payload to encrypt.
   * @param keyName The key name for the EncryptedContent.
   * @param key The encryption key value.
   * @param params The parameters for encryption.
   */
  static void
  encryptData
    (Data& data, const Blob& payload, const Name& keyName, const Blob& key,
     const EncryptParams& params);

  static const Name::Component&
  getNAME_COMPONENT_FOR() { return getValues().NAME_COMPONENT_FOR; }

  static const Name::Component&
  getNAME_COMPONENT_READ() { return getValues().NAME_COMPONENT_READ; }

  static const Name::Component&
  getNAME_COMPONENT_SAMPLE() { return getValues().NAME_COMPONENT_SAMPLE; }

  static const Name::Component&
  getNAME_COMPONENT_ACCESS() { return getValues().NAME_COMPONENT_ACCESS; }

  static const Name::Component&
  getNAME_COMPONENT_E_KEY() { return getValues().NAME_COMPONENT_E_KEY; }

  static const Name::Component&
  getNAME_COMPONENT_D_KEY() { return getValues().NAME_COMPONENT_D_KEY; }

  static const Name::Component&
  getNAME_COMPONENT_C_KEY() { return getValues().NAME_COMPONENT_C_KEY; }

private:
  /**
   * Values holds values used by the static member values_.
   */
  class Values {
  public:
    Values()
    : NAME_COMPONENT_FOR("FOR"),
      NAME_COMPONENT_READ("READ"),
      NAME_COMPONENT_SAMPLE("SAMPLE"),
      NAME_COMPONENT_ACCESS("ACCESS"),
      NAME_COMPONENT_E_KEY("E-KEY"),
      NAME_COMPONENT_D_KEY("D-KEY"),
      NAME_COMPONENT_C_KEY("C-KEY")
    {}

    Name::Component NAME_COMPONENT_FOR;
    Name::Component NAME_COMPONENT_READ;
    Name::Component NAME_COMPONENT_SAMPLE;
    Name::Component NAME_COMPONENT_ACCESS;
    Name::Component NAME_COMPONENT_E_KEY;
    Name::Component NAME_COMPONENT_D_KEY;
    Name::Component NAME_COMPONENT_C_KEY;
  };

  /**
   * Get the static Values object, creating it if needed. We do this explicitly
   * because some C++ environments don't handle static constructors well.
   * @return The static Values object.
   */
  static Values&
  getValues()
  {
    if (!values_)
      values_ = new Values();

    return *values_;
  }

  static Values* values_;
};

}

#endif
