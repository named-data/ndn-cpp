/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the PSync library https://github.com/named-data/PSync/blob/master/PSync/detail/state.hpp
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

#include <sstream>
#include "../../encoding/tlv-encoder.hpp"
#include "../../encoding/tlv-decoder.hpp"
#include "../../c/encoding/tlv/tlv-name.h"
#include "psync-state.hpp"

using namespace std;

namespace ndn {

Blob
PSyncState::wireEncode() const
{
  // Encode directly as TLV. We don't support the WireFormat abstraction
  // because this isn't meant to go directly on the wire.
  TlvEncoder encoder(256);

  encoder.writeNestedTlv(Tlv_PSyncContent, encodeContent, this);

  return encoder.finish();
}

void
PSyncState::encodeContent(const void *context, TlvEncoder &encoder)
{
  const PSyncState& pSyncState =  *(const PSyncState *)context;

  for (size_t i = 0; i < pSyncState.content_.size(); ++i) {
    const Name& name = pSyncState.content_[i];
    struct ndn_NameComponent nameComponents[100];
    NameLite nameLite
      (nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]));
    name.get(nameLite);

    ndn_Error error;
    size_t dummyBeginOffset, dummyEndOffset;
    if ((error = ndn_encodeTlvName
         (&nameLite, &dummyBeginOffset, &dummyEndOffset, &encoder)))
      throw runtime_error(ndn_getErrorString(error));

  }
}

void
PSyncState::wireDecode(const uint8_t *input, size_t inputLength)
{
  clear();

  // Decode directly as TLV. We don't support the WireFormat abstraction
  // because this isn't meant to go directly on the wire.
  TlvDecoder decoder(input, inputLength);
  size_t endOffset = decoder.readNestedTlvsStart(Tlv_PSyncContent);

  // Decode a sequence of Name.
  while (decoder.offset < inputLength) {
    struct ndn_NameComponent nameComponents[100];
    NameLite nameLite
      (nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]));

    ndn_Error error;
    size_t dummyBeginOffset, dummyEndOffset;
    if ((error = ndn_decodeTlvName
         (&nameLite, &dummyBeginOffset, &dummyEndOffset, &decoder)))
      throw runtime_error(ndn_getErrorString(error));

    Name name;
    name.set(nameLite);
    content_.push_back(name);
  }

  decoder.finishNestedTlvs(endOffset);
}

string
PSyncState::toString() const
{
  ostringstream result;

  result << "[";

  for (size_t i = 0; i < content_.size(); ++i) {
    result << content_[i];
    if (i < content_.size() - 1)
      result << ", ";
  }

  result << "]";

  return result.str();
}

}
