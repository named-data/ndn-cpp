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

#ifndef NDN_BLOB_HPP
#define NDN_BLOB_HPP

#include <iostream>
#include "../common.hpp"

struct ndn_Blob;

namespace ndn {

/**
 * A Blob holds a pointer to an immutable byte array implemented as const std::vector<uint8_t>.
 * This is like a JavaScript string which is a pointer to an immutable string.
 * It is OK to pass a pointer to the string because the new owner can't change the bytes
 * of the string.  However, like a JavaScript string, it is possible to change the pointer, and so this does allow
 * the copy constructor and assignment to change the pointer.  Also remember that the pointer can be null.
 * (Note that we could have made Blob derive directly from vector<uint8_t> and then explicitly use
 * a pointer to it like Blob, but this does not enforce immutability because we can't declare
 * Blob as derived from const vector<uint8_t>.)
 */
class Blob : public ptr_lib::shared_ptr<const std::vector<uint8_t> > {
public:
  /**
   * Create a new Blob with a null pointer.
   */
  Blob()
  {
  }

  /**
   * Create a new Blob with an immutable copy of the given array.
   * @param value A pointer to the byte array which is copied.
   * @param valueLength The length of value.
   */
  Blob(const uint8_t* value, size_t valueLength)
  : ptr_lib::shared_ptr<const std::vector<uint8_t> >(new std::vector<uint8_t>(value, value + valueLength))
  {
  }

  /**
   * Create a new Blob with an immutable copy of the array in the given vector.
   * If you want to transfer the array without copying, the the vector has to start as a
   * ptr_lib::shared_ptr<std::vector<uint8_t> > and you can use the Blob constructor with this type.
   * @param value A reference to a vector which is copied.
   */
  Blob(const std::vector<uint8_t> &value)
  : ptr_lib::shared_ptr<const std::vector<uint8_t> >(new std::vector<uint8_t>(value))
  {
  }

  /**
   * Create a new Blob with an immutable copy of the array in the given Blob struct.
   * @param blobStruct The C ndn_Blob struct to receive the pointer.
   */
  Blob(const struct ndn_Blob& blobStruct);

  /**
   * Create a new Blob and take another pointer to the given blob's buffer.
   * @param blob The Blob from which we take another pointer to the same buffer.
   */
  Blob(const Blob& blob)
  : ptr_lib::shared_ptr<const std::vector<uint8_t> >(blob)
  {
  }

  /**
   * Create a new Blob to point to an existing byte array. IMPORTANT: If copy is
   * false, after calling this constructor, if you keep a pointer to the array
   * then you must treat the array as immutable and promise not to change it.
   * @param value A pointer to a vector with the byte array.
   * @param copy If true, copy the value into a new vector. Otherwise, take
   * another reference and do not copy the bytes.
   */
  Blob(const ptr_lib::shared_ptr<std::vector<uint8_t> > &value, bool copy)
  : ptr_lib::shared_ptr<const std::vector<uint8_t> >
     ((const ptr_lib::shared_ptr<const std::vector<uint8_t> > &)value)
  {
    if (copy)
      this->reset(new std::vector<uint8_t>(*value));
  }
  Blob(const ptr_lib::shared_ptr<const std::vector<uint8_t> > &value, bool copy)
  : ptr_lib::shared_ptr<const std::vector<uint8_t> >(value)
  {
    if (copy)
      this->reset(new std::vector<uint8_t>(*value));
  }

  /**
   * Create a new Blob to point to an existing byte array.
   * @deprecated Use the constructor with an explicit copy parameter.
   */
  DEPRECATED_IN_NDN_CPP Blob(const ptr_lib::shared_ptr<std::vector<uint8_t> > &value)
  : ptr_lib::shared_ptr<const std::vector<uint8_t> >((const ptr_lib::shared_ptr<const std::vector<uint8_t> > &)value)
  {
  }
  DEPRECATED_IN_NDN_CPP Blob(const ptr_lib::shared_ptr<const std::vector<uint8_t> > &value)
  : ptr_lib::shared_ptr<const std::vector<uint8_t> >(value)
  {
  }

  /**
   * Return the length of the immutable byte array.
   */
  size_t
  size() const
  {
    if (*this)
      return (*this)->size();
    else
      return 0;
  }

  /**
   * Return a const pointer to the first byte of the immutable byte array, or 0 if the pointer is null.
   */
  const uint8_t*
  buf() const
  {
    if (*this)
      return &(*this)->front();
    else
      return 0;
  }

  /**
   * Check if the array pointer is null.
   * @return true if the buffer pointer is null, otherwise false.
   */
  bool
  isNull() const { return !(*this); }

  /**
   * Return the hex representation of the bytes in array.
   * @return The hex bytes as a string, or an empty string if the pointer is
   * null.
   */
  std::string
  toHex() const { return (*this) ? ndn::toHex(**this) : std::string(); }

  /**
   * Return the bytes of the byte array as a raw str of the same length. This
   * does not do any character encoding such as UTF-8.
   * @return The buffer as a string, or "" if isNull().
   */
  std::string
  toRawStr() const
  {
    return (*this) ? std::string((const char*)(&(*this)->front()), (*this)->size())
                   : std::string();
  }

  /**
   * Check if the value of this Blob equals the other blob.
   * @param other The other Blob to check.
   * @return True if this isNull and other isNull or if the bytes of this
   * blob equal the bytes of the other.
   */
  bool
  equals(const Blob& other) const
  {
    if (isNull())
      return other.isNull();
    else if (other.isNull())
      return false;
    else
      // Use the vector equals operator.
      return (**this) == (*other);
  }

  /**
   * Set the blobStruct to point to this Blob's byte array, without copying any memory.
   * WARNING: The resulting pointer in blobStruct is invalid after a further use of this object which could reallocate memory.
   * @param blobStruct The C ndn_Blob struct to receive the pointer.
   */
  void
  get(struct ndn_Blob& blobStruct) const;
};

inline std::ostream&
operator << (std::ostream& os, const Blob& blob)
{
  for (size_t i = 0; i < blob.size(); ++i)
    os << blob.buf()[i];
  return os;
}

}

#endif
