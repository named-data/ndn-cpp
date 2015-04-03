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

#ifndef NDN_BINARYXMLDECODER_HPP
#define NDN_BINARYXMLDECODER_HPP

#include <stdexcept>
#include "../c/encoding/binary-xml-decoder.h"

using namespace std;

namespace ndn {

/**
 * A BinaryXmlDecoder extends a C ndn_BinaryXmlDecoder struct and wraps related functions.
 */
class BinaryXmlDecoder : public ndn_BinaryXmlDecoder {
public:
  /**
   * Initialize the base ndn_BinaryXmlDecoder struct with the input.
   */
  BinaryXmlDecoder(const uint8_t *input, size_t inputLength)
  {
    ndn_BinaryXmlDecoder_initialize(this, input, inputLength);
  }

  /**
   * Decode the header from the input starting at offset, and if it is a DTAG where the value is the expectedTag,
   * then return true, else false.  Do not update offset, including if throwing an exception.
   * @param expectedTag the expected value for DTAG
   * @return true if got the expected tag, else false
   */
  bool
  peekDTag(unsigned int expectedTag)
  {
    int gotExpectedTag;
    ndn_Error error;
    if ((error = ndn_BinaryXmlDecoder_peekDTag(this, expectedTag, &gotExpectedTag)))
      throw runtime_error(ndn_getErrorString(error));

    return gotExpectedTag != 0;
  }
};

}

#endif
