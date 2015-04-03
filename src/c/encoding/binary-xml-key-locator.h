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

#ifndef NDN_BINARY_XML_KEY_LOCATOR_H
#define NDN_BINARY_XML_KEY_LOCATOR_H

#include "../key-locator.h"
#include "binary-xml-encoder.h"
#include "binary-xml-decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Encode the ndn_KeyLocator struct using Binary XML.  If keyLocator->type is -1, then do nothing.
 * @param keyLocator pointer to the ndn_KeyLocator struct
 * @param encoder pointer to the ndn_BinaryXmlEncoder struct
 * @return 0 for success, else an error code
 */
ndn_Error ndn_encodeBinaryXmlKeyLocator(struct ndn_KeyLocator *keyLocator, struct ndn_BinaryXmlEncoder *encoder);

/**
 * Expect the next element to be a Binary XML KeyLocator and decode into the ndn_KeyLocator struct.
 * @param keyLocator pointer to the ndn_KeyLocator struct
 * @param decoder pointer to the ndn_BinaryXmlDecoder struct
 * @return 0 for success, else an error code, including if the next element is not KeyLocator.
 */
ndn_Error ndn_decodeBinaryXmlKeyLocator(struct ndn_KeyLocator *keyLocator, struct ndn_BinaryXmlDecoder *decoder);

/**
 * Peek the next element and if it is a Binary XML KeyLocator and decode into the ndn_KeyLocator struct.
 * Otherwise, set the ndn_KeyLocator struct to none.
 * @param keyLocator pointer to the ndn_KeyLocator struct
 * @param decoder pointer to the ndn_BinaryXmlDecoder struct
 * @return 0 for success, else an error code, including if the next element is not KeyLocator.
 */
ndn_Error ndn_decodeOptionalBinaryXmlKeyLocator(struct ndn_KeyLocator *keyLocator, struct ndn_BinaryXmlDecoder *decoder);

#ifdef __cplusplus
}
#endif

#endif
