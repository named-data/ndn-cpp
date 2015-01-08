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

#include "../c/util/blob.h"
#include <ndn-cpp/util/blob.hpp>

using namespace std;

namespace ndn {

Blob::Blob(const struct ndn_Blob& blobStruct)
  : ptr_lib::shared_ptr<const vector<uint8_t> >(new vector<uint8_t>(blobStruct.value, blobStruct.value + blobStruct.length))
{
}

void
Blob::get(struct ndn_Blob& blobStruct) const
{
  blobStruct.length = size();
  if (size() > 0)
    blobStruct.value = buf();
  else
    blobStruct.value = 0;
}

}
