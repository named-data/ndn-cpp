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

#ifndef NDN_BINARY_XML_FORWARDING_ENTRY_H
#define NDN_BINARY_XML_FORWARDING_ENTRY_H

#include "../forwarding-entry.h"
#include "binary-xml-encoder.h"
#include "binary-xml-decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Encode the ndn_ForwardingEntry struct using Binary XML.
 * @param forwardingEntry pointer to the ndn_ForwardingEntry struct
 * @param encoder pointer to the ndn_BinaryXmlEncoder struct
 * @return 0 for success, else an error code
 */
ndn_Error ndn_encodeBinaryXmlForwardingEntry(struct ndn_ForwardingEntry *forwardingEntry, struct ndn_BinaryXmlEncoder *encoder);

/**
 * Expect the next element to be a Binary XML ForwardingEntry and decode into the ndn_ForwardingEntry struct.
 * @param forwardingEntry pointer to the ndn_ForwardingEntry struct
 * @param decoder pointer to the ndn_BinaryXmlDecoder struct
 * @return 0 for success, else an error code, including if the next element is not KeyLocator.
 */
ndn_Error ndn_decodeBinaryXmlForwardingEntry(struct ndn_ForwardingEntry *forwardingEntry, struct ndn_BinaryXmlDecoder *decoder);

#ifdef __cplusplus
}
#endif

#endif
