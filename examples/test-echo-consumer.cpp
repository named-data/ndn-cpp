/**
 * Copyright (C) 2014-2015 Regents of the University of California.
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

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <ndn-cpp/face.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

class Counter
{
public:
  Counter() {
    callbackCount_ = 0;
  }

  void onData(const ptr_lib::shared_ptr<const Interest>& interest, const ptr_lib::shared_ptr<Data>& data)
  {
    ++callbackCount_;
    cout << "Got data packet with name " << data->getName().toUri() << endl;
    for (size_t i = 0; i < data->getContent().size(); ++i)
      cout << (*data->getContent())[i];
    cout << endl;
  }

  void onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
  {
    ++callbackCount_;
    cout << "Time out for interest " << interest->getName().toUri() << endl;
  }

  int callbackCount_;
};

int main(int argc, char** argv)
{
  try {
    // The default Face will connect using a Unix socket, or to "localhost".
    Face face;

    // Counter holds data used by the callbacks.
    Counter counter;

    string word;
    cout << "Enter a word to echo:" << endl;
    cin >> word;

    Name name("/testecho");
    name.append(word);
    cout << "Express name " << name.toUri() << endl;
    // Use bind to pass the counter object to the callbacks.
    face.expressInterest(name, bind(&Counter::onData, &counter, _1, _2), bind(&Counter::onTimeout, &counter, _1));

    // The main event loop.
    while (counter.callbackCount_ < 1) {
      face.processEvents();
      // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
      usleep(10000);
    }
  } catch (std::exception& e) {
    cout << "exception: " << e.what() << endl;
  }
  return 0;
}
