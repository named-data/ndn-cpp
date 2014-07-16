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

using namespace std;
using namespace ndn;

static const char *WHITESPACE_CHARS = " \n\r\t";

/**
 * Modify str in place to erase whitespace on the left.
 * @param str
 */
static inline void
trimLeft(string& str)
{
  size_t found = str.find_first_not_of(WHITESPACE_CHARS);
  if (found != string::npos) {
    if (found > 0)
      str.erase(0, found);
  }
  else
    // All whitespace
    str.clear();
}

/**
 * Modify str in place to erase whitespace on the right.
 * @param str
 */
static inline void
trimRight(string& str)
{
  size_t found = str.find_last_not_of(WHITESPACE_CHARS);
  if (found != string::npos) {
    if (found + 1 < str.size())
      str.erase(found + 1);
  }
  else
    // All whitespace
    str.clear();
}

/**
 * Modify str in place to erase whitespace on the left and right.
 * @param str
 */
static void
trim(string& str)
{
  trimLeft(str);
  trimRight(str);
}

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
    ChronoChat::start("user1", "ndnchat1", "localhost");

    while (true) {
      if (isStdinReady()) {
        char inputBuffer[256];
        ssize_t nBytes = ::read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer) - 1);
        if (nBytes < 0)
          // Don't expect an error reading from stdin.
          return -1;

        inputBuffer[nBytes] = 0;
        string input(inputBuffer);
        trim(input);
        
        ChronoChat::chat->sendMessage(input);
      }

      ChronoChat::face->processEvents();
      // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
      usleep(10000);
     }
  } catch (std::exception& e) {
    cout << "exception: " << e.what() << endl;
  }
  return 0;
}
