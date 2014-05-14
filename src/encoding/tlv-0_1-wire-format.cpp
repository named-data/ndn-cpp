/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include <stdexcept>
#include <ndn-cpp/interest.hpp>
#include <ndn-cpp/data.hpp>
#include <ndn-cpp/control-parameters.hpp>
#include "../c/encoding/tlv/tlv-interest.h"
#include "../c/encoding/tlv/tlv-data.h"
#include "../c/encoding/tlv/tlv-control-parameters.h"
#include "tlv-encoder.hpp"
#include "tlv-decoder.hpp"
#include <ndn-cpp/encoding/tlv-0_1-wire-format.hpp>

using namespace std;

namespace ndn {

Blob 
Tlv0_1WireFormat::encodeInterest
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

  TlvEncoder encoder(256);
  ndn_Error error;
  if ((error = ndn_encodeTlvInterest
       (&interestStruct, signedPortionBeginOffset, signedPortionEndOffset, 
        &encoder)))
    throw runtime_error(ndn_getErrorString(error));
     
  return Blob(encoder.getOutput(), false);
}

void 
Tlv0_1WireFormat::decodeInterest(Interest& interest, const uint8_t *input, size_t inputLength)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_ExcludeEntry excludeEntries[100];
  struct ndn_NameComponent keyNameComponents[100];
  struct ndn_Interest interestStruct;
  ndn_Interest_initialize
    (&interestStruct, nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]), 
     excludeEntries, sizeof(excludeEntries) / sizeof(excludeEntries[0]), 
     keyNameComponents, sizeof(keyNameComponents) / sizeof(keyNameComponents[0]));
    
  TlvDecoder decoder(input, inputLength);  
  ndn_Error error;
  if ((error = ndn_decodeTlvInterest(&interestStruct, &decoder)))
    throw runtime_error(ndn_getErrorString(error));

  interest.set(interestStruct);
}

Blob 
Tlv0_1WireFormat::encodeData(const Data& data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset) 
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
     
  return Blob(encoder.getOutput(), false);
}

void 
Tlv0_1WireFormat::decodeData
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

Blob 
Tlv0_1WireFormat::encodeControlParameters
  (const ControlParameters& controlParameters)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_ControlParameters controlParametersStruct;
  ndn_ControlParameters_initialize
    (&controlParametersStruct, nameComponents, 
     sizeof(nameComponents) / sizeof(nameComponents[0]));
  controlParameters.get(controlParametersStruct);

  TlvEncoder encoder(256);
  ndn_Error error;
  if ((error = ndn_encodeTlvControlParameters
       (&controlParametersStruct, &encoder)))
    throw runtime_error(ndn_getErrorString(error));
     
  return Blob(encoder.getOutput(), false);
}

#if 0
void 
Tlv0_1WireFormat::decodeControlParameters
  (ControlParameters& controlParameters, const uint8_t *input, 
   size_t inputLength)
{
}
#endif

}
