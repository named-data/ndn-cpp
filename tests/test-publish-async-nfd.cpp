/**
 * Copyright (C) 2013-2014 Regents of the University of California.
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#include <cstdlib>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <ndn-cpp/face.hpp>
#include <ndn-cpp/security/key-chain.hpp>

using namespace std;
using namespace ndn;

class Echo {
public:
  Echo(KeyChain &keyChain, const Name& certificateName)
  : keyChain_(keyChain), certificateName_(certificateName), responseCount_(0)
  {
  }

  // onInterest.
  void operator()
     (const ptr_lib::shared_ptr<const Name>& prefix,
      const ptr_lib::shared_ptr<const Interest>& interest, Transport& transport,
      uint64_t registeredPrefixId)
  {
    ++responseCount_;

    // Make and sign a Data packet.
    Data data(interest->getName());
    string content(string("Echo ") + interest->getName().toUri());
    data.setContent((const uint8_t *)&content[0], content.size());
    keyChain_.sign(data, certificateName_);
    Blob encodedData = data.wireEncode();

    cout << "Sent content " << content << endl;
    transport.send(*encodedData);
  }

  // onRegisterFailed.
  void operator()(const ptr_lib::shared_ptr<const Name>& prefix)
  {
    ++responseCount_;
    cout << "Register failed for prefix " << prefix->toUri() << endl;
  }

  KeyChain keyChain_;
  Name certificateName_;
  int responseCount_;
};

int main(int argc, char** argv)
{
  try {
    // The default Face will connect using a Unix socket, or to "localhost".
    Face face;

    // Use the system default key chain and certificate name to sign commands.
    KeyChain keyChain;
    face.setCommandSigningInfo(keyChain, keyChain.getDefaultCertificateName());

    // Also use the default certificate name to sign data packets.
    Echo echo(keyChain, keyChain.getDefaultCertificateName());
    Name prefix("/testecho");
    cout << "Register prefix  " << prefix.toUri() << endl;
    face.registerPrefix(prefix, func_lib::ref(echo), func_lib::ref(echo));

    // The main event loop.
    // Wait forever to receive one interest for the prefix.
    while (echo.responseCount_ < 1) {
      face.processEvents();
      // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
      usleep(10000);
    }
  } catch (std::exception& e) {
    cout << "exception: " << e.what() << endl;
  }
  return 0;
}
