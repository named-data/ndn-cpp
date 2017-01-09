/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015-2017 Regents of the University of California.
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
 * This sends interests for /testarduino/voltage?<reading number> and prints the
 * readings returned by the Arduino sketch examples/arduino/analog-reading.
 */

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <ndn-cpp/face.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

class Callbacks
{
public:
  Callbacks(Face *face) {
    face_ = face;
    enabled_ = true;
    expectedReadingNumber_ = 1;
  }

  /**
   * Express and interest for "/testarduino/voltage/<reading number>" for the
   * expectedReadingNumber_.
   */
  void
  expressInterestForReadingNumber()
  {
    Interest interest(Name("/testarduino/voltage"));
    interest.getName().appendVersion(expectedReadingNumber_);
    interest.setInterestLifetimeMilliseconds(2000);

    face_->expressInterest
      (interest, bind(&Callbacks::onData, this, _1, _2),
       bind(&Callbacks::onTimeout, this, _1));
  }

  void
  onData
    (const ptr_lib::shared_ptr<const Interest>& interest,
     const ptr_lib::shared_ptr<Data>& data)
  {
    cout << "Got data packet " << data->getName().toUri() << endl;

    const size_t nExpectedComponents = 3;
    if (data->getName().size() != nExpectedComponents) {
      // We don't expect this to happen.
      cout << "Expected " << nExpectedComponents << " but got " <<
        data->getName().size() << ". Skipping." << endl;
      enabled_ = false;
      return;
    }

    uint64_t readingNumber = data->getName()[2].toVersion();
    if (readingNumber != expectedReadingNumber_)
      cout << "Expected reading #" << expectedReadingNumber_ << " but got #" <<
        readingNumber << ". Skipping." << endl;
    else {
      cout << "Reading #" << readingNumber << ": " <<
        data->getContent().toRawStr() << endl;

      // Get the next reading.
      ++expectedReadingNumber_;
      expressInterestForReadingNumber();
    }
  }

  void
  onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
  {
    cout << "Time out for interest " << interest->getName().toUri() << endl;
    enabled_ = false;
  }

  Face *face_;
  bool enabled_;
  uint64_t expectedReadingNumber_;
};

int main(int argc, char** argv)
{
  try {
    // The default Face connects to the local NFD.
    Face face;

    Callbacks callbacks(&face);

    // Send the first interest.
    callbacks.expressInterestForReadingNumber();

    // Loop calling processEvents until a callback sets callbacks.enabled_ = false.
    while (callbacks.enabled_) {
      face.processEvents();
      // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
      usleep(10000);
    }
  } catch (std::exception& e) {
    cout << "exception: " << e.what() << endl;
  }
  return 0;
}
