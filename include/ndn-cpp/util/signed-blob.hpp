/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
 * @author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
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

#ifndef NDN_SIGNED_BLOB_HPP
#define NDN_SIGNED_BLOB_HPP

#include "blob.hpp"

namespace ndn {

/**
 * A SignedBlob extends Blob to keep the offsets of a signed portion (e.g., the bytes of Data packet).
 */
class SignedBlob : public Blob {
public:
  /**
   * Create a new SignedBlob with a null pointer.
   */
  SignedBlob()
  : signedPortionBeginOffset_(0), signedPortionEndOffset_(0)
  {
  }

  /**
   * Create a new SignedBlob and take another pointer to the given blob's
   * buffer.
   * @param blob The Blob from which we take another pointer to the same buffer.
   * @param signedPortionBeginOffset The offset in the encoding of the beginning of the signed portion.
   * @param signedPortionEndOffset The offset in the encoding of the end of the signed portion.
   */
  SignedBlob
    (const Blob& blob, size_t signedPortionBeginOffset, size_t signedPortionEndOffset)
  : Blob(blob), signedPortionBeginOffset_(signedPortionBeginOffset),
    signedPortionEndOffset_(signedPortionEndOffset)
  {
  }

  /**
   * Create a new SignedBlob with an immutable copy of the given array.
   * @param value A pointer to the byte array which is copied.
   * @param valueLength The length of value.
   * @param signedPortionBeginOffset The offset in the encoding of the beginning of the signed portion.
   * @param signedPortionEndOffset The offset in the encoding of the end of the signed portion.
   */
  SignedBlob
    (const uint8_t* value, size_t valueLength, size_t signedPortionBeginOffset, size_t signedPortionEndOffset)
  : Blob(value, valueLength), signedPortionBeginOffset_(signedPortionBeginOffset), signedPortionEndOffset_(signedPortionEndOffset)
  {
  }

  /**
   * Create a new SignedBlob with an immutable copy of the array in the given vector.
   * If you want to transfer the array without copying, the the vector has to start as a
   * ptr_lib::shared_ptr<std::vector<uint8_t> > and you can use the SignedBlob constructor with this type.
   * @param value A reference to a vector which is copied.
   * @param signedPortionBeginOffset The offset in the encoding of the beginning of the signed portion.
   * @param signedPortionEndOffset The offset in the encoding of the end of the signed portion.
   */
  SignedBlob
    (const std::vector<uint8_t> &value, size_t signedPortionBeginOffset, size_t signedPortionEndOffset)
  : Blob(value), signedPortionBeginOffset_(signedPortionBeginOffset), signedPortionEndOffset_(signedPortionEndOffset)
  {
  }

  /**
   * Create a new SignedBlob to point to an existing byte array. IMPORTANT: If copy is
   * false, after calling this constructor, if you keep a pointer to the array
   * then you must treat the array as immutable and promise not to change it.
   * @param value A pointer to a vector with the byte array.
   * @param copy If true, copy the value into a new vector. Otherwise, take
   * another reference and do not copy the bytes.
   * @param signedPortionBeginOffset The offset in the array of the beginning of the signed portion.
   * @param signedPortionEndOffset The offset in the array of the end of the signed portion.
   */
  SignedBlob
    (const ptr_lib::shared_ptr<std::vector<uint8_t> > &value, bool copy,
     size_t signedPortionBeginOffset, size_t signedPortionEndOffset)
  : Blob(value, copy), signedPortionBeginOffset_(signedPortionBeginOffset),
    signedPortionEndOffset_(signedPortionEndOffset)
  {
  }
  SignedBlob
    (const ptr_lib::shared_ptr<const std::vector<uint8_t> > &value, bool copy,
     size_t signedPortionBeginOffset, size_t signedPortionEndOffset)
  : Blob(value, copy), signedPortionBeginOffset_(signedPortionBeginOffset),
    signedPortionEndOffset_(signedPortionEndOffset)
  {
  }

  /**
   * Create a new SignedBlob to point to an existing byte array.
   * @deprecated Use the constructor with an explicit copy parameter.
   */
  DEPRECATED_IN_NDN_CPP SignedBlob
    (const ptr_lib::shared_ptr<std::vector<uint8_t> > &value,
     size_t signedPortionBeginOffset, size_t signedPortionEndOffset)
  : Blob(value, false), signedPortionBeginOffset_(signedPortionBeginOffset), signedPortionEndOffset_(signedPortionEndOffset)
  {
  }
  DEPRECATED_IN_NDN_CPP SignedBlob
    (const ptr_lib::shared_ptr<const std::vector<uint8_t> > &value,
     size_t signedPortionBeginOffset, size_t signedPortionEndOffset)
  : Blob(value,false), signedPortionBeginOffset_(signedPortionBeginOffset), signedPortionEndOffset_(signedPortionEndOffset)
  {
  }

  /**
   * Return the length of the signed portion of the immutable byte array, or 0 of the pointer to the array is null.
   */
  size_t
  signedSize() const
  {
    if (*this)
      return signedPortionEndOffset_ - signedPortionBeginOffset_;
    else
      return 0;
  }

  /**
   * Return a const pointer to the first byte of the signed portion of the immutable byte array, or 0 if the
   * pointer to the array is null.
   */
  const uint8_t*
  signedBuf() const
  {
    if (*this)
      return &(*this)->front() + signedPortionBeginOffset_;
    else
      return 0;
  }

  /**
   * Return the offset in the array of the beginning of the signed portion.
   */
  size_t
  getSignedPortionBeginOffset() const { return signedPortionBeginOffset_; }

  /**
   * Return the offset in the array of the end of the signed portion.
   */
  size_t
  getSignedPortionEndOffset() const { return signedPortionEndOffset_; }

private:
  size_t signedPortionBeginOffset_;
  size_t signedPortionEndOffset_;
};

}

#endif
