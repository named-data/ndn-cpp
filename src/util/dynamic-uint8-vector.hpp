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

#ifndef NDN_DYNAMIC_UCHAR_VECTOR_HPP
#define NDN_DYNAMIC_UCHAR_VECTOR_HPP

#include <vector>
#include <stdexcept>
#include <ndn-cpp/common.hpp>
#include "../c/util/dynamic-uint8-array.h"

namespace ndn {

/**
 * A DynamicUInt8Vector extends ndn_DynamicUInt8Array to hold a shared_ptr<vector<uint8_t> > for use with
 * C functions which need an ndn_DynamicUInt8Array.
 */
class DynamicUInt8Vector : public ndn_DynamicUInt8Array {
public:
  /**
   * Create a new DynamicUInt8Vector with an initial length.
   * @param initialLength The initial size of the allocated vector.
   */
  DynamicUInt8Vector(size_t initialLength);

  /**
   * Ensure that the vector size is greater than or equal to length.  If it is,
   * just return. Otherwise, resize the vector (copying as needed).
   * (The new vector size may be greater than length.)
   * @param length The minimum size of the vector.
   */
  void
  ensureLength(size_t length)
  {
    ndn_Error error;
    if ((error = ndn_DynamicUInt8Array_ensureLength(this, length)))
      throw std::runtime_error(ndn_getErrorString(error));
  }

  /**
   * Copy value into the vector at offset, using ensureLength to make sure
   * the vector has enough size.
   * @param value The buffer to copy from.
   * @param valueLength The length of the value buffer.
   * @param offset The offset in the vector to copy to.
   * @return The new offset which is offset + valueLength.
   */
  size_t
  copy(const uint8_t *value, size_t valueLength, size_t offset)
  {
    ndn_Error error;
    if ((error = ndn_DynamicUInt8Array_copy(this, value, valueLength, offset)))
      throw std::runtime_error(ndn_getErrorString(error));

    return offset + valueLength;
  }

  /**
   * Copy value into the vector at offset, using ensureLength to make sure
   * the vector has enough size.
   * @param value The buffer to copy from.
   * @param offset The offset in the vector to copy to.
   * @return The new offset which is offset + value.size().
   */
  size_t
  copy(const std::vector<uint8_t>& value, size_t offset)
  {
    return copy(&value[0], value.size(), offset);
  }

  /**
   * Ensure that the vector size is greater than or equal to length.  If it is,
   * just return. Otherwise, resize the vector and shift existing data to the
   * back. (The new vector size may be greater than length.)
   * @param length The minimum size of the vector.
   */
  void
  ensureLengthFromBack(size_t length)
  {
    ndn_Error error;
    if ((error = ndn_DynamicUInt8Array_ensureLengthFromBack(this, length)))
      throw std::runtime_error(ndn_getErrorString(error));
  }

  /**
   * First call ensureLengthFromBack to make sure the vector has offsetFromBack
   * bytes. Then copy value into the vector starting offsetFromBack bytes from
   * the back of the vector.
   * @param value The buffer to copy from.
   * @param valueLength The length of the value buffer.
   * @param offsetFromBack The offset from the back of the array to start
   * copying.
   */
  void
  copyFromBack(const uint8_t *value, size_t valueLength, size_t offsetFromBack)
  {
    ndn_Error error;
    if ((error = ndn_DynamicUInt8Array_copyFromBack
         (this, value, valueLength, offsetFromBack)))
      throw std::runtime_error(ndn_getErrorString(error));
  }

  /**
   * Get the shared_ptr to the allocated vector.
   * @return The shared_ptr to the allocated vector.
   */
  ptr_lib::shared_ptr<std::vector<uint8_t> >&
  get() { return vector_; }

  uint8_t&
  operator [] (size_t i) { return (*vector_)[i]; }

  const uint8_t&
  operator [] (size_t i) const { return (*vector_)[i]; }

private:
  /**
   * Implement the static realloc function using vector resize.
   * @param self A pointer to this object.
   * @param array Should be the front of the vector.
   * @param length The new length for the vector.
   * @return The front of the allocated vector.
   */
  static uint8_t*
  realloc(struct ndn_DynamicUInt8Array *self, uint8_t *array, size_t length);

  ptr_lib::shared_ptr<std::vector<uint8_t> > vector_;
};

}

#endif
