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

#include "crypto.h"

#ifdef ARDUINO

void ndn_generateRandomBytes(uint8_t *buffer, size_t bufferLength)
{
  // Assume the application has already initialized it, e.g.:
  // randomSeed(analogRead(0));
  size_t i;
  for (i = 0; i < bufferLength; ++i)
    buffer[i] = random(0, 256);
}

#else

#include <openssl/ssl.h>
#include <openssl/rand.h>

void ndn_digestSha256(const uint8_t *data, size_t dataLength, uint8_t *digest)
{
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, data, dataLength);
  SHA256_Final(digest, &sha256);
}

void ndn_generateRandomBytes(uint8_t *buffer, size_t bufferLength)
{
  RAND_bytes(buffer, (int)bufferLength);
}

#endif
