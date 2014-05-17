/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_PROTOBUF = 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_PROTOBUF

#include <google/protobuf/message.h>
#include "tlv-encoder.hpp"
#include "tlv-decoder.hpp"
#include <ndn-cpp/encoding/protobuf-tlv.hpp>

using namespace std;
using namespace google::protobuf;

namespace ndn {

static void
encodeMessageValue(void *context, TlvEncoder &encoder)
{
  const Message& message = *(const Message *)context;
  const Reflection& reflection = *message.GetReflection();
  const Descriptor& descriptor = *message.GetDescriptor();

  for (int iField = 0; iField < descriptor.field_count(); ++iField) {  
    const FieldDescriptor* field = descriptor.field(iField);
    int tlvType = field->number();

    if (field->is_repeated()) {
      for (int i = 0; i < reflection.FieldSize(message, field); ++i) {
        if (field->type() == FieldDescriptor::TYPE_MESSAGE)
          encoder.writeNestedTlv
            (tlvType, encodeMessageValue, 
             (void*)&reflection.GetRepeatedMessage(message, field, i));
        else if (field->type() == FieldDescriptor::TYPE_UINT32)
          encoder.writeNonNegativeIntegerTlv
            (tlvType, (uint64_t)reflection.GetRepeatedUInt32(message, field, i));
        else if (field->type() == FieldDescriptor::TYPE_UINT64)
          encoder.writeNonNegativeIntegerTlv
            (tlvType, (uint64_t)reflection.GetRepeatedUInt64(message, field, i));
        else if (field->type() == FieldDescriptor::TYPE_BYTES ||
                 field->type() == FieldDescriptor::TYPE_STRING)
          encoder.writeRawStringTlv
            (tlvType, reflection.GetRepeatedString(message, field, i));
        else if (field->type() == FieldDescriptor::TYPE_BOOL) {
          if (reflection.GetRepeatedBool(message, field, i))
            encoder.writeTypeAndLength(tlvType, 0);
        }
        else
          throw runtime_error("ProtobufTlv::encode: Unknown repeated field type");
      }
    }
    else {
      // Not repeated.
      if (reflection.HasField(message, field)) {
        if (field->type() == FieldDescriptor::TYPE_MESSAGE)
          encoder.writeNestedTlv
            (tlvType, encodeMessageValue, 
             (void*)&reflection.GetMessage(message, field));
        else if (field->type() == FieldDescriptor::TYPE_UINT32)
          encoder.writeNonNegativeIntegerTlv
            (tlvType, (uint64_t)reflection.GetUInt32(message, field));
        else if (field->type() == FieldDescriptor::TYPE_UINT64)
          encoder.writeNonNegativeIntegerTlv
            (tlvType, (uint64_t)reflection.GetUInt64(message, field));
        else if (field->type() == FieldDescriptor::TYPE_BYTES ||
                 field->type() == FieldDescriptor::TYPE_STRING)
          encoder.writeRawStringTlv
            (tlvType, reflection.GetString(message, field));
        else if (field->type() == FieldDescriptor::TYPE_BOOL) {
          if (reflection.GetBool(message, field))
            encoder.writeTypeAndLength(tlvType, 0);
        }
        else
          throw runtime_error("ProtobufTlv::encode: Unknown field type");
      }
    }
  }
}

Blob 
ProtobufTlv::encode(const Message& message)
{
  message.CheckInitialized();
  TlvEncoder encoder(256);
  
  encodeMessageValue((void*)&message, encoder);
  return Blob(encoder.getOutput(), false);
}

static void 
decodeMessageValue
  (Message& message, TlvDecoder &decoder, size_t endOffset)
{
  const Reflection& reflection = *message.GetReflection();
  const Descriptor& descriptor = *message.GetDescriptor();

  for (int iField = 0; iField < descriptor.field_count(); ++iField) {  
    const FieldDescriptor* field = descriptor.field(iField);
    int tlvType = field->number();
    
    if (field->is_optional() && !decoder.peekType(tlvType, endOffset))
      continue;
    
    if (field->is_repeated()) {
      while (decoder.peekType(tlvType, endOffset)) {
        if (field->type() == FieldDescriptor::TYPE_MESSAGE) {
          size_t innerEndOffset = decoder.readNestedTlvsStart(tlvType);
          decodeMessageValue
            (*reflection.AddMessage(&message, field), decoder, innerEndOffset);
          decoder.finishNestedTlvs(innerEndOffset);
        }
        else if (field->type() == FieldDescriptor::TYPE_UINT32)
          reflection.AddUInt32
            (&message, field, decoder.readNonNegativeIntegerTlv(tlvType));
        else if (field->type() == FieldDescriptor::TYPE_UINT64)
          reflection.AddUInt64
            (&message, field, decoder.readNonNegativeIntegerTlv(tlvType));
        else if (field->type() == FieldDescriptor::TYPE_BYTES ||
                 field->type() == FieldDescriptor::TYPE_STRING) {
          struct ndn_Blob value = decoder.readBlobTlv(tlvType);
          reflection.AddString
            (&message, field, string((const char*)value.value, value.length));
        }
        else if (field->type() == FieldDescriptor::TYPE_BOOL)
          reflection.AddBool
            (&message, field, decoder.readBooleanTlv(tlvType, endOffset));
        else
          throw runtime_error("ProtobufTlv::decode: Unknown repeated field type");
      }
    }
    else {
      // Not repeated.
      if (field->type() == FieldDescriptor::TYPE_MESSAGE) {
        size_t innerEndOffset = decoder.readNestedTlvsStart(tlvType);
        decodeMessageValue
          (*reflection.MutableMessage(&message, field), decoder, 
           innerEndOffset);
        decoder.finishNestedTlvs(innerEndOffset);
      }
      else if (field->type() == FieldDescriptor::TYPE_UINT32)
        reflection.SetUInt32
          (&message, field, decoder.readNonNegativeIntegerTlv(tlvType));
      else if (field->type() == FieldDescriptor::TYPE_UINT64)
        reflection.SetUInt64
          (&message, field, decoder.readNonNegativeIntegerTlv(tlvType));
      else if (field->type() == FieldDescriptor::TYPE_BYTES ||
               field->type() == FieldDescriptor::TYPE_STRING) {
        struct ndn_Blob value = decoder.readBlobTlv(tlvType);
        reflection.SetString
          (&message, field, string((const char*)value.value, value.length));
      }
      else if (field->type() == FieldDescriptor::TYPE_BOOL)
        reflection.SetBool
          (&message, field, decoder.readBooleanTlv(tlvType, endOffset));
      else
        throw runtime_error("ProtobufTlv::decode: Unknown field type");
    }
  }
}

void 
ProtobufTlv::decode(Message& message, const uint8_t *input, size_t inputLength)
{
  TlvDecoder decoder(input, inputLength);  
  decodeMessageValue(message, decoder, inputLength);
}

}

#endif // NDN_CPP_HAVE_PROTOBUF
