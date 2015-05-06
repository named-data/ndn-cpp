/**
 * Copyright (C) 2015 Regents of the University of California.
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

#include "tlv/tlv-interest.h"
#include "tlv/tlv-data.h"
#include "tlv-0_1_1-wire-format.h"

ndn_Error
ndn_Tlv0_1_1WireFormat_encodeInterest
  (const struct ndn_Interest *interest, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset, struct ndn_DynamicUInt8Array *output,
   size_t *encodingLength)
{
  struct ndn_TlvEncoder encoder;
  ndn_TlvEncoder_initialize(&encoder, output);
  ndn_Error error = ndn_encodeTlvInterest
    (interest, signedPortionBeginOffset, signedPortionEndOffset, &encoder);
  *encodingLength = encoder.offset;

  return error;
}

ndn_Error
ndn_Tlv0_1_1WireFormat_decodeInterest
  (struct ndn_Interest *interest, const uint8_t *input, size_t inputLength,
   size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset)
{
  struct ndn_TlvDecoder decoder;
  ndn_TlvDecoder_initialize(&decoder, input, inputLength);
  return ndn_decodeTlvInterest
    (interest, signedPortionBeginOffset, signedPortionEndOffset, &decoder);
}

ndn_Error
ndn_Tlv0_1_1WireFormat_encodeData
  (const struct ndn_Data *data, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset, struct ndn_DynamicUInt8Array *output,
   size_t *encodingLength)
{
  struct ndn_TlvEncoder encoder;
  ndn_TlvEncoder_initialize(&encoder, output);
  ndn_Error error = ndn_encodeTlvData
    (data, signedPortionBeginOffset, signedPortionEndOffset, &encoder);
  *encodingLength = encoder.offset;

  return error;
}

ndn_Error
ndn_Tlv0_1_1WireFormat_decodeData
  (struct ndn_Data *data, const uint8_t *input, size_t inputLength,
   size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset)
{
  struct ndn_TlvDecoder decoder;
  ndn_TlvDecoder_initialize(&decoder, input, inputLength);
  return ndn_decodeTlvData
    (data, signedPortionBeginOffset, signedPortionEndOffset, &decoder);
}
