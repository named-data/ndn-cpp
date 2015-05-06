/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
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

#ifndef NDN_DIGEST_SHA256_SIGNATURE_HPP
#define NDN_DIGEST_SHA256_SIGNATURE_HPP

#include "signature.hpp"

namespace ndn {

/**
 * A DigestSha256Signature extends Signature and holds the signature bits (which
 * are only the SHA256 digest) and an empty SignatureInfo for a data packet or
 * signed interest.
 */
class DigestSha256Signature : public Signature {
public:
  DigestSha256Signature()
  : changeCount_(0)
  {
  }

  /**
   * Return a pointer to a new DigestSha256Signature which is a copy of this
   * signature.
   */
  virtual ptr_lib::shared_ptr<Signature>
  clone() const;

  /**
   * Set the signatureStruct to point to the values in this signature object,
   * without copying any memory.
   * WARNING: The resulting pointers in signatureStruct are invalid after a
   * further use of this object which could reallocate memory.
   * @param signatureStruct a C ndn_Signature struct where the name components
   * array is already allocated.
   */
  virtual void
  get(struct ndn_Signature& signatureStruct) const;

  /**
   * Clear this signature, and set the values by copying from the ndn_Signature
   * struct.
   * @param signatureStruct a C ndn_Signature struct
   */
  virtual void
  set(const struct ndn_Signature& signatureStruct);

  /**
   * Get the signature bytes (which are only the digest).
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
   * Clear all the fields.
   */
  void
  clear()
  {
    signature_.reset();
    ++changeCount_;
  }

  /**
   * Get the change count, which is incremented each time this object is changed.
   * @return The change count.
   */
  virtual uint64_t
  getChangeCount() const
  {
    return changeCount_;
  }

private:
  Blob signature_;
  uint64_t changeCount_;
};


}

#endif
