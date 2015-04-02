/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015 Regents of the University of California.
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
 * This sends interests for /testarduino/voltage and prints the readings
 * returned by the Arduino sketch examples/arduino/analog-reading.
 */

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <ndn-cpp/face.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

static void
processReading
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data, Face* face, bool* enabled);

static void
onTimeout(const ptr_lib::shared_ptr<const Interest>& interest, bool* enabled);

int main(int argc, char** argv)
{
  try {
    // The default Face connects to the local NFD.
    Face face;

    // First get the latest reading (rightmost child) in the forwarder's content
    // store, if any.
    Name prefix("/testarduino/voltage");
    Interest interest(prefix);
    interest.setChildSelector(1);
    interest.setInterestLifetimeMilliseconds(4000);

    bool enabled = true;
    face.expressInterest
      (interest, bind(&processReading, _1, _2, &face, &enabled),
       bind(&onTimeout, _1, &enabled));

    // Loop calling processEvents until a callback sets enabled = false.
    while (enabled) {
      face.processEvents();
      // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
      usleep(10000);
    }
  } catch (std::exception& e) {
    cout << "exception: " << e.what() << endl;
  }
  return 0;
}

/**
 * This is called after expressing an interest for a reading. (The first answer
 * may be the latest reading from the forwarder's content store.) Express an
 * interest for the next reading.
 * @param expressedInterest The expressed interest.
 * @param data The reading data packet.
 * @param face The Face which is used to call expressInterest.
 * @param enabled On success or error, set *enabled = false.
 */
static void
processReading
  (const ptr_lib::shared_ptr<const Interest>& expressedInterest,
   const ptr_lib::shared_ptr<Data>& data, Face *face, bool* enabled)
{
  cout << "Got data packet " << data->getName().toUri() << endl;

  const size_t nExpectedComponents = 3;
  if (data->getName().size() != nExpectedComponents) {
    // We don't expect this to happen.
    cout << "Expected " << nExpectedComponents << " but got " <<
      data->getName().size() << endl;
    *enabled = false;
    return;
  }

  cout << "Reading #" << data->getName()[2].getValue().toRawStr() << ": " <<
    data->getContent().toRawStr() << endl;

  // Exclude up to the reading number and get the lowest reading number
  // (leftmost child) after that.
  Interest interest(data->getName().getPrefix(2));
  interest.getExclude().appendAny();
  interest.getExclude().appendComponent(data->getName()[2]);
  interest.setChildSelector(0);
  interest.setInterestLifetimeMilliseconds(4000);

  // Express the interest to call this callback again, looping forever.
  face->expressInterest
    (interest, bind(&processReading, _1, _2, face, enabled),
     bind(&onTimeout, _1, enabled));
}

/**
 * This is called when the interest times out. Print a message and quit.
 * @param interest The timed-out interest.
 * @param enabled On success or error, set *enabled = false.
 */
static void
onTimeout(const ptr_lib::shared_ptr<const Interest>& interest, bool* enabled)
{
  cout << "Interest timed out for " << interest->toUri() << endl;
  *enabled = false;
}
