/**
 * Copyright (C) 2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From PSync unit tests:
 * https://github.com/named-data/PSync/blob/master/tests/test-iblt.cpp
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

#include "gtest/gtest.h"
#include <ndn-cpp/name.hpp>
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include "../../src/sync/detail/invertible-bloom-lookup-table.hpp"

using namespace std;
using namespace ndn;

class TestInvertibleBloomLookupTable : public ::testing::Test {
};

TEST_F(TestInvertibleBloomLookupTable, Equal)
{
  size_t size = 10;

  InvertibleBloomLookupTable iblt1(size);
  InvertibleBloomLookupTable iblt2(size);

  ASSERT_TRUE(iblt1.equals(iblt2));

  string prefix = Name("/test/memphis").appendNumber(1).toUri();
  uint32_t newHash = CryptoLite::murmurHash3(11, prefix.data(), prefix.size());
  iblt1.insert(newHash);
  iblt2.insert(newHash);
  ASSERT_TRUE(iblt1.equals(iblt2));

  Name ibfName1("/sync"), ibfName2("/sync");
  ibfName1.append(iblt1.encode());
  ibfName2.append(iblt2.encode());
  ASSERT_TRUE(ibfName1.equals(ibfName2));
}

TEST_F(TestInvertibleBloomLookupTable, NameAppendAndExtract)
{
  size_t size = 10;

  InvertibleBloomLookupTable iblt(size);
  string prefix = Name("/test/memphis").appendNumber(1).toUri();
  uint32_t newHash = CryptoLite::murmurHash3(11, prefix.data(), prefix.size());
  iblt.insert(newHash);

  uint8_t expectedEncoding[] = {
    0x78, 0xda, 0x63, 0x64, 0x60, 0x60, 0xd8, 0x55, 0xb5, 0xfc,
    0x5b, 0xb2, 0xef, 0xe2, 0x6c, 0x06, 0x0a, 0x00, 0x23, 0x1d,
    0xcd, 0x01, 0x00, 0x65, 0x29, 0x0d, 0xb1
  };

  Name ibltName("sync");
  Blob encodedIblt = iblt.encode();
  ASSERT_TRUE(encodedIblt.equals(Blob(expectedEncoding, sizeof(expectedEncoding))));
  ibltName.append(encodedIblt);

  InvertibleBloomLookupTable received(size);
  received.initialize(ibltName.get(-1).getValue());

  ASSERT_TRUE(iblt.equals(received));

  InvertibleBloomLookupTable receivedDifferentSize(20);
  ASSERT_THROW(receivedDifferentSize.initialize(ibltName.get(-1).getValue()),
               runtime_error);
}

TEST_F(TestInvertibleBloomLookupTable, CopyInsertErase)
{
  size_t size = 10;

  InvertibleBloomLookupTable iblt1(size);

  string prefix = Name("/test/memphis").appendNumber(1).toUri();
  uint32_t hash1 = CryptoLite::murmurHash3(11, prefix.data(), prefix.size());
  iblt1.insert(hash1);

  InvertibleBloomLookupTable iblt2(iblt1);
  iblt2.erase(hash1);
  prefix = Name("/test/memphis").appendNumber(2).toUri();
  uint32_t hash3 = CryptoLite::murmurHash3(11, prefix.data(), prefix.size());
  iblt2.insert(hash3);

  iblt1.erase(hash1);
  prefix = Name("/test/memphis").appendNumber(5).toUri();
  uint32_t hash5 = CryptoLite::murmurHash3(11, prefix.data(), prefix.size());
  iblt1.insert(hash5);

  iblt2.erase(hash3);
  iblt2.insert(hash5);

  ASSERT_TRUE(iblt1.equals(iblt2));
}

TEST_F(TestInvertibleBloomLookupTable, HigherSequence)
{
  // This is the case where we can't recognize if the received IBF has a higher
  // sequence number. This is relevant to the full sync case.
  size_t size = 10;

  InvertibleBloomLookupTable ownIBLT(size);
  InvertibleBloomLookupTable receivedIBLT(size);

  string prefix = Name("/test/memphis").appendNumber(3).toUri();
  uint32_t hash1 = CryptoLite::murmurHash3(11, prefix.data(), prefix.size());
  ownIBLT.insert(hash1);

  string prefix2 = Name("/test/memphis").appendNumber(4).toUri();
  uint32_t hash2 = CryptoLite::murmurHash3(11, prefix2.data(), prefix2.size());
  receivedIBLT.insert(hash2);

  ptr_lib::shared_ptr<InvertibleBloomLookupTable> diff =
    ownIBLT.difference(receivedIBLT);
  set<uint32_t> positive;
  set<uint32_t> negative;

  ASSERT_TRUE(diff->listEntries(positive, negative));
  ASSERT_EQ(1, positive.size());
  ASSERT_TRUE(*positive.begin() == hash1);

  ASSERT_EQ(1, negative.size());
  ASSERT_TRUE(*negative.begin() == hash2);
}

TEST_F(TestInvertibleBloomLookupTable, Difference)
{
  size_t size = 10;

  InvertibleBloomLookupTable ownIBLT(size);

  InvertibleBloomLookupTable receivedIBLT(ownIBLT);

  ptr_lib::shared_ptr<InvertibleBloomLookupTable> diff =
    ownIBLT.difference(receivedIBLT);

  // Non-empty positive means we have some elements that the other doesn't.
  set<uint32_t> positive;
  set<uint32_t> negative;

  ASSERT_TRUE(diff->listEntries(positive, negative));
  ASSERT_EQ(0, positive.size());
  ASSERT_EQ(0, negative.size());

  string prefix = Name("/test/memphis").appendNumber(1).toUri();
  uint32_t newHash = CryptoLite::murmurHash3(11, prefix.data(), prefix.size());
  ownIBLT.insert(newHash);

  diff = ownIBLT.difference(receivedIBLT);
  ASSERT_TRUE(diff->listEntries(positive, negative));
  ASSERT_EQ(1, positive.size());
  ASSERT_EQ(0, negative.size());

  prefix = Name("/test/csu").appendNumber(1).toUri();
  newHash = CryptoLite::murmurHash3(11, prefix.data(), prefix.size());
  receivedIBLT.insert(newHash);

  diff = ownIBLT.difference(receivedIBLT);
  ASSERT_TRUE(diff->listEntries(positive, negative));
  ASSERT_EQ(1, positive.size());
  ASSERT_EQ(1, negative.size());
}

TEST_F(TestInvertibleBloomLookupTable, DifferenceBwOversizedIBLTs)
{
  // Insert 50 elements into an IBLT of size 10. Then check that we can still
  // list the difference even though we can't list the IBLT itself.

  size_t size = 10;

  InvertibleBloomLookupTable ownIBLT(size);

  for (int i = 0; i < 50; ++i) {
    string prefix = Name("/test/memphis" + std::to_string(i)).appendNumber(1).toUri();
    uint32_t newHash = CryptoLite::murmurHash3(11, prefix.data(), prefix.size());
    ownIBLT.insert(newHash);
  }

  InvertibleBloomLookupTable receivedIBLT(ownIBLT);

  string prefix = Name("/test/ucla").appendNumber(1).toUri();
  uint32_t newHash = CryptoLite::murmurHash3(11, prefix.data(), prefix.size());
  ownIBLT.insert(newHash);

  ptr_lib::shared_ptr<InvertibleBloomLookupTable> diff =
    ownIBLT.difference(receivedIBLT);

  set<uint32_t> positive;
  set<uint32_t> negative;
  ASSERT_TRUE(diff->listEntries(positive, negative));
  ASSERT_EQ(1, positive.size());
  ASSERT_EQ(newHash, *positive.begin());
  ASSERT_EQ(0, negative.size());

  ASSERT_TRUE(!ownIBLT.listEntries(positive, negative));
  ASSERT_TRUE(!receivedIBLT.listEntries(positive, negative));
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
