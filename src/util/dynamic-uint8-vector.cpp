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

#include "dynamic-uint8-vector.hpp"

using namespace std;

namespace ndn {

DynamicUInt8Vector::DynamicUInt8Vector(size_t initialLength)
: vector_(new vector<uint8_t>(initialLength))
{
  ndn_DynamicUInt8Array_initialize(this, &vector_->front(), initialLength, DynamicUInt8Vector::realloc);
}

uint8_t*
DynamicUInt8Vector::realloc(struct ndn_DynamicUInt8Array *self, uint8_t *array, size_t length)
{
  // Because this method is private, assume there is not a problem with upcasting.
  DynamicUInt8Vector *thisObject = (DynamicUInt8Vector *)self;

  if (array != &thisObject->vector_->front())
    // We don't expect this to ever happen. The caller didn't pass the array from this object.
    return 0;

  thisObject->vector_->resize(length);
  return &thisObject->vector_->front();
}

}
