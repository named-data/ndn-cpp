/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#ifndef NDN_TLV_INTEREST_H
#define NDN_TLV_INTEREST_H

#include "../../errors.h"
#include "../../interest.h"
#include "tlv-encoder.h"
#include "tlv-decoder.h"

#ifdef  __cplusplus
extern "C" {
#endif

ndn_Error 
ndn_encodeTlvInterest
  (struct ndn_Interest *interest, size_t *signedPortionBeginOffset, 
   size_t *signedPortionEndOffset, struct ndn_TlvEncoder *encoder);

ndn_Error 
ndn_decodeTlvInterest(struct ndn_Interest *interest, struct ndn_TlvDecoder *decoder);

#ifdef  __cplusplus
}
#endif

#endif
