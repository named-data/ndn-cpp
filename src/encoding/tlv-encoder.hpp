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

#ifndef NDN_TLV_ENCODER_HPP
#define NDN_TLV_ENCODER_HPP

#include <vector>
#include <string>
#include <stdexcept>
#include <ndn-cpp/common.hpp>
#include "../util/dynamic-uint8-vector.hpp"
#include "../c/encoding/tlv/tlv-encoder.h"

namespace ndn {

/**
 * A TlvEncoder extends a C ndn_TlvEncoder struct and wraps related functions.
 */
class TlvEncoder : public ndn_TlvEncoder {
public:
  /**
   * Initialize the base ndn_TlvEncoder struct with the initialLength.  Use simpleRealloc.
   * @param initialLength The initial size of the output.  If omitted, use 16.
   */
  TlvEncoder(size_t initialLength = 16)
  : output_(16)
  {
    ndn_TlvEncoder_initialize(this, &output_);
  }

  /**
   * Resize the output vector to the correct encoding length and return.
   * @return The encoding as a shared_ptr.  Assume that the caller now owns the vector.
   */
  const ptr_lib::shared_ptr<std::vector<uint8_t> >&
  getOutput()
  {
    output_.get()->resize(offset);
    return output_.get();
  }

  void
  writeTypeAndLength(unsigned int type, size_t length)
  {
    ndn_Error error;
    if ((error = ndn_TlvEncoder_writeTypeAndLength(this, type, length)))
      throw std::runtime_error(ndn_getErrorString(error));
  }

  void
  writeNonNegativeInteger(uint64_t value)
  {
    ndn_Error error;
    if ((error = ndn_TlvEncoder_writeNonNegativeInteger(this, value)))
      throw std::runtime_error(ndn_getErrorString(error));
  }

  void
  writeBlobTlv(unsigned int type, struct ndn_Blob *value)
  {
    ndn_Error error;
    if ((error = ndn_TlvEncoder_writeBlobTlv(this, type, value)))
      throw std::runtime_error(ndn_getErrorString(error));
  }

  /**
   * Call writeBlobTlv using the raw string bytes.
   * @param type The type of the TLV.
   * @param value The raw string.
   */
  void
  writeRawStringTlv(unsigned int type, const std::string& value)
  {
    struct ndn_Blob valueBlob;
    ndn_Blob_initialize(&valueBlob, (const uint8_t*)&value[0], value.size());
    writeBlobTlv(type, &valueBlob);
  }

  void
  writeNonNegativeIntegerTlv(unsigned int type, uint64_t value)
  {
    ndn_Error error;
    if ((error = ndn_TlvEncoder_writeNonNegativeIntegerTlv(this, type, value)))
      throw std::runtime_error(ndn_getErrorString(error));
  }

  void
  writeNestedTlv
  (unsigned int type,
   ndn_Error (*writeValue)(const void *context, struct ndn_TlvEncoder* encoder),
   const void *context, bool omitZeroLength = false)
  {
    ndn_Error error;
    if ((error = ndn_TlvEncoder_writeNestedTlv
         (this, type, writeValue, context, omitZeroLength ? 1 : 0)))
      throw std::runtime_error(ndn_getErrorString(error));
  }

  /**
   * Call writeNestedTlv so that it calls writeValue(context, *this).
   * @param type The TLV type for writeNestedTlv.
   * @param writeValue The callback to write the TLV value. This calls
   * writeValue(context, *this).
   * @param context The context to pass to writeValue.
   * @param omitZeroLength The omitZeroLength flag for writeNestedTlv.
   */
  void
  writeNestedTlv
  (unsigned int type, void (*writeValue)(const void *context, TlvEncoder& encoder),
   const void *context, bool omitZeroLength = false)
  {
    WriteValueWrapper(writeValue, context).writeNestedTlv
      (*this, type, omitZeroLength);
  }

private:
  /* An WriteValueWrapper holds the caller context so that it can be
   * passed to the caller's writeValue. See writeNestedTlv.
   */
  class WriteValueWrapper {
  public:
    /**
     * Create a new WriteValueWrapper to hold the writeValue and context.
     * See writeNestedTlv.
     * @param writeValue The callback to write the TLV value. This calls
     * writeValue(context, encoder).
     * @param context The context to pass to writeValue.
     */
    WriteValueWrapper
      (void (*writeValue)(const void *context, TlvEncoder& encoder), const void *context)
      : writeValue_(writeValue), context_(context) {}

    /**
     * Call encoder.writeNestedTlv so that it calls writeValue(context, encoder)
     * with the writeValue and context given to the constructor.
     * @param encoder The TlvEncoder.
     * @param type The TLV type for encoder.writeNestedTlv.
     * @param omitZeroLength The omitZeroLength flag for encoder.writeNestedTlv.
     */
    void
    writeNestedTlv(TlvEncoder& encoder, unsigned int type, bool omitZeroLength)
    {
      encoder.writeNestedTlv(type, writeValueWrapper, this, omitZeroLength);
    }

  private:
    /**
     * This private function is called by ndn_TlvEncoder_writeTlv to write the
     * TLVs in the body of a next TLV.
     * @param context A pointer to this WriteValueWrapper which was passed to
     * writeTlv.
     * @param encoder the TlvEncoder object which is calling this.
     * @return 0 for success. The wrapped writeValue_ throws an exception if
     * needed.
     */
    static ndn_Error
    writeValueWrapper(const void *context, struct ndn_TlvEncoder *encoder)
    {
      const WriteValueWrapper& wrapper = *(const WriteValueWrapper*)context;
      wrapper.writeValue_(wrapper.context_, (TlvEncoder&)*encoder);

      // wrapper.writeValue_ has thrown an exception if needed.
      return NDN_ERROR_success;
    }

    void (*writeValue_)(const void *context, TlvEncoder& encoder);
    const void *context_;
  };

  DynamicUInt8Vector output_;
};

}

#endif
