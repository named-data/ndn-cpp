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

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_PROTOBUF = 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_PROTOBUF

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <stdexcept>
#include "tlv-encoder.hpp"
#include "tlv-decoder.hpp"
#include <ndn-cpp/encoding/protobuf-tlv.hpp>

using namespace std;
using namespace google::protobuf;

namespace ndn {

static void
encodeMessageValue(const void *context, TlvEncoder &encoder)
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
             (const void*)&reflection.GetRepeatedMessage(message, field, i));
        else if (field->type() == FieldDescriptor::TYPE_UINT32)
          encoder.writeNonNegativeIntegerTlv
            (tlvType, (uint64_t)reflection.GetRepeatedUInt32(message, field, i));
        else if (field->type() == FieldDescriptor::TYPE_UINT64)
          encoder.writeNonNegativeIntegerTlv
            (tlvType, (uint64_t)reflection.GetRepeatedUInt64(message, field, i));
        else if (field->type() == FieldDescriptor::TYPE_ENUM)
        {
          int number = reflection.GetRepeatedEnum(message, field, i)->number();
          if (number < 0)
            throw runtime_error("ProtobufTlv::encode: ENUM value may not be negative");
          encoder.writeNonNegativeIntegerTlv(tlvType, (uint64_t)number);
        }
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
             (const void*)&reflection.GetMessage(message, field));
        else if (field->type() == FieldDescriptor::TYPE_UINT32)
          encoder.writeNonNegativeIntegerTlv
            (tlvType, (uint64_t)reflection.GetUInt32(message, field));
        else if (field->type() == FieldDescriptor::TYPE_UINT64)
          encoder.writeNonNegativeIntegerTlv
            (tlvType, (uint64_t)reflection.GetUInt64(message, field));
        else if (field->type() == FieldDescriptor::TYPE_ENUM)
        {
          int number = reflection.GetEnum(message, field)->number();
          if (number < 0)
            throw runtime_error("ProtobufTlv::encode: ENUM value may not be negative");
          encoder.writeNonNegativeIntegerTlv(tlvType, (uint64_t)number);
        }
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
        else if (field->type() == FieldDescriptor::TYPE_ENUM)
          reflection.AddEnum
            (&message, field,
             field->enum_type()->FindValueByNumber
               (decoder.readNonNegativeIntegerTlv(tlvType)));
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
      else if (field->type() == FieldDescriptor::TYPE_ENUM)
        reflection.SetEnum
          (&message, field,
           field->enum_type()->FindValueByNumber
             (decoder.readNonNegativeIntegerTlv(tlvType)));
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
