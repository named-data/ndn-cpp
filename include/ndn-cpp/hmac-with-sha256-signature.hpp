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

#ifndef NDN_HMAC_WITH_SHA256_SIGNATURE_HPP
#define NDN_HMAC_WITH_SHA256_SIGNATURE_HPP

#include "signature.hpp"
#include "key-locator.hpp"
#include "util/change-counter.hpp"


namespace ndn {

/**
 * An HmacWithSha256Signature extends Signature and holds the signature bits and
 * other info representing an HmacWithSha256 signature in a data packet.
 */
class HmacWithSha256Signature : public Signature {
public:
  HmacWithSha256Signature()
  : changeCount_(0)
  {
  }

  /**
   * Return a pointer to a new HmacWithSha256Signature which is a copy of this
   * signature.
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
   * Get the signature bytes.
   * @return The signature bytes. If not specified, the value isNull().
   */
  virtual const Blob&
  getSignature() const;

  const KeyLocator&
  getKeyLocator() const { return keyLocator_.get(); }

  KeyLocator&
  getKeyLocator() { return keyLocator_.get(); }

  /**
   * Set the signature bytes to the given value.
   * @param signature A Blob with the signature bytes.
   */
  virtual void
  setSignature(const Blob& signature);

  void
  setKeyLocator(const KeyLocator& keyLocator)
  {
    keyLocator_.set(keyLocator);
    ++changeCount_;
  }

  /**
   * Clear all the fields.
   */
  void
  clear()
  {
    signature_.reset();
    keyLocator_.get().clear();
    ++changeCount_;
  }

  /**
   * Get the change count, which is incremented each time this object (or a
   * child object) is changed.
   * @return The change count.
   */
  virtual uint64_t
  getChangeCount() const;

private:
  Blob signature_;
  ChangeCounter<KeyLocator> keyLocator_;
  uint64_t changeCount_;
};

}

#endif
