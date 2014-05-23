/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2014 Regents of the University of California.
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#ifndef NDN_DYNAMIC_UCHAR_VECTOR_HPP
#define NDN_DYNAMIC_UCHAR_VECTOR_HPP

#include <vector>
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
   * Get the shared_ptr to the allocated vector.
   * @return The shared_ptr to the allocated vector. 
   */
  ptr_lib::shared_ptr<std::vector<uint8_t> >& 
  get() { return vector_; }
  
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
