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

#ifndef NDN_SIGNATURE_HPP
#define NDN_SIGNATURE_HPP

#include "common.hpp"
#include "util/blob.hpp"

struct ndn_Signature;

namespace ndn {

/**
 * A Signature is an abstract base class providing methods to work with the signature information in a Data packet.
 * You must create an object of a subclass, for example Sha256WithRsaSignature.
 */
class Signature {
public:
  /**
   * The virtual destructor.
   */
  virtual
  ~Signature();

  /**
   * Return a pointer to a new Signature which is a copy of this signature.
   * This is pure virtual, the subclass must implement it.
   */
  virtual ptr_lib::shared_ptr<Signature>
  clone() const = 0;

  /**
   * Get the signature bytes.
   * @return The signature bytes. If not specified, the value isNull().
   */
  virtual const Blob&
  getSignature() const = 0;

  /**
   * Set the signature bytes to the given value.
   * @param signature A Blob with the signature bytes.
   */
  virtual void
  setSignature(const Blob& signature) = 0;

  /**
   * Set the signatureStruct to point to the values in this signature object, without copying any memory.
   * WARNING: The resulting pointers in signatureStruct are invalid after a further use of this object which could reallocate memory.
   * This is pure virtual, the subclass must implement it.
   * @param signatureStruct a C ndn_Signature struct where the name components array is already allocated.
   */
  virtual void
  get(struct ndn_Signature& signatureStruct) const = 0;

  /**
   * Clear this signature, and set the values by copying from the ndn_Signature struct.
   * This is pure virtual, the subclass must implement it.
   * @param signatureStruct a C ndn_Signature struct
   */
  virtual void
  set(const struct ndn_Signature& signatureStruct) = 0;

  /**
   * Get the change count, which is incremented each time this object (or a child object) is changed.
   * @return The change count.
   */
  virtual uint64_t
  getChangeCount() const = 0;
};

}

#endif
