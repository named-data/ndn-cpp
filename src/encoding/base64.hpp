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

#ifndef NDN_BASE64_HPP
#define NDN_BASE64_HPP

#include <string>
#include <vector>
#include <ndn-cpp/common.hpp>

namespace ndn {

/**
 * Return the base64 representation of the bytes in array.
 * @param array The array of bytes.
 * @param arrayLength The number of bytes in array.
 * @param addNewlines (optional) If true, add newlines to the output (good for
 * writing to a file).  If omitted, do not add newlines.
 * @return The base64 string.
 */
std::string
toBase64(const uint8_t* array, size_t arrayLength, bool addNewlines = false);

/**
 * Return the base64 representation of the bytes in array.
 * @param array The array of bytes.
 * @param addNewlines (optional) If true, add newlines to the output (good for
 * writing to a file).  If omitted, do not add newlines.
 * @return The base64 string.
 */
static std::string
toBase64(const std::vector<uint8_t>& array, bool addNewlines = false)
{
  return toBase64(&array[0], array.size(), addNewlines);
}

/**
 * Decode the base64 input string and write the binary result to output.
 * @param input The input base64 string.
 * @param output Write the result to output starting at index 0, calling
 * output.resize() as needed.
 */
void
fromBase64(const std::string& input, std::vector<uint8_t>& output);


}

#endif
