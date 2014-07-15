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
#include <poll.h>
#include "../src/util/chrono-sync/chat.hpp"
#include "../src/util/chrono-sync/chrono-chat.hpp"
#include "../src/c/util/time.h"

using namespace std;
using namespace ndn;

static bool
isStdinReady()
{
  struct pollfd pollInfo;
  pollInfo.fd = STDIN_FILENO;
  pollInfo.events = POLLIN;

  return poll(&pollInfo, 1, 0) > 0;
}

int main(int argc, char** argv)
{
  try {
    ChronoChat::start("user1", "ndnchat7", "localhost");

    MillisecondsSince1970 start = ndn_getNowMilliseconds();
    bool sentMessage1 = false;
    bool sentMessage2 = false;
    while (true) {
      if (!sentMessage1 && ndn_getNowMilliseconds() - start > 10000) {
        sentMessage1 = true;
        ChronoChat::chat->sendMessage();
      }
#if 0
      if (!sentMessage2 && ndn_getNowMilliseconds() - start > 20000) {
        sentMessage2 = true;
        ChronoChat::chat->sendMessage();
      }
#endif
      ChronoChat::face->processEvents();
      // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
      usleep(10000);
     }
  } catch (std::exception& e) {
    cout << "exception: " << e.what() << endl;
  }
  return 0;
}
