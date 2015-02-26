/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015 Regents of the University of California.
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

#ifndef NDN_BLOB_LITE_HPP
#define NDN_BLOB_LITE_HPP

#include <ndn-cpp/c/blob-types.h>

namespace ndn {

class BlobLite : private ndn_Blob {
public:
  BlobLite();

  BlobLite(const uint8_t* value, size_t length);

  const uint8_t*
  buf() const { return value; }

  size_t
  size() const { return length; }

  /**
   * Upcast the reference to the ndn_Blob struct to a BlobLite.
   * @param blob A reference to the ndn_Blob struct.
   * @return The same reference as BlobLite.
   */
  static BlobLite&
  upCast(ndn_Blob& blob) { return *(BlobLite*)&blob; }

  static const BlobLite&
  upCast(const ndn_Blob& blob) { return *(BlobLite*)&blob; }
};

}

#endif
