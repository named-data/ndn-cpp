/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
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

#ifndef NDN_BLOB_STREAM_HPP
#define NDN_BLOB_STREAM_HPP

// We can use ndnboost::iostreams because this is internal and will not conflict with the application if it uses boost::iostreams.
#include <ndnboost/iostreams/detail/ios.hpp>
#include <ndnboost/iostreams/categories.hpp>
#include <ndnboost/iostreams/stream.hpp>
#include <ndn-cpp/common.hpp>

namespace ndn {

class blob_append_device {
public:
  typedef char  char_type;
  typedef ndnboost::iostreams::sink_tag category;

  blob_append_device(std::vector<uint8_t>& container)
  : container_(container)
  {
  }

  std::streamsize
  write(const char_type* s, std::streamsize n)
  {
    std::copy(s, s+n, std::back_inserter(container_));
    return n;
  }

protected:
  std::vector<uint8_t>& container_;
};

/**
 * This is called "blob_stream" but it doesn't use an ndn::Blob which is immutable.  It uses a pointer to a vector<uint8_t>.
 * This is inteded for internal library use, not exported in the API.
 */
struct blob_stream : public ndnboost::iostreams::stream<blob_append_device>
{
  blob_stream()
    : buffer_(new std::vector<uint8_t>())
    , device_(*buffer_)
  {
    open(device_);
  }

  ptr_lib::shared_ptr<std::vector<uint8_t> >
  buf()
  {
    flush();
    return buffer_;
  }

private:
  ptr_lib::shared_ptr<std::vector<uint8_t> > buffer_;
  blob_append_device device_;
};

}

#endif
