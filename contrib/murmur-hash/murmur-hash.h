/**
 * murmurHash3 was written by Austin Appleby, and is placed in the public
 * domain. The author hereby disclaims copyright to this source code.
 * https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
 */

#ifndef NDN_MURMUR_HASH_H
#define NDN_MURMUR_HASH_H

#include <ndn-cpp/c/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compute the MurmurHash3 of the data.
 * @param nHashSeed The hash seed.
 * @param dataToHash A pointer to the input byte array to hash.
 * @param dataToHashLength The length of the data to hash.
 * @return The hash value.
 */
uint32_t
ndn_murmurHash3
  (uint32_t nHashSeed, const uint8_t* dataToHash, size_t dataToHashLength);

#ifdef __cplusplus
}
#endif

#endif
