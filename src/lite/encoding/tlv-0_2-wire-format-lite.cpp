/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015-2020 Regents of the University of California.
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

#include "../../c/encoding/tlv-0_2-wire-format.h"
#include <ndn-cpp/lite/encoding/tlv-0_2-wire-format-lite.hpp>

namespace ndn {

ndn_Error
Tlv0_2WireFormatLite::encodeInterest
  (const InterestLite& interest, size_t* signedPortionBeginOffset,
   size_t* signedPortionEndOffset, DynamicUInt8ArrayLite& output,
   size_t* encodingLength)
{
  return ndn_Tlv0_2WireFormat_encodeInterest
    (&interest, signedPortionBeginOffset, signedPortionEndOffset,
     &output, encodingLength);
}

ndn_Error
Tlv0_2WireFormatLite::decodeInterest
  (InterestLite& interest, const uint8_t* input, size_t inputLength,
   size_t* signedPortionBeginOffset, size_t* signedPortionEndOffset)
{
  return ndn_Tlv0_2WireFormat_decodeInterest
    (&interest, input, inputLength, signedPortionBeginOffset,
     signedPortionEndOffset);
}

}
