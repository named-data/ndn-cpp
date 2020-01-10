/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the PSync library https://github.com/named-data/PSync/blob/master/PSync/detail/iblt.cpp
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Gavin Andresen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_LIBZ 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_LIBZ

#include <stdexcept>
#include <zlib.h>
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include "invertible-bloom-lookup-table.hpp"

using namespace std;

namespace ndn {

InvertibleBloomLookupTable::InvertibleBloomLookupTable(size_t expectedNEntries)
{
  // 1.5 times the expected number of entries gives a very low probability of a
  // decoding failure.
  size_t nEntries = expectedNEntries + expectedNEntries / 2;
  // Make nEntries exactly divisible by N_HASH.
  size_t remainder = nEntries % N_HASH;
  if (remainder != 0)
    nEntries += (N_HASH - remainder);

  hashTable_.resize(nEntries);
}

void
InvertibleBloomLookupTable::initialize(const Blob& encoding)
{
  vector<uint32_t> values = decode(encoding);

  if (3 * hashTable_.size() != values.size())
    throw runtime_error("The received Invertible Bloom Filter cannot be decoded");

  for (size_t i = 0; i < hashTable_.size(); i++) {
    HashTableEntry& entry = hashTable_.at(i);
    if (values[i * 3] != 0) {
      entry.count_ = values[i * 3];
      entry.keySum_ = values[(i * 3) + 1];
      entry.keyCheck_ = values[(i * 3) + 2];
    }
  }
}

bool
InvertibleBloomLookupTable::listEntries
  (set<uint32_t>& positive, set<uint32_t>& negative) const
{
  positive.clear();
  negative.clear();

  // Make a deep copy.
  InvertibleBloomLookupTable peeled(*this);

  size_t nErased = 0;
  do {
    nErased = 0;
    for (vector<HashTableEntry>::const_iterator entry = peeled.hashTable_.begin();
         entry != peeled.hashTable_.end(); ++entry) {
      if (entry->isPure()) {
        if (entry->count_ == 1)
          positive.insert(entry->keySum_);
        else
          negative.insert(entry->keySum_);

        peeled.update(-entry->count_, entry->keySum_);
        ++nErased;
      }
    }
  } while (nErased > 0);

  // If any buckets for one of the hash functions is not empty, then we didn't
  // peel them all.
  for (vector<HashTableEntry>::const_iterator entry = peeled.hashTable_.begin();
       entry != peeled.hashTable_.end(); ++entry) {
    if (!entry->isEmpty())
      return false;
  }

  return true;
}

ptr_lib::shared_ptr<InvertibleBloomLookupTable>
InvertibleBloomLookupTable::difference
  (const InvertibleBloomLookupTable& other) const
{
  if (hashTable_.size() != other.hashTable_.size())
    throw runtime_error("IBLT difference: Both tables must be the same size");

  ptr_lib::shared_ptr<InvertibleBloomLookupTable> result =
    ptr_lib::make_shared<InvertibleBloomLookupTable>(*this);
  for (size_t i = 0; i < hashTable_.size(); i++) {
    HashTableEntry& e1 = result->hashTable_.at(i);
    const HashTableEntry& e2 = other.hashTable_.at(i);
    e1.count_ -= e2.count_;
    e1.keySum_ ^= e2.keySum_;
    e1.keyCheck_ ^= e2.keyCheck_;
  }

  return result;
}

Blob
InvertibleBloomLookupTable::encode() const
{
  size_t nEntries = hashTable_.size();
  size_t unitSize = (32 * 3) / 8; // hard coding
  size_t tableSize = unitSize * nEntries;

  vector<uint8_t> table(tableSize);

  for (size_t i = 0; i < nEntries; i++) {
    const HashTableEntry& entry = hashTable_[i];

    // table[i*12],   table[i*12+1], table[i*12+2], table[i*12+3] --> hashTable[i].count_

    table[(i * unitSize)]   = 0xFF & entry.count_;
    table[(i * unitSize) + 1] = 0xFF & (entry.count_ >> 8);
    table[(i * unitSize) + 2] = 0xFF & (entry.count_ >> 16);
    table[(i * unitSize) + 3] = 0xFF & (entry.count_ >> 24);

    // table[i*12+4], table[i*12+5], table[i*12+6], table[i*12+7] --> hashTable[i].keySum_

    table[(i * unitSize) + 4] = 0xFF & entry.keySum_;
    table[(i * unitSize) + 5] = 0xFF & (entry.keySum_ >> 8);
    table[(i * unitSize) + 6] = 0xFF & (entry.keySum_ >> 16);
    table[(i * unitSize) + 7] = 0xFF & (entry.keySum_ >> 24);

    // table[i*12+8], table[i*12+9], table[i*12+10], table[i*12+11] --> hashTable[i].keyCheck_

    table[(i * unitSize) + 8] = 0xFF & entry.keyCheck_;
    table[(i * unitSize) + 9] = 0xFF & (entry.keyCheck_ >> 8);
    table[(i * unitSize) + 10] = 0xFF & (entry.keyCheck_ >> 16);
    table[(i * unitSize) + 11] = 0xFF & (entry.keyCheck_ >> 24);
  }

  return zlibCompress(table.data(), table.size());
}

bool
InvertibleBloomLookupTable::equals(const InvertibleBloomLookupTable& other) const
{
  const vector<HashTableEntry>& iblt1HashTable = hashTable_;
  const vector<HashTableEntry>& iblt2HashTable = other.hashTable_;
  if (iblt1HashTable.size() != iblt2HashTable.size())
    return false;

  for (size_t i = 0; i < iblt1HashTable.size(); i++) {
    if (iblt1HashTable[i].count_ != iblt2HashTable[i].count_ ||
        iblt1HashTable[i].keySum_ != iblt2HashTable[i].keySum_ ||
        iblt1HashTable[i].keyCheck_ != iblt2HashTable[i].keyCheck_)
      return false;
  }

  return true;
}

void
InvertibleBloomLookupTable::update(int plusOrMinus, uint32_t key)
{
  size_t bucketsPerHash = hashTable_.size() / N_HASH;

  for (size_t i = 0; i < N_HASH; i++) {
    size_t startEntry = i * bucketsPerHash;
    uint32_t h = CryptoLite::murmurHash3(i, key);
    HashTableEntry& entry = hashTable_.at(startEntry + (h % bucketsPerHash));
    entry.count_ += plusOrMinus;
    entry.keySum_ ^= key;
    entry.keyCheck_ ^= CryptoLite::murmurHash3(N_HASHCHECK, key);
  }
}

vector<uint32_t>
InvertibleBloomLookupTable::decode(const Blob& encoding)
{
  Blob ibltValues = zlibDecompress(encoding.buf(), encoding.size());

  size_t nEntries = ibltValues.size() / 4;
  vector<uint32_t> values(nEntries, 0);

  const uint8_t* ibltValuesBuf = ibltValues.buf();
  for (size_t i = 0; i < 4 * nEntries; i += 4) {
    uint32_t t = (ibltValuesBuf[i + 3] << 24) +
                 (ibltValuesBuf[i + 2] << 16) +
                 (ibltValuesBuf[i + 1] << 8)  +
                 ibltValuesBuf[i];
    values[i / 4] = t;
  }

  return values;
}

Blob
InvertibleBloomLookupTable::zlibCompress(const uint8_t* data, size_t dataLength)
{
  // From https://www.zlib.net/zlib_how.html

  // Use a vector in a shared_ptr so we can make it a Blob without copying.
  ptr_lib::shared_ptr<vector<uint8_t>> result(new vector<uint8_t>());
  uint8_t outBuffer[16384];

  z_stream stream;
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  deflateInit(&stream, Z_BEST_COMPRESSION);

  stream.next_in = const_cast<uint8_t*>(data);
  stream.avail_in = dataLength;

  int status;
  do {
    stream.avail_out = sizeof(outBuffer);
    stream.next_out = outBuffer;
    status = deflate(&stream, Z_FINISH);
    if (!(status == Z_OK || status == Z_STREAM_END))
      // We don't expect a compression error.
      throw runtime_error("zlibCompress: Error during deflate");

    size_t have = sizeof(outBuffer) - stream.avail_out;
    result->insert(result->end(), outBuffer, outBuffer + have);
  } while (stream.avail_out == 0);

  deflateEnd(&stream);
  if (status != Z_STREAM_END)
    // We don't expect this error since stream.avail_out == 0.
    throw runtime_error("zlibCompress: status is not Z_STREAM_END");

  return Blob(result, false);
}

Blob
InvertibleBloomLookupTable::zlibDecompress(const uint8_t* data, size_t dataLength)
{
  // From https://www.zlib.net/zlib_how.html

  // Use a vector in a shared_ptr so we can make it a Blob without copying.
  ptr_lib::shared_ptr<vector<uint8_t>> result(new vector<uint8_t>());
  uint8_t outBuffer[16384];

  z_stream stream;
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  stream.avail_in = 0;
  stream.next_in = Z_NULL;
  inflateInit(&stream);

  stream.avail_in = dataLength;
  stream.next_in = const_cast<uint8_t*>(data);

  int status;
  do {
    stream.avail_out = sizeof(outBuffer);
    stream.next_out = outBuffer;
    status = inflate(&stream, Z_NO_FLUSH);
    if (!(status == Z_OK || status == Z_STREAM_END))
      throw runtime_error("zlibDecompress: Error during inflate");

    size_t have = sizeof(outBuffer) - stream.avail_out;
    result->insert(result->end(), outBuffer, outBuffer + have);
  } while (stream.avail_out == 0);

  inflateEnd(&stream);
  if (status != Z_STREAM_END)
    // We don't expect this error since stream.avail_out == 0.
    throw runtime_error("zlibDecompress: status is not Z_STREAM_END");

  return Blob(result, false);
}

bool
InvertibleBloomLookupTable::HashTableEntry::isPure() const
{
  if (count_ == 1 || count_ == -1) {
    uint32_t check = CryptoLite::murmurHash3(N_HASHCHECK, keySum_);
    return keyCheck_ == check;
  }

  return false;
}

}

#endif // NDN_CPP_HAVE_LIBZ
