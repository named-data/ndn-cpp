/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the PSync library https://github.com/named-data/PSync/blob/master/PSync/detail/iblt.hpp
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

#ifndef NDN_INVERTIBLE_BLOOM_LOOKUP_TABLE_HPP
#define NDN_INVERTIBLE_BLOOM_LOOKUP_TABLE_HPP

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_LIBZ 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_LIBZ

#include <set>
#include <ndn-cpp/util/blob.hpp>

namespace ndn {

/**
 * InvertibleBloomLookupTable implements an Invertible Bloom Lookup Table (IBLT)
 * (Invertible Bloom Filter). This is used by FullPSync2017.
 */
class InvertibleBloomLookupTable {
public:
  /**
   * Create an InvertibleBloomLookupTable.
   * @param expectedNEntries the expected number of entries in the IBLT.
   */
  InvertibleBloomLookupTable(size_t expectedNEntries);

  /**
   * Populate the hash table using the encoded array representation of the IBLT.
   * @param encoding The encoded representation of the IBLT.
   * @throws runtime_error if the size of the decoded values is not compatible
   * with this IBLT.
   */
  void
  initialize(const Blob& encoding);

  void
  insert(uint32_t key) { update(INSERT, key); }

  void
  erase(uint32_t key) { update(ERASE, key); }

  /**
   * List all the entries in the IBLT.
   * This is called on a difference of two IBLTs: ownIBLT - receivedIBLT.
   * Entries listed in positive are in ownIBLT but not in receivedIBLT.
   * Entries listed in negative are in receivedIBLT but not in ownIBLT.
   * @param positive Add positive entries to this set. This does not first clear
   * the set.
   * @param negative Add negative entries to this set. This does not first clear
   * the set.
   * @return True if decoding is completed successfully.
   */
  bool
  listEntries(std::set<uint32_t>& positive, std::set<uint32_t>& negative) const;

  /**
   * Get a new IBLT which is the difference of the other IBLT from this IBLT.
   * @param other The other IBLT.
   * @return A new IBLT of this - other.
   */
  ptr_lib::shared_ptr<InvertibleBloomLookupTable>
  difference(const InvertibleBloomLookupTable& other) const;

  /**
   * Encode this IBLT to a Blob. This encodes this hash table from a uint32_t
   * array to a uint8_t array. We create a uin8_t array 12 times the size of
   * the uint32_t array. We put the first count in the first 4 cells, keySum in
   * the next 4, and keyCheck in the next 4. We repeat for all the other cells
   * of the hash table. Then we append this uint8_t array to the name.
   * @return The encoded Blob.
   */
  Blob
  encode() const;

  /**
   * Check if this IBLT has the same number of entries as the other IBLT and
   * that they are equal.
   * @param other The other OBLT to check.
   * @return true if this IBLT is equal to the other, otherwise false.
   */
  bool
  equals(const InvertibleBloomLookupTable& other) const;

  bool
  operator== (const InvertibleBloomLookupTable &other) const { return equals(other); }

  bool
  operator!= (const InvertibleBloomLookupTable &other) const { return !equals(other); }

  static const size_t N_HASH = 3;
  static const size_t N_HASHCHECK = 11;

private:
  class HashTableEntry
  {
  public:
    int32_t count_;
    uint32_t keySum_;
    uint32_t keyCheck_;

    bool
    isPure() const;

    bool
    isEmpty() const
    {
      return count_ == 0 && keySum_ == 0 && keyCheck_ == 0;
    }
  };

  /**
   * Update the entries in hashTable_.
   * @param plusOrMinus The amount to update the count.
   * @param key The key for computing the entry.
   */
  void
  update(int plusOrMinus, uint32_t key);

  /**
   * Decode the IBLT from the Blob. This converts the Blob into a uint8_t array
   * which is then decoded to a uint32_t array.
   * @param encoding The encoded IBLT.
   * @return A uint32_t array representing the hash table of the IBLT.
   */
  static std::vector<uint32_t>
  decode(const Blob& encoding);

  static Blob
  zlibCompress(const uint8_t* data, size_t dataLength);

  static Blob
  zlibDecompress(const uint8_t* data, size_t dataLength);

  std::vector<HashTableEntry> hashTable_;

  static const int INSERT = 1;
  static const int ERASE = -1;
};

}

#endif // NDN_CPP_HAVE_LIBZ

#endif
