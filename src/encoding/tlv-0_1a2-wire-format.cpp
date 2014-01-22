/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include <stdexcept>
#include <ndn-cpp/interest.hpp>
#include <ndn-cpp/data.hpp>
#include <ndn-cpp/forwarding-entry.hpp>
#include "../c/encoding/tlv/tlv-interest.h"
#include "../c/encoding/tlv/tlv-data.h"
#if 0
#include "../c/encoding/tlv/tlv-forwarding-entry.h"
#endif
#include "tlv-encoder.hpp"
#include "tlv-decoder.hpp"
#include <ndn-cpp/encoding/tlv-0_1a2-wire-format.hpp>

using namespace std;

namespace ndn {

Blob 
Tlv1_0a2WireFormat::encodeInterest(const Interest& interest) 
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_ExcludeEntry excludeEntries[100];
  struct ndn_Interest interestStruct;
  ndn_Interest_initialize
    (&interestStruct, nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]), 
     excludeEntries, sizeof(excludeEntries) / sizeof(excludeEntries[0]));
  interest.get(interestStruct);

  TlvEncoder encoder;
  ndn_Error error;
  if ((error = ndn_encodeTlvInterest(&interestStruct, &encoder)))
    throw runtime_error(ndn_getErrorString(error));
     
  return encoder.getOutput();
}

void 
Tlv1_0a2WireFormat::decodeInterest(Interest& interest, const uint8_t *input, size_t inputLength)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_ExcludeEntry excludeEntries[100];
  struct ndn_Interest interestStruct;
  ndn_Interest_initialize
    (&interestStruct, nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]), 
     excludeEntries, sizeof(excludeEntries) / sizeof(excludeEntries[0]));
    
  TlvDecoder decoder(input, inputLength);  
  ndn_Error error;
  if ((error = ndn_decodeTlvInterest(&interestStruct, &decoder)))
    throw runtime_error(ndn_getErrorString(error));

  interest.set(interestStruct);
}

Blob 
Tlv1_0a2WireFormat::encodeData(const Data& data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset) 
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_NameComponent keyNameComponents[100];
  struct ndn_Data dataStruct;
  ndn_Data_initialize
    (&dataStruct, nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]), 
     keyNameComponents, sizeof(keyNameComponents) / sizeof(keyNameComponents[0]));
  data.get(dataStruct);

  TlvEncoder encoder(1500);
  ndn_Error error;
  if ((error = ndn_encodeTlvData(&dataStruct, signedPortionBeginOffset, signedPortionEndOffset, &encoder)))
    throw runtime_error(ndn_getErrorString(error));
     
  return encoder.getOutput();
}

void 
Tlv1_0a2WireFormat::decodeData
  (Data& data, const uint8_t *input, size_t inputLength, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_NameComponent keyNameComponents[100];
  struct ndn_Data dataStruct;
  ndn_Data_initialize
    (&dataStruct, nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]), 
     keyNameComponents, sizeof(keyNameComponents) / sizeof(keyNameComponents[0]));
    
  TlvDecoder decoder(input, inputLength);  
  ndn_Error error;
  if ((error = ndn_decodeTlvData(&dataStruct, signedPortionBeginOffset, signedPortionEndOffset, &decoder)))
    throw runtime_error(ndn_getErrorString(error));

  data.set(dataStruct);
}

#if 0
Blob 
Tlv1_0a2WireFormat::encodeForwardingEntry(const ForwardingEntry& forwardingEntry) 
{
  struct ndn_NameComponent prefixNameComponents[100];
  struct ndn_ForwardingEntry forwardingEntryStruct;
  ndn_ForwardingEntry_initialize
    (&forwardingEntryStruct, prefixNameComponents, sizeof(prefixNameComponents) / sizeof(prefixNameComponents[0]));
  forwardingEntry.get(forwardingEntryStruct);

  TlvEncoder encoder;
  ndn_Error error;
  if ((error = ndn_encodeTlvForwardingEntry(&forwardingEntryStruct, &encoder)))
    throw runtime_error(ndn_getErrorString(error));
     
  return encoder.getOutput();
}

void 
Tlv1_0a2WireFormat::decodeForwardingEntry(ForwardingEntry& forwardingEntry, const uint8_t *input, size_t inputLength)
{
  struct ndn_NameComponent prefixNameComponents[100];
  struct ndn_ForwardingEntry forwardingEntryStruct;
  ndn_ForwardingEntry_initialize
    (&forwardingEntryStruct, prefixNameComponents, sizeof(prefixNameComponents) / sizeof(prefixNameComponents[0]));
    
  TlvDecoder decoder(input, inputLength);  
  ndn_Error error;
  if ((error = ndn_decodeTlvForwardingEntry(&forwardingEntryStruct, &decoder)))
    throw runtime_error(ndn_getErrorString(error));

  forwardingEntry.set(forwardingEntryStruct);
}
#endif

}
