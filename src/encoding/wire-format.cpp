/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
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

#include <stdexcept>
#include <ndn-cpp/encoding/wire-format.hpp>

using namespace std;

namespace ndn {

bool WireFormat::ENABLE_NDNX = false;

static bool gotInitialDefaultWireFormat = false;

WireFormat* WireFormat::defaultWireFormat_ = 0;

WireFormat*
WireFormat::getDefaultWireFormat()
{
  if (!defaultWireFormat_ && !gotInitialDefaultWireFormat) {
    // There is no defaultWireFormat_ and we have not yet initialized initialDefaultWireFormat_, so initialize and use it.
    gotInitialDefaultWireFormat = true;
    // NOTE: This allocates one object which we never free for the life of the application.
    defaultWireFormat_ = newInitialDefaultWireFormat();
  }

  return defaultWireFormat_;
}

Blob
WireFormat::encodeName(const Name& name)
{
  throw logic_error("unimplemented");
}

void
WireFormat::decodeName
  (Name& name, const uint8_t *input, size_t inputLength)
{
  throw logic_error("unimplemented");
}

Blob
WireFormat::encodeInterest
  (const Interest& interest, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset)
{
  throw logic_error("unimplemented");
}

void
WireFormat::decodeInterest
  (Interest& interest, const uint8_t *input, size_t inputLength,
   size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset)
{
  throw logic_error("unimplemented");
}

Blob
WireFormat::encodeData(const Data& data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset)
{
  throw logic_error("unimplemented");
}

void
WireFormat::decodeData
  (Data& data, const uint8_t *input, size_t inputLength, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset)
{
  throw logic_error("unimplemented");
}

Blob
WireFormat::encodeForwardingEntry(const ForwardingEntry& forwardingEntry)
{
  throw logic_error("unimplemented");
}

void
WireFormat::decodeForwardingEntry(ForwardingEntry& forwardingEntry, const uint8_t *input, size_t inputLength)
{
  throw logic_error("unimplemented");
}

Blob
WireFormat::encodeControlParameters(const ControlParameters& controlParameters)
{
  throw logic_error("unimplemented");
}

void
WireFormat::decodeControlParameters
  (ControlParameters& controlParameters, const uint8_t *input,
   size_t inputLength)
{
  throw logic_error("unimplemented");
}

Blob
WireFormat::encodeSignatureInfo(const Signature& signature)
{
  throw logic_error("unimplemented");
}

ptr_lib::shared_ptr<Signature>
WireFormat::decodeSignatureInfoAndValue
  (const uint8_t *signatureInfo, size_t signatureInfoLength,
   const uint8_t *signatureValue, size_t signatureValueLength)
{
  throw logic_error("unimplemented");
}

Blob
WireFormat::encodeSignatureValue(const Signature& signature)
{
  throw logic_error("unimplemented");
}

}
