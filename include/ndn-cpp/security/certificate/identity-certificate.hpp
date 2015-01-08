/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
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

#ifndef NDN_IDENTITY_CERTIFICATE_HPP
#define NDN_IDENTITY_CERTIFICATE_HPP

#include "certificate.hpp"

namespace ndn {

class IdentityCertificate : public Certificate
{
public:
  /**
   * The default constructor.
   */
  IdentityCertificate()
  {
  }

  // Note: The copy constructor works because publicKeyName_ has a copy constructor.

  /**
   * Create an IdentityCertificate from the content in the data packet.
   * @param data The data packet with the content to decode.
   */
  IdentityCertificate(const Data& data);

  /**
   * The copy constructor.
   */
  IdentityCertificate(const IdentityCertificate& identityCertificate);

  /**
   * The virtual destructor.
   */
  virtual
  ~IdentityCertificate();

  /**
   * Override the base class method to check that the name is a valid identity certificate name.
   * @param name The identity certificate name which is copied.
   * @return This Data so that you can chain calls to update values.
   */
  virtual Data &
  setName(const Name& name);

  /**
   * Override to call the base class wireDecode then update the public key name.
   * @param input The input byte array to be decoded as an immutable Blob.
   * @param wireFormat A WireFormat object used to decode the input. If omitted,
   * use WireFormat getDefaultWireFormat().
   */
  virtual void
  wireDecode
    (const Blob& input,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  Name
  getPublicKeyName () const { return publicKeyName_; }

  static bool
  isIdentityCertificate(const Certificate& certificate);

  /**
   * Get the public key name from the full certificate name.
   * @param certificateName The full certificate name.
   * @return The related public key name.
   */
  static Name
  certificateNameToPublicKeyName(const Name& certificateName);

private:
  static bool
  isCorrectName(const Name& name);

  void
  setPublicKeyName();

protected:
  Name publicKeyName_;
};

}

#endif
