/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2020 Regents of the University of California.
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

#include <stdexcept>
#include <algorithm>
#include <vector>
#include <sstream>
#include "../../contrib/apache/apr_base64.h"
#include "base64.hpp"

using namespace std;

namespace ndn {

string
toBase64(const uint8_t* array, size_t arrayLength, bool addNewlines)
{
  vector<char> output(::apr_base64_encode_len(arrayLength));
  // Don't include the null terminator in the length.
  size_t outputLength = (size_t)::apr_base64_encode_binary
    (&output[0], (const unsigned char *)array, arrayLength) - 1;
  if (addNewlines) {
    ostringstream withNewlines;
    for (size_t i = 0; i < outputLength; i += 64) {
      size_t lineLength = 64;
      if (i + lineLength > outputLength)
        lineLength = outputLength - i;

      withNewlines.write(&output[i], lineLength);
      withNewlines << '\n';
    }

    return withNewlines.str();
  }
  else
    return string(output.begin(), output.begin() + outputLength);
}

void
fromBase64(const string& input, vector<uint8_t>& output)
{
  // We are only concerned with whitespace characters which are all less than
  // the first base64 character '+'. If we find whitespace, then we'll copy
  // non-whitespace to noWhitespaceStream.
  ostringstream noWhitespaceStream;
  bool gotWhitespace = false;
  for (size_t i = 0; i < input.size(); ++i) {
    if (input[i] < '+') {
      if (!gotWhitespace) {
        // We need to use the noWitespaceStream. Initialize it.
        gotWhitespace = true;
        noWhitespaceStream.write(&input[0], i);
      }
    }
    else {
      if (gotWhitespace)
        noWhitespaceStream << input[i];
    }
  }

  string noWhitespace;
  const char* inputCString;
  if (gotWhitespace) {
    noWhitespace = noWhitespaceStream.str();
    inputCString = noWhitespace.c_str();
  }
  else
    // The input didn't have any whitespace, so use it as is.
    inputCString = input.c_str();

  output.resize(::apr_base64_decode_len(inputCString));
  size_t outputLength = (size_t)::apr_base64_decode_binary
    (&output[0], inputCString);
  output.resize(outputLength);
}

}
