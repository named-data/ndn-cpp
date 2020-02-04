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

#ifndef NDN_TLV_INTEREST_H
#define NDN_TLV_INTEREST_H

#include "../../interest.h"
#include "tlv-encoder.h"
#include "tlv-decoder.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * Encode the Interest packet as NDN-TLV.
 * @param interest A pointer to the Interest object to encode.
 * @param signedPortionBeginOffset Return the offset in the encoding of the 
 * beginning of the signed portion. If you are not encoding in order to sign,
 * you can ignore this returned value.
 * @param signedPortionEndOffset Return the offset in the encoding of the end of 
 * the signed portion. If you are not encoding in order to sign, you can ignore
 * this returned value.
 * @param encoder Pointer to the ndn_TlvEncoder struct which receives the encoding.
 * @return 0 for success, else an error code.
 */
ndn_Error
ndn_encodeTlvInterest
  (const struct ndn_Interest *interest, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset, struct ndn_TlvEncoder *encoder);

/**
 * Decode the Interest packet as NDN-TLV format v0.3, or if this fails then as
 * format v0.2. Set the fields in the data object.
 * @param interest A pointer to the Interest object whose fields are updated.
 * @param signedPortionBeginOffset Return the offset in the input buffer of the 
 * beginning of the signed portion. If you are not decoding in order to verify,
 * you can ignore this returned value.
 * @param signedPortionEndOffset Return the offset in the input buffer of the 
 * end of the signed portion. If you are not decoding in order to verify, you
 * can ignore this returned value.
 * @param decoder Pointer to the ndn_TlvDecoder struct which has been
 * initialized with the buffer to decode.
 * @return 0 for success, else an error code.
 */
ndn_Error
ndn_decodeTlvInterest
  (struct ndn_Interest *interest, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset, struct ndn_TlvDecoder *decoder);

#ifdef  __cplusplus
}
#endif

#endif
