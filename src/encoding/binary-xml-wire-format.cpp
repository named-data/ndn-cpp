/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
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

#include <stdexcept>
#include <ndn-cpp/interest.hpp>
#include <ndn-cpp/data.hpp>
#include <ndn-cpp/forwarding-entry.hpp>
#include "../c/encoding/binary-xml-name.h"
#include "../c/encoding/binary-xml-interest.h"
#include "../c/encoding/binary-xml-data.h"
#include "../c/encoding/binary-xml-forwarding-entry.h"
#include "binary-xml-encoder.hpp"
#include "binary-xml-decoder.hpp"
#include <ndn-cpp/encoding/binary-xml-wire-format.hpp>

using namespace std;

namespace ndn {

BinaryXmlWireFormat* BinaryXmlWireFormat::instance_ = 0;

// NDN_CPP_USE_DEFAULT_TLV takes precedence over NDN_CPP_USE_DEFAULT_BINARY_XML.
#if !(defined(NDN_CPP_USE_DEFAULT_TLV) || !defined(NDN_CPP_USE_DEFAULT_BINARY_XML))
// This is declared in the WireFormat class.
WireFormat*
WireFormat::newInitialDefaultWireFormat()
{
  return BinaryXmlWireFormat::get();
}
#endif

BinaryXmlWireFormat::BinaryXmlWireFormat()
{
  if (!WireFormat::ENABLE_NDNX)
    throw runtime_error
      ("BinaryXmlWireFormat (NDNx) is deprecated. To enable while you upgrade your code to use NDN-TLV, set WireFormat::ENABLE_NDNX = true");
}

Blob
BinaryXmlWireFormat::encodeName(const Name& name)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_Name nameStruct;
  ndn_Name_initialize
    (&nameStruct, nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]));
  name.get(nameStruct);

  BinaryXmlEncoder encoder(256);
  ndn_Error error;
  size_t dummyBeginOffset, dummyEndOffset;
  if ((error = ndn_encodeBinaryXmlName
       (&nameStruct, &dummyBeginOffset, &dummyEndOffset, &encoder)))
    throw runtime_error(ndn_getErrorString(error));

  return Blob(encoder.getOutput(), false);
}

void
BinaryXmlWireFormat::decodeName
  (Name& name, const uint8_t *input, size_t inputLength)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_Name nameStruct;
  ndn_Name_initialize
    (&nameStruct, nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]));

  BinaryXmlDecoder decoder(input, inputLength);
  ndn_Error error;
  size_t dummyBeginOffset, dummyEndOffset;
  if ((error = ndn_decodeBinaryXmlName
       (&nameStruct, &dummyBeginOffset, &dummyEndOffset, &decoder)))
    throw runtime_error(ndn_getErrorString(error));

  name.set(nameStruct);
}

Blob
BinaryXmlWireFormat::encodeInterest
  (const Interest& interest, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_ExcludeEntry excludeEntries[100];
  struct ndn_NameComponent keyNameComponents[100];
  struct ndn_Interest interestStruct;
  ndn_Interest_initialize
    (&interestStruct, nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]),
     excludeEntries, sizeof(excludeEntries) / sizeof(excludeEntries[0]),
     keyNameComponents, sizeof(keyNameComponents) / sizeof(keyNameComponents[0]));
  interest.get(interestStruct);

  BinaryXmlEncoder encoder(256);
  ndn_Error error;
  if ((error = ndn_encodeBinaryXmlInterest
       (&interestStruct, signedPortionBeginOffset, signedPortionEndOffset,
        &encoder)))
    throw runtime_error(ndn_getErrorString(error));

  return Blob(encoder.getOutput(), false);
}

void
BinaryXmlWireFormat::decodeInterest
  (Interest& interest, const uint8_t *input, size_t inputLength,
   size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_ExcludeEntry excludeEntries[100];
  struct ndn_NameComponent keyNameComponents[100];
  struct ndn_Interest interestStruct;
  ndn_Interest_initialize
    (&interestStruct, nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]),
     excludeEntries, sizeof(excludeEntries) / sizeof(excludeEntries[0]),
     keyNameComponents, sizeof(keyNameComponents) / sizeof(keyNameComponents[0]));

  BinaryXmlDecoder decoder(input, inputLength);
  ndn_Error error;
  if ((error = ndn_decodeBinaryXmlInterest
       (&interestStruct, signedPortionBeginOffset, signedPortionEndOffset,
        &decoder)))
    throw runtime_error(ndn_getErrorString(error));

  interest.set(interestStruct);
}

Blob
BinaryXmlWireFormat::encodeData(const Data& data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_NameComponent keyNameComponents[100];
  struct ndn_Data dataStruct;
  ndn_Data_initialize
    (&dataStruct, nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]),
     keyNameComponents, sizeof(keyNameComponents) / sizeof(keyNameComponents[0]));
  data.get(dataStruct);

  BinaryXmlEncoder encoder(1500);
  ndn_Error error;
  if ((error = ndn_encodeBinaryXmlData(&dataStruct, signedPortionBeginOffset, signedPortionEndOffset, &encoder)))
    throw runtime_error(ndn_getErrorString(error));

  return Blob(encoder.getOutput(), false);
}

void
BinaryXmlWireFormat::decodeData
  (Data& data, const uint8_t *input, size_t inputLength, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_NameComponent keyNameComponents[100];
  struct ndn_Data dataStruct;
  ndn_Data_initialize
    (&dataStruct, nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]),
     keyNameComponents, sizeof(keyNameComponents) / sizeof(keyNameComponents[0]));

  BinaryXmlDecoder decoder(input, inputLength);
  ndn_Error error;
  if ((error = ndn_decodeBinaryXmlData(&dataStruct, signedPortionBeginOffset, signedPortionEndOffset, &decoder)))
    throw runtime_error(ndn_getErrorString(error));

  data.set(dataStruct);
}

Blob
BinaryXmlWireFormat::encodeForwardingEntry(const ForwardingEntry& forwardingEntry)
{
  struct ndn_NameComponent prefixNameComponents[100];
  struct ndn_ForwardingEntry forwardingEntryStruct;
  ndn_ForwardingEntry_initialize
    (&forwardingEntryStruct, prefixNameComponents, sizeof(prefixNameComponents) / sizeof(prefixNameComponents[0]));
  forwardingEntry.get(forwardingEntryStruct);

  BinaryXmlEncoder encoder;
  ndn_Error error;
  if ((error = ndn_encodeBinaryXmlForwardingEntry(&forwardingEntryStruct, &encoder)))
    throw runtime_error(ndn_getErrorString(error));

  return Blob(encoder.getOutput(), false);
}

void
BinaryXmlWireFormat::decodeForwardingEntry(ForwardingEntry& forwardingEntry, const uint8_t *input, size_t inputLength)
{
  struct ndn_NameComponent prefixNameComponents[100];
  struct ndn_ForwardingEntry forwardingEntryStruct;
  ndn_ForwardingEntry_initialize
    (&forwardingEntryStruct, prefixNameComponents, sizeof(prefixNameComponents) / sizeof(prefixNameComponents[0]));

  BinaryXmlDecoder decoder(input, inputLength);
  ndn_Error error;
  if ((error = ndn_decodeBinaryXmlForwardingEntry(&forwardingEntryStruct, &decoder)))
    throw runtime_error(ndn_getErrorString(error));

  forwardingEntry.set(forwardingEntryStruct);
}

}
