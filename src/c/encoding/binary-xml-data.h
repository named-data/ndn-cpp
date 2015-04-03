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

#ifndef NDN_BINARY_XML_DATA_H
#define NDN_BINARY_XML_DATA_H

#include "../data.h"
#include "binary-xml-encoder.h"
#include "binary-xml-decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Encode the data packet as binary XML.
 * @param data Pointer to the data object to encode.
 * @param signedPortionBeginOffset Return the offset in the encoding of the beginning of the signed portion.
 * If you are not encoding in order to sign, you can ignore this returned value.
 * @param signedPortionEndOffset Return the offset in the encoding of the end of the signed portion.
 * If you are not encoding in order to sign, you can ignore this returned value.
 * @param encoder Pointer to the ndn_BinaryXmlEncoder struct which receives the encoding.
 * @return 0 for success, else an error code.
 */
ndn_Error ndn_encodeBinaryXmlData
  (struct ndn_Data *data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset, struct ndn_BinaryXmlEncoder *encoder);

/**
 * Decode the data packet as binary XML and set the fields in the data object.
 * @param data Pointer to the data object whose fields are updated.
 * @param signedPortionBeginOffset Return the offset in the input buffer of the beginning of the signed portion.
 * If you are not decoding in order to verify, you can ignore this returned value.
 * @param signedPortionEndOffset Return the offset in the input buffer of the end of the signed portion.
 * If you are not decoding in order to verify, you can ignore this returned value.
 * @param decoder Pointer to the ndn_BinaryXmlDecoder struct which has been initialized with the buffer to decode.
 * @return 0 for success, else an error code.
 */
ndn_Error ndn_decodeBinaryXmlData
  (struct ndn_Data *data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset, struct ndn_BinaryXmlDecoder *decoder);

#ifdef __cplusplus
}
#endif

#endif
