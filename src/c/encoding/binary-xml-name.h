/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_BINARYXMLNAME_H
#define NDN_BINARYXMLNAME_H

#include "../errors.h"
#include "../name.h"
#include "binary-xml-encoder.h"
#include "binary-xml-decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Encode the name as binary XML.
 * @param name A pointer to the name object to encode.
 * @param signedPortionBeginOffset Return the offset in the encoding of the 
 * beginning of the signed portion. The signed portion starts from the first
 * name component and ends just before the final name component (which is
 * assumed to be a signature for a signed interest).
 * If you are not encoding in order to sign, you can ignore this returned value.
 * @param signedPortionEndOffset Return the offset in the encoding of the end of 
 * the signed portion. The signed portion starts from the first
 * name component and ends just before the final name component (which is
 * assumed to be a signature for a signed interest).
 * If you are not encoding in order to sign, you can ignore this returned value.
 * @param encoder A pointer to the ndn_BinaryXmlEncoder struct which receives 
 * the encoding. 
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_encodeBinaryXmlName
  (struct ndn_Name *name, size_t *signedPortionBeginOffset, 
   size_t *signedPortionEndOffset, struct ndn_BinaryXmlEncoder *encoder);

ndn_Error 
ndn_decodeBinaryXmlName
  (struct ndn_Name *name, struct ndn_BinaryXmlDecoder *decoder);

#ifdef __cplusplus
}
#endif

#endif

