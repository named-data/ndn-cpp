/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
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
#include <ndn-cpp/control-parameters.hpp>
#include <ndn-cpp/digest-sha256-signature.hpp>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include <ndn-cpp/sha256-with-ecdsa-signature.hpp>
#include "../c/encoding/tlv/tlv-name.h"
#include "../c/encoding/tlv/tlv-interest.h"
#include "../c/encoding/tlv/tlv-data.h"
#include "../c/encoding/tlv/tlv-control-parameters.h"
#include "../c/encoding/tlv/tlv-signature-info.h"
#include "tlv-encoder.hpp"
#include "tlv-decoder.hpp"
#include <ndn-cpp/encoding/tlv-0_1_1-wire-format.hpp>

using namespace std;

namespace ndn {

Blob
Tlv0_1_1WireFormat::encodeName(const Name& name)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_Name nameStruct;
  ndn_Name_initialize
    (&nameStruct, nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]));
  name.get(nameStruct);

  TlvEncoder encoder(256);
  ndn_Error error;
  size_t dummyBeginOffset, dummyEndOffset;
  if ((error = ndn_encodeTlvName
       (&nameStruct, &dummyBeginOffset, &dummyEndOffset, &encoder)))
    throw runtime_error(ndn_getErrorString(error));

  return Blob(encoder.getOutput(), false);
}

void
Tlv0_1_1WireFormat::decodeName
  (Name& name, const uint8_t *input, size_t inputLength)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_Name nameStruct;
  ndn_Name_initialize
    (&nameStruct, nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]));

  TlvDecoder decoder(input, inputLength);
  ndn_Error error;
  size_t dummyBeginOffset, dummyEndOffset;
  if ((error = ndn_decodeTlvName
       (&nameStruct, &dummyBeginOffset, &dummyEndOffset, &decoder)))
    throw runtime_error(ndn_getErrorString(error));

  name.set(nameStruct);
}

Blob
Tlv0_1_1WireFormat::encodeInterest
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
Tlv0_1_1WireFormat::decodeInterest
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

  TlvDecoder decoder(input, inputLength);
  ndn_Error error;
  if ((error = ndn_decodeTlvInterest
       (&interestStruct, signedPortionBeginOffset, signedPortionEndOffset,
        &decoder)))
    throw runtime_error(ndn_getErrorString(error));

  interest.set(interestStruct);
}

Blob
Tlv0_1_1WireFormat::encodeData(const Data& data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset)
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
Tlv0_1_1WireFormat::decodeData
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
Tlv0_1_1WireFormat::encodeControlParameters
  (const ControlParameters& controlParameters)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_NameComponent strategyNameComponents[100];
  struct ndn_ControlParameters controlParametersStruct;
  ndn_ControlParameters_initialize
    (&controlParametersStruct, nameComponents,
     sizeof(nameComponents) / sizeof(nameComponents[0]), strategyNameComponents,
     sizeof(strategyNameComponents) / sizeof(strategyNameComponents[0]));
  controlParameters.get(controlParametersStruct);

  TlvEncoder encoder(256);
  ndn_Error error;
  if ((error = ndn_encodeTlvControlParameters
       (&controlParametersStruct, &encoder)))
    throw runtime_error(ndn_getErrorString(error));

  return Blob(encoder.getOutput(), false);
}

void
Tlv0_1_1WireFormat::decodeControlParameters
  (ControlParameters& controlParameters, const uint8_t *input,
   size_t inputLength)
{
  struct ndn_NameComponent nameComponents[100];
  struct ndn_NameComponent strategyNameComponents[100];
  struct ndn_ControlParameters controlParametersStruct;
  ndn_ControlParameters_initialize
    (&controlParametersStruct, nameComponents,
     sizeof(nameComponents) / sizeof(nameComponents[0]), strategyNameComponents,
     sizeof(strategyNameComponents) / sizeof(strategyNameComponents[0]));

  TlvDecoder decoder(input, inputLength);
  ndn_Error error;
  if ((error = ndn_decodeTlvControlParameters(&controlParametersStruct, &decoder)))
    throw runtime_error(ndn_getErrorString(error));

  controlParameters.set(controlParametersStruct);
}

Blob
Tlv0_1_1WireFormat::encodeSignatureInfo(const Signature& signature)
{
  struct ndn_Signature signatureStruct;
  struct ndn_NameComponent nameComponents[100];
  ndn_Signature_initialize
    (&signatureStruct, nameComponents,
     sizeof(nameComponents) / sizeof(nameComponents[0]));
  signature.get(signatureStruct);

  TlvEncoder encoder(256);
  ndn_Error error;
  if ((error = ndn_encodeTlvSignatureInfo(&signatureStruct, &encoder)))
    throw runtime_error(ndn_getErrorString(error));

  return Blob(encoder.getOutput(), false);
}

ptr_lib::shared_ptr<Signature>
Tlv0_1_1WireFormat::decodeSignatureInfoAndValue
  (const uint8_t *signatureInfo, size_t signatureInfoLength,
   const uint8_t *signatureValue, size_t signatureValueLength)
{
  struct ndn_NameComponent keyNameComponents[100];
  struct ndn_Signature signatureStruct;
  ndn_Error error;
  ndn_Signature_initialize
    (&signatureStruct,
     keyNameComponents, sizeof(keyNameComponents) / sizeof(keyNameComponents[0]));

  {
    TlvDecoder decoder(signatureInfo, signatureInfoLength);
    if ((error = ndn_decodeTlvSignatureInfo(&signatureStruct, &decoder)))
      throw runtime_error(ndn_getErrorString(error));
  }
  {
    TlvDecoder decoder(signatureValue, signatureValueLength);
    if ((error = ndn_TlvDecoder_readBlobTlv
         (&decoder, ndn_Tlv_SignatureValue, &signatureStruct.signature)))
      throw runtime_error(ndn_getErrorString(error));
  }

  ptr_lib::shared_ptr<Signature> result;
  if (signatureStruct.type == ndn_SignatureType_Sha256WithRsaSignature)
    result.reset(new Sha256WithRsaSignature());
  else if (signatureStruct.type == ndn_SignatureType_Sha256WithEcdsaSignature)
    result.reset(new Sha256WithEcdsaSignature());
  else if (signatureStruct.type == ndn_SignatureType_DigestSha256Signature)
    result.reset(new DigestSha256Signature());
  else
    // We don't expect this to happen.
    throw runtime_error("signatureStruct.type has an unrecognized value");

  result->set(signatureStruct);
  return result;
}

Blob
Tlv0_1_1WireFormat::encodeSignatureValue(const Signature& signature)
{
  struct ndn_Blob signatureStruct;
  signature.getSignature().get(signatureStruct);

  TlvEncoder encoder(256);
  ndn_Error error;
  if ((error = ndn_TlvEncoder_writeBlobTlv
       (&encoder, ndn_Tlv_SignatureValue, &signatureStruct)))
    throw runtime_error(ndn_getErrorString(error));

  return Blob(encoder.getOutput(), false);
}

Tlv0_1_1WireFormat* Tlv0_1_1WireFormat::instance_ = 0;

}
