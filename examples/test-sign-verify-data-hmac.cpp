/**
 * Copyright (C) 2016 Regents of the University of California.
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

/**
 * This decodes and verifies a hard-coded data packet with an HMAC signature.
 * Then this creates a fresh data packet with an HMAC signature, signs and
 * verifies it. For testing, this uses a hard-coded shared secret.
 */

#include <cstdlib>
#include <iostream>
#include <time.h>
#include <ndn-cpp/data.hpp>
#include <ndn-cpp/security/key-chain.hpp>
#include <ndn-cpp/hmac-with-sha256-signature.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

uint8_t TlvData[] = {
0x06, 0x57, // NDN Data
  0x07, 0x0a, // Name
    0x08, 0x03, 0x6e, 0x64, 0x6e,
    0x08, 0x03, 0x61, 0x62, 0x63,
  0x14, 0x00, // MetaInfo
  0x15, 0x08, 0x53, 0x55, 0x43, 0x43, 0x45, 0x53, 0x53, 0x21, // Content
  0x16, 0x1b, // SignatureInfo
    0x1b, 0x01, 0x04, // SignatureType = SignatureHmacWithSha256
    0x1c, 0x16, // KeyLocator
      0x07, 0x14, // Name
        0x08, 0x03, 0x4b, 0x45, 0x59,
        0x08, 0x04, 0x6b, 0x65, 0x79, 0x31,
        0x08, 0x07, 0x49, 0x44, 0x2d, 0x43, 0x45, 0x52, 0x54,
  0x17, 0x20, // SignatureValue
    0xad, 0x05, 0x25, 0xa3, 0x5a, 0x75, 0x49, 0x05, 0x9b, 0x18, 0x1f, 0x69, 0xb5, 0x37, 0xc3, 0x4a,
    0x4f, 0x2a, 0x29, 0x0b, 0x18, 0x05, 0x46, 0x4b, 0x87, 0x38, 0xf0, 0xfc, 0xbb, 0xaf, 0x47, 0x15
};

int main(int argc, char** argv)
{
  try {
    Data data;
    data.wireDecode(TlvData, sizeof(TlvData));

    // Use a hard-wired secret for testing. In a real application the signer
    // ensures that the verifier knows the shared key and its keyName.
    const uint8_t keyBytes[] = {
       0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
      16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    };
    Blob key(keyBytes, sizeof(keyBytes));

    if (KeyChain::verifyDataWithHmacWithSha256(data, key))
      cout << "Hard-coded data signature verification: VERIFIED" << endl;
    else
      cout << "Hard-coded data signature verification: FAILED" << endl;

    Data freshData(Name("/ndn/abc"));
    HmacWithSha256Signature signature;
    freshData.setSignature(signature);
    const uint8_t freshContent[] = "SUCCESS!";
    freshData.setContent(freshContent, sizeof(freshContent) - 1);
    cout << "Signing fresh data packet " << freshData.getName().toUri() << endl;
    KeyChain::signWithHmacWithSha256(freshData, key);

    if (KeyChain::verifyDataWithHmacWithSha256(freshData, key))
      cout << "Freshly-signed data signature verification: VERIFIED" << endl;
    else
      cout << "Freshly-signed data signature verification: FAILED" << endl;
  } catch (std::exception& e) {
    cout << "exception: " << e.what() << endl;
  }
  return 0;
}
