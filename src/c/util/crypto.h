/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#ifndef NDN_CRYPTO_H
#define NDN_CRYPTO_H

#include <openssl/ssl.h>
#include <ndn-cpp/c/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compute the sha-256 digest of data.
 * @param data Pointer to the input byte array.
 * @param dataLength The length of data.
 * @param digest A pointer to a buffer of size SHA256_DIGEST_LENGTH to receive the data.
 */
void ndn_digestSha256(const uint8_t *data, size_t dataLength, uint8_t *digest);

/**
 * Fill the buffer with random bytes.
 * @param buffer Write the random bytes to this buffer.
 * @param bufferLength The number of bytes to write to buffer.
 */
void ndn_generateRandomBytes(uint8_t *buffer, size_t bufferLength);

#ifdef __cplusplus
}
#endif

#endif
