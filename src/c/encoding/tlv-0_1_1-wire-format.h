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

#ifndef NDN_TLV_0_1_1_WIRE_FORMAT_H
#define NDN_TLV_0_1_1_WIRE_FORMAT_H

#include <ndn-cpp/c/common.h>
#include <ndn-cpp/c/errors.h>

struct ndn_Interest;
struct ndn_Data;
struct ndn_DynamicUInt8Array;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Encode interest as NDN-TLV.
 * @param interest A pointer to the interest object to encode.
 * @param signedPortionBeginOffset Return the offset in the encoding of the
 * beginning of the signed portion. The signed portion starts from the first
 * name component and ends just before the final name component (which is
 * assumed to be a signature for a signed interest).
 * @param signedPortionEndOffset Return the offset in the encoding of the end
 * of the signed portion. The signed portion starts from the first
 * name component and ends just before the final name component (which is
 * assumed to be a signature for a signed interest).
 * @param output A pointer to a ndn_DynamicUInt8Array struct which receives the
 * encoded output.  If the output->realloc function pointer is null, its array
 * must be large enough to receive the entire encoding.
 * @param encodingLength Set encodingLength to the length of the encoded output.
 * @return 0 for success, else an error code.
 */
ndn_Error
ndn_Tlv0_1_1WireFormat_encodeInterest
  (const struct ndn_Interest *interest, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset, struct ndn_DynamicUInt8Array *output,
   size_t *encodingLength);

/**
 * Decode input as an interest in NDN-TLV and set the fields of the interest
 * object.
 * @param interest A pointer to the Interest object whose fields are updated.
 * @param input A pointer to the input buffer to decode.
 * @param inputLength The number of bytes in input.
 * @param signedPortionBeginOffset Return the offset in the encoding of the
 * beginning of the signed portion. The signed portion starts from the first
 * name component and ends just before the final name component (which is
 * assumed to be a signature for a signed interest).
 * @param signedPortionEndOffset Return the offset in the encoding of the end
 * of the signed portion. The signed portion starts from the first
 * name component and ends just before the final name component (which is
 * assumed to be a signature for a signed interest).
 * @return 0 for success, else an error code.
 */
ndn_Error
ndn_Tlv0_1_1WireFormat_decodeInterest
  (struct ndn_Interest *interest, const uint8_t *input, size_t inputLength,
   size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset);

/**
 * Encode the data packet as NDN-TLV.
 * @param data A pointer to the data object to encode.
 * @param signedPortionBeginOffset Return the offset in the encoding of the
 * beginning of the signed portion. If you are not encoding in order to sign,
 * you can ignore this returned value.
 * @param signedPortionEndOffset Return the offset in the encoding of the end of
 * the signed portion. If you are not encoding in order to sign, you can ignore
 * this returned value.
 * @param output A pointer to a ndn_DynamicUInt8Array struct which receives the
 * encoded output.  If the output->realloc function pointer is null, its array
 * must be large enough to receive the entire encoding.
 * @param encodingLength Set encodingLength to the length of the encoded output.
 * @return 0 for success, else an error code.
 */
ndn_Error
ndn_Tlv0_1_1WireFormat_encodeData
  (const struct ndn_Data *data, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset, struct ndn_DynamicUInt8Array *output,
   size_t *encodingLength);

/**
 * Decode input as a data packet in NDN-TLV and set the fields in the data object.
 * @param data A pointer to the data object whose fields are updated.
 * @param input A pointer to the input buffer to decode.
 * @param inputLength The number of bytes in input.
 * @param signedPortionBeginOffset Return the offset in the input buffer of the
 * beginning of the signed portion. If you are not decoding in order to verify,
 * you can ignore this returned value.
 * @param signedPortionEndOffset Return the offset in the input buffer of the end
 * of the signed portion. If you are not decoding in order to verify, you can
 * ignore this returned value.
 * @return 0 for success, else an error code.
 */
ndn_Error
ndn_Tlv0_1_1WireFormat_decodeData
  (struct ndn_Data *data, const uint8_t *input, size_t inputLength,
   size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset);

#ifdef __cplusplus
}
#endif

#endif
