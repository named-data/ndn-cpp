/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From PyNDN der.py by Adeola Bannis <thecodemaiden@gmail.com>.
 * @author: Originally from code in ndn-cxx by Yingdi Yu <yingdi@cs.ucla.edu>
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

#include <sstream>
#include "../../c/util/time.h"
#include "der-exception.hpp"
#include "der-node.hpp"

using namespace std;

namespace ndn {

size_t
DerNode::getSize()
{
  return header_.size() + payloadPosition_;
}

Blob
DerNode::encode()
{
  DynamicUInt8Vector buffer(getSize());

  size_t position = 0;
  position = buffer.copy(&header_[0], header_.size(), position);
  position = buffer.copy(&payload_[0], payloadPosition_, position);

  buffer.get()->resize(position);
  return Blob(buffer.get(), false);
}

ptr_lib::shared_ptr<DerNode>
DerNode::parse(const uint8_t* inputBuf, size_t startIdx)
{
  int nodeType = ((int)inputBuf[startIdx]) & 0xff;
  // Don't increment startIdx. We're just peeking.

  ptr_lib::shared_ptr<DerNode> newNode;
  if (nodeType == DerNodeType_Boolean)
    newNode.reset(new DerBoolean());
  else if (nodeType == DerNodeType_Integer)
    newNode.reset(new DerInteger());
  else if (nodeType == DerNodeType_BitString)
    newNode.reset(new DerBitString());
  else if (nodeType == DerNodeType_OctetString)
    newNode.reset(new DerOctetString());
  else if (nodeType == DerNodeType_Null)
    newNode.reset(new DerNull());
  else if (nodeType == DerNodeType_ObjectIdentifier)
    newNode.reset(new DerOid());
  else if (nodeType == DerNodeType_Sequence)
    newNode.reset(new DerSequence());
  else if (nodeType == DerNodeType_PrintableString)
    newNode.reset(new DerPrintableString());
  else if (nodeType == DerNodeType_GeneralizedTime)
    newNode.reset(new DerGeneralizedTime());
  else
    throw DerDecodingException("Unimplemented DER type");

  newNode->decode(inputBuf, startIdx);
  return newNode;
}

Blob
DerNode::toVal()
{
  return encode();
}

const std::vector<ptr_lib::shared_ptr<DerNode> >&
DerNode::getChildren()
{
  throw DerDecodingException("getChildren: This DerNode is not DerSequence");
}

DerNode::DerSequence&
DerNode::getSequence
  (const std::vector<ptr_lib::shared_ptr<DerNode> >&children, size_t index)
{
  if (index >= children.size())
    throw DerDecodingException("getSequence: Child index is out of bounds");

  try {
    return dynamic_cast<DerSequence&>(*children[index]);
  }
  catch (std::exception& e) {
    throw DerDecodingException("getSequence: Child DerNode is not a DerSequence");
  }
}

void
DerNode::encodeHeader(size_t size)
{
  DynamicUInt8Vector buffer(10);
  size_t bufferPosition = 0;
  buffer[bufferPosition++] = (uint8_t)nodeType_;
  if (size <= 127)
    buffer[bufferPosition++] = size & 0xff;
  else {
    DynamicUInt8Vector tempBuf(10);
    // We encode backwards from the back.

    int val = size;
    size_t n = 0;
    while (val != 0) {
      ++n;
      tempBuf.ensureLengthFromBack(n);
      tempBuf[tempBuf.get()->size() - n] = val & 0xff;
      val >>= 8;
    }
    size_t nTempBufBytes = n + 1;
    tempBuf.ensureLengthFromBack(nTempBufBytes);
    tempBuf[tempBuf.get()->size() - nTempBufBytes] = ((1<<7) | n) & 0xff;

    buffer.copy
      (&tempBuf[tempBuf.get()->size() - nTempBufBytes], nTempBufBytes,
       bufferPosition);
    bufferPosition += nTempBufBytes;
  }

  buffer.get()->resize(bufferPosition);
  header_.swap(*buffer.get());
}

size_t
DerNode::decodeHeader(const uint8_t* inputBuf, size_t startIdx)
{
  size_t idx = startIdx;

  int nodeType = inputBuf[idx] & 0xff;
  idx += 1;

  nodeType_ = (DerNodeType)nodeType;

  int sizeLen = inputBuf[idx] & 0xff;
  idx += 1;

  header_.clear();
  header_.push_back(nodeType);
  header_.push_back(sizeLen);

  size_t size = sizeLen;
  bool isLongFormat = (sizeLen & (1 << 7)) != 0;
  if (isLongFormat) {
    int lenCount = sizeLen & ((1<<7) - 1);
    size = 0;
    while (lenCount > 0) {
      uint8_t b = inputBuf[idx];
      idx += 1;
      header_.push_back(b);
      size = 256 * size + (b & 0xff);
      lenCount -= 1;
    }
  }

  return size;
}

void
DerNode::decode(const uint8_t* inputBuf, size_t startIdx)
{
  size_t idx = startIdx;
  size_t payloadSize = decodeHeader(inputBuf, idx);
  size_t skipBytes = header_.size();
  if (payloadSize > 0) {
    idx += skipBytes;
    payloadAppend(inputBuf + idx, payloadSize);
  }
}

size_t
DerNode::DerStructure::getSize()
{
  if (childChanged_) {
    updateSize();
    childChanged_ = false;
  }

  encodeHeader(size_);
  return size_ + header_.size();
}

const std::vector<ptr_lib::shared_ptr<DerNode> >&
DerNode::DerStructure::getChildren()
{
  return nodeList_;
}

Blob
DerNode::DerStructure::encode()
{
  DynamicUInt8Vector buffer(10);
  size_t bufferPosition = 0;
  updateSize();
  encodeHeader(size_);
  bufferPosition = buffer.copy(&header_[0], header_.size(), bufferPosition);

  for (size_t i = 0; i < nodeList_.size(); ++i) {
    DerNode& n = *nodeList_[i];
    Blob encodedChild = n.encode();
    bufferPosition = buffer.copy
      (encodedChild.buf(), encodedChild.size(), bufferPosition);
  }

  buffer.get()->resize(bufferPosition);
  return Blob(buffer.get(), false);
}

void
DerNode::DerStructure::decode(const uint8_t* inputBuf, size_t startIdx)
{
  size_t idx = startIdx;
  size_ = decodeHeader(inputBuf, idx);
  idx += header_.size();

  size_t accSize = 0;
  while (accSize < size_) {
    ptr_lib::shared_ptr<DerNode> node = DerNode::parse(inputBuf, idx);
    idx += node->getSize();
    accSize += node->getSize();
    addChild(node, false);
  }
}

void
DerNode::DerStructure::updateSize()
{
  size_t newSize = 0;

  for (size_t i = 0; i < nodeList_.size(); ++i) {
    DerNode& n = *nodeList_[i];
    newSize += n.getSize();
  }

  size_ = newSize;
  childChanged_ = false;
}

Blob
DerNode::DerByteString::toVal()
{
  // Make a copy since payload_ can change.
  return Blob(&payload_[0], payloadPosition_);
}

DerNode::DerInteger::DerInteger(int integer)
: DerNode(DerNodeType_Integer)
{
  // Convert the integer to bytes the easy/slow way.
  DynamicUInt8Vector temp(10);
  // We encode backwards from the back.
  size_t length = 0;
  while (true) {
    ++length;
    temp.ensureLengthFromBack(length);
    temp[temp.get()->size() - length] = integer & 0xff;
    integer >>= 8;

    if (integer <= 0)
      // We check for 0 at the end so we encode one byte if it is 0.
      break;
  }

  payloadAppend(&temp[temp.get()->size() - length], length);
  encodeHeader(payloadPosition_);
}

DerNode::DerInteger::DerInteger()
: DerNode(DerNodeType_Integer)
{
}

int
DerNode::DerInteger::toIntegerVal() const
{
  int result = 0;
  for (int i = 0; i < payloadPosition_; ++i) {
    result <<= 8;
    result += payload_[i];
  }

  return result;
}

Blob
DerNode::DerOid::toVal()
{
  size_t offset = 0;
  vector<int> components;

  while (offset < payloadPosition_) {
    size_t skip;
    int nextVal = decode128(offset, skip);
    offset += skip;
    components.push_back(nextVal);
  }

  // For some odd reason, the first digits are represented in one byte.
  int firstByte = components[0];
  int firstDigit = firstByte / 40;
  int secondDigit = firstByte % 40;

  ostringstream result;
  result << firstDigit << "." << secondDigit;
  for (int i = 1; i < components.size(); ++i)
    result << "." << components[i];

  string resultString = result.str();
  return Blob((const uint8_t*)&resultString[0], resultString.size());
}

void
DerNode::DerOid::prepareEncoding(const vector<int>& value)
{
  int firstNumber = 0;
  if (value.size() == 0)
    throw DerEncodingException("No integer in OID");
  else {
    if (value[0] >= 0 && value[0] <= 2)
      firstNumber = value[0] * 40;
    else
      throw DerEncodingException("First integer in OID is out of range");
  }

  if (value.size() >= 2) {
    if (value[1] >= 0 && value[1] <= 39)
      firstNumber += value[1];
    else
      throw DerEncodingException("Second integer in OID is out of range");
  }

  DynamicUInt8Vector encodedBuffer(10);
  size_t position = 0;
  position = encodedBuffer.copy(encode128(firstNumber), position);

  if (value.size() > 2) {
    for (int i = 2; i < value.size(); ++i)
      position = encodedBuffer.copy(encode128(value[i]), position);
  }

  encodeHeader(position);
  payloadAppend(&encodedBuffer[0], position);
}

vector<uint8_t>
DerNode::DerOid::encode128(int value)
{
  int mask = (1 << 7) - 1;
  // We encode backwards by inserting in the front of the vector. We don't
  //   expect the result to be very large, so it is OK to keep shifting bytes.
  vector<uint8_t> outBytes;

  if (value < 128)
    outBytes.insert(outBytes.begin(), value & mask);
  else {
    outBytes.insert(outBytes.begin(), value & mask);
    value >>= 7;
    while (value != 0) {
      outBytes.insert(outBytes.begin(), (value & mask) | (1 << 7));
      value >>= 7;
    }
  }

  return outBytes;
}

int
DerNode::DerOid::decode128(size_t offset, size_t& skip)
{
  int flagMask = 0x80;
  int result = 0;
  size_t oldOffset = offset;

  while ((payload_[offset] & flagMask) != 0) {
    result = 128 * result + ((int)payload_[offset] & 0xff) - 128;
    offset += 1;
  }

  result = result * 128 + ((int)payload_[offset] & 0xff);

  skip = offset - oldOffset + 1;
  return result;
}

MillisecondsSince1970
DerNode::DerGeneralizedTime::toMillisecondsSince1970()
{
  string payloadString((const char*)&payload_[0], payloadPosition_);
  // The payload time has Z at the end. Convert to ISO time with 'T' in the middle.
  return fromIsoString
    (payloadString.substr(0, 8) + 'T' + payloadString.substr(8, 6));
}

string
DerNode::DerGeneralizedTime::toDerTimeString(MillisecondsSince1970 msSince1970)
{
  string pTimeStr = toIsoString(msSince1970);
  // The ISO string has the 'T' in the middle. Convert to UTC with 'Z' at the end.
  size_t index = pTimeStr.find_first_of('T');
  return pTimeStr.substr(0, index) +
    pTimeStr.substr(index + 1, pTimeStr.size() - (index + 1)) + 'Z';
}

string
DerNode::DerGeneralizedTime::toIsoString(const MillisecondsSince1970& time)
{
  char isoString[25];
  ndn_Error error;
  if ((error = ndn_toIsoString(time, isoString)))
    throw runtime_error(ndn_getErrorString(error));

  return isoString;
}

MillisecondsSince1970
DerNode::DerGeneralizedTime::fromIsoString(const string& isoString)
{
  MillisecondsSince1970 milliseconds;
  ndn_Error error;
  if ((error = ndn_fromIsoString(isoString.c_str(), &milliseconds)))
    throw runtime_error(ndn_getErrorString(error));

  return milliseconds;
}

}
