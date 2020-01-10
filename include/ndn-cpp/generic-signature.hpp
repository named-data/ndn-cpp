/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2020 Regents of the University of California.
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

#ifndef NDN_GENERIC_SIGNATURE_HPP
#define NDN_GENERIC_SIGNATURE_HPP

#include "signature.hpp"

namespace ndn {

/**
 * A GenericSignature extends Signature and holds the encoding bytes of the
 * SignatureInfo so that the application can process experimental signature
 * types. When decoding a packet, if the type of SignatureInfo is not
 * recognized, the library creates a GenericSignature.
 */
class GenericSignature : public Signature {
public:
  /**
   * Create a new GenericSignature with default values.
   */
  GenericSignature()
  : changeCount_(0),
    typeCode_(-1)
  {
  }

  /**
   * Return a pointer to a new DigestSha256Signature which is a copy of this
   * GenericSignature.
   * @return A new GenericSignature.
   */
  virtual ptr_lib::shared_ptr<Signature>
  clone() const;

  /**
   * Set signatureLite to point to the values in this signature object, without
   * copying any memory.
   * WARNING: The resulting pointers in signatureLite are invalid after a
   * further use of this object which could reallocate memory.
   * @param signatureLite A SignatureLite object where the name components array
   * is already allocated.
   */
  virtual void
  get(SignatureLite& signatureLite) const;

  /**
   * Clear this signature, and set the values by copying from signatureLite.
   * @param signatureLite A SignatureLite object.
   */
  virtual void
  set(const SignatureLite& signatureLite);

  /**
   * Get the bytes of the entire signature info encoding (including the type
   * code).
   * @return The encoding bytes. If not specified, the value isNull().
   */
  const Blob&
  getSignatureInfoEncoding() const { return signatureInfoEncoding_; }

  /**
   * Set the bytes of the entire signature info encoding (including the type
   * code).
   * @param signatureInfoEncoding A Blob with the encoding bytes.
   * @param typeCode (optional) The type code of the signature type, or -1 if
   * not known. (When a GenericSignature is created by wire decoding, it sets
   * the typeCode.)
   */
  void
  setSignatureInfoEncoding(const Blob& signatureInfoEncoding, int typeCode = -1)
  {
    signatureInfoEncoding_ = signatureInfoEncoding;
    typeCode_ = typeCode;

    ++changeCount_;
  }

  /**
   * Get the signature bytes.
   * @return The signature bytes. If not specified, the value isNull().
   */
  virtual const Blob&
  getSignature() const;

  /**
   * Set the signature bytes to the given value.
   * @param signature A Blob with the signature bytes.
   */
  virtual void
  setSignature(const Blob& signature);

  /**
   * Get the type code of the signature type. When wire decode calls
   * setSignatureInfoEncoding, it sets the type code. Note that the type code
   * is ignored during wire encode, which simply uses getSignatureInfoEncoding()
   * where the encoding already has the type code.
   * @return The type code, or -1 if not known.
   */
  int
  getTypeCode() const { return typeCode_; }

  /**
   * Clear all the fields.
   */
  void
  clear()
  {
    signature_.reset();
    signatureInfoEncoding_.reset();
    typeCode_ = -1;
    ++changeCount_;
  }

  /**
   * Get the change count, which is incremented each time this object is changed.
   * @return The change count.
   */
  virtual uint64_t
  getChangeCount() const;

private:
  Blob signature_;
  Blob signatureInfoEncoding_;
  int typeCode_;
  uint64_t changeCount_;
};


}

#endif
