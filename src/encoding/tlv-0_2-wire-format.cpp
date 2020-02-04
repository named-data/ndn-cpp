/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2020 Regents of the University of California.
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
#include <ndn-cpp/interest.hpp>
#include <ndn-cpp/lite/encoding/tlv-0_2-wire-format-lite.hpp>
#include "tlv-encoder.hpp"
#include <ndn-cpp/encoding/tlv-0_2-wire-format.hpp>

using namespace std;

namespace ndn {

static bool didCanBePrefixWarning_ = false;

Blob
Tlv0_2WireFormat::encodeInterest
  (const Interest& interest, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset)
{
  if (!interest.getDidSetCanBePrefix_() && !didCanBePrefixWarning_) {
    printf
      ("WARNING: The default CanBePrefix will change. See Interest::setDefaultCanBePrefix() for details.\n");
    didCanBePrefixWarning_ = true;
  }

  struct ndn_NameComponent nameComponents[100];
  struct ndn_ExcludeEntry excludeEntries[100];
  struct ndn_NameComponent keyNameComponents[100];
  InterestLite interestLite
    (nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]),
     excludeEntries, sizeof(excludeEntries) / sizeof(excludeEntries[0]),
     keyNameComponents, sizeof(keyNameComponents) / sizeof(keyNameComponents[0]));
  Blob blob1, blob2;
  interest.get(interestLite, *this, blob1, blob2);

  DynamicUInt8Vector output(256);
  ndn_Error error;
  size_t encodingLength;
  if ((error = Tlv0_2WireFormatLite::encodeInterest
       (interestLite, signedPortionBeginOffset, signedPortionEndOffset,
        DynamicUInt8ArrayLite::downCast(output), &encodingLength)))
    throw runtime_error(ndn_getErrorString(error));

  return output.finish(encodingLength);
}

void
Tlv0_2WireFormat::decodeInterest
  (Interest& interest, const uint8_t *input, size_t inputLength,
   size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_ExcludeEntry excludeEntries[100];
  struct ndn_NameComponent keyNameComponents[100];
  InterestLite interestLite
    (nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]),
     excludeEntries, sizeof(excludeEntries) / sizeof(excludeEntries[0]),
     keyNameComponents, sizeof(keyNameComponents) / sizeof(keyNameComponents[0]));

  ndn_Error error;
  if ((error = Tlv0_2WireFormatLite::decodeInterest
       (interestLite, input, inputLength, signedPortionBeginOffset,
        signedPortionEndOffset)))
    throw runtime_error(ndn_getErrorString(error));

  if (interestLite.getForwardingHintWireEncoding().buf()) {
    // Throw any decoding exceptions now before calling set.
    DelegationSet delegationSet;
    decodeDelegationSet
      (delegationSet, interestLite.getForwardingHintWireEncoding().buf(),
       interestLite.getForwardingHintWireEncoding().size());
  }

  interest.set(interestLite, *this);
}

Tlv0_2WireFormat* Tlv0_2WireFormat::instance_ = 0;

}
