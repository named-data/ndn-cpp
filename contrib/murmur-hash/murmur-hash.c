/**
 * murmurHash3 was written by Austin Appleby, and is placed in the public
 * domain. The author hereby disclaims copyright to this source code.
 * https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
 */

#include "murmur-hash.h"

static uint32_t
ROTL32(uint32_t x, int8_t r)
{
  return (x << r) | (x >> (32 - r));
}

uint32_t
ndn_murmurHash3
  (uint32_t nHashSeed, const uint8_t* dataToHash, size_t dataToHashLength)
{
  // Imitate MurmurHash3_x86_32
  // https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp#L94

  uint32_t h1 = nHashSeed;
  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;

  const size_t nblocks = dataToHashLength / 4;

  //----------
  // body
  const uint32_t* blocks = (const uint32_t*)(dataToHash + nblocks * 4);

  for (int i = -nblocks; i < 0; i++) {
    // Note that this indexes backwards from the end of the array.
    uint32_t k1 = blocks[i];

    k1 *= c1;
    k1 = ROTL32(k1,15);
    k1 *= c2;

    h1 ^= k1;
    h1 = ROTL32(h1,13);
    h1 = h1*5+0xe6546b64;
  }

  //----------
  // tail
  const uint8_t* tail = dataToHash + nblocks * 4;

  uint32_t k1 = 0;

  switch (dataToHashLength & 3) {
    case 3:
      k1 ^= tail[2] << 16;
      // fall through...

    case 2:
      k1 ^= tail[1] << 8;
      // fall through...

    case 1:
      k1 ^= tail[0];
      k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
  }

  //----------
  // finalization
  h1 ^= dataToHashLength;
  h1 ^= h1 >> 16;
  h1 *= 0x85ebca6b;
  h1 ^= h1 >> 13;
  h1 *= 0xc2b2ae35;
  h1 ^= h1 >> 16;

  return h1;
}
