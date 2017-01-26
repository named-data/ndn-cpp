/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
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

#include <stdexcept>
#include <math.h>
#include <ndn-cpp/encoding/protobuf-tlv.hpp>
#include "content-meta-info.pb.h"
#include <ndn-cpp-tools/usersync/content-meta-info.hpp>

using namespace std;
using namespace ndn;

namespace ndntools {

void
ContentMetaInfo::clear()
{
  contentType_ = "";
  timestamp_ = -1;
  hasSegments_ = false;
  other_ = Blob();
}

Blob
ContentMetaInfo::wireEncode() const
{
  if (timestamp_ < 0)
    throw runtime_error("ContentMetaInfo.wireEncode: The timestamp is not specified");

  ndn_message::ContentMetaInfoMessage meta;
  meta.mutable_content_meta_info()->set_content_type(contentType_);
  meta.mutable_content_meta_info()->set_timestamp((uint64_t)::round(timestamp_));
  meta.mutable_content_meta_info()->set_has_segments(hasSegments_);
  if (!other_.isNull())
    meta.mutable_content_meta_info()->set_other(other_.buf(), other_.size());

  return ProtobufTlv::encode(meta);
}

void
ContentMetaInfo::wireDecode(const uint8_t *input, size_t inputLength)
{
  clear();

  ndn_message::ContentMetaInfoMessage decodedMeta;
  ProtobufTlv::decode(decodedMeta, input, inputLength);
  const ndn_message::ContentMetaInfoMessage::ContentMetaInfo&
    meta = decodedMeta.content_meta_info();

  contentType_ = meta.content_type();
  timestamp_ = meta.timestamp();
  hasSegments_ = meta.has_segments();
  if (meta.has_other())
    other_ = Blob((const uint8_t*)&meta.other()[0], meta.other().size());
}

}

#endif // NDN_CPP_HAVE_PROTOBUF
