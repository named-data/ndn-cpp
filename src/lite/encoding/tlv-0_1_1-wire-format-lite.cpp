/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015-2016 Regents of the University of California.
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

#include "../../c/encoding/tlv-0_1_1-wire-format.h"
#include <ndn-cpp/lite/encoding/tlv-0_1_1-wire-format-lite.hpp>

namespace ndn {

ndn_Error
Tlv0_1_1WireFormatLite::encodeName
  (const NameLite& name, size_t* signedPortionBeginOffset,
   size_t* signedPortionEndOffset, DynamicUInt8ArrayLite& output,
   size_t* encodingLength)
{
  return ndn_Tlv0_1_1WireFormat_encodeName
    (&name, signedPortionBeginOffset, signedPortionEndOffset,
     &output, encodingLength);
}

ndn_Error
Tlv0_1_1WireFormatLite::decodeName
  (NameLite& name, const uint8_t* input, size_t inputLength,
   size_t* signedPortionBeginOffset, size_t* signedPortionEndOffset)
{
  return ndn_Tlv0_1_1WireFormat_decodeName
    (&name, input, inputLength, signedPortionBeginOffset,
     signedPortionEndOffset);
}

ndn_Error
Tlv0_1_1WireFormatLite::encodeInterest
  (const InterestLite& interest, size_t* signedPortionBeginOffset,
   size_t* signedPortionEndOffset, DynamicUInt8ArrayLite& output,
   size_t* encodingLength)
{
  return ndn_Tlv0_1_1WireFormat_encodeInterest
    (&interest, signedPortionBeginOffset, signedPortionEndOffset,
     &output, encodingLength);
}

ndn_Error
Tlv0_1_1WireFormatLite::decodeInterest
  (InterestLite& interest, const uint8_t* input, size_t inputLength,
   size_t* signedPortionBeginOffset, size_t* signedPortionEndOffset)
{
  return ndn_Tlv0_1_1WireFormat_decodeInterest
    (&interest, input, inputLength, signedPortionBeginOffset,
     signedPortionEndOffset);
}

ndn_Error
Tlv0_1_1WireFormatLite::encodeData
  (const DataLite& data, size_t* signedPortionBeginOffset,
   size_t* signedPortionEndOffset, DynamicUInt8ArrayLite& output,
   size_t* encodingLength)
{
  return ndn_Tlv0_1_1WireFormat_encodeData
    (&data, signedPortionBeginOffset, signedPortionEndOffset, &output,
     encodingLength);
}

ndn_Error
Tlv0_1_1WireFormatLite::decodeData
  (DataLite& data, const uint8_t* input, size_t inputLength,
   size_t* signedPortionBeginOffset, size_t* signedPortionEndOffset)
{
  return ndn_Tlv0_1_1WireFormat_decodeData
    (&data, input, inputLength, signedPortionBeginOffset,
     signedPortionEndOffset);
}

ndn_Error
Tlv0_1_1WireFormatLite::encodeControlParameters
  (const ControlParametersLite& controlParameters,
   DynamicUInt8ArrayLite& output, size_t* encodingLength)
{
  return ndn_Tlv0_1_1WireFormat_encodeControlParameters
    (&controlParameters, &output, encodingLength);
}

ndn_Error
Tlv0_1_1WireFormatLite::decodeControlParameters
  (ControlParametersLite& controlParameters, const uint8_t* input,
   size_t inputLength)
{
  return ndn_Tlv0_1_1WireFormat_decodeControlParameters
    (&controlParameters, input, inputLength);
}

ndn_Error
Tlv0_1_1WireFormatLite::encodeSignatureInfo
  (const SignatureLite& signature, DynamicUInt8ArrayLite& output,
   size_t* encodingLength)
{
  return ndn_Tlv0_1_1WireFormat_encodeSignatureInfo
    (&signature, &output, encodingLength);
}

ndn_Error
Tlv0_1_1WireFormatLite::encodeSignatureValue
  (const SignatureLite& signature, DynamicUInt8ArrayLite& output,
   size_t* encodingLength)
{
  return ndn_Tlv0_1_1WireFormat_encodeSignatureValue
    (&signature, &output, encodingLength);
}

ndn_Error
Tlv0_1_1WireFormatLite::decodeSignatureInfoAndValue
  (SignatureLite& signature, const uint8_t *signatureInfo,
   size_t signatureInfoLength, const uint8_t *signatureValue,
   size_t signatureValueLength)
{
  return ndn_Tlv0_1_1WireFormat_decodeSignatureInfoAndValue
    (&signature, signatureInfo, signatureInfoLength, signatureValue,
     signatureValueLength);
}

}
