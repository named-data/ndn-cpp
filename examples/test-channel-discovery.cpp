/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
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

/* This tests the ndntools ChannelDiscovery which listens for channel
 * announcements and lets you enter the name of a new channel which is
 * announced to others. This requires a local running NFD.
 */

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_PROTOBUF = 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_PROTOBUF

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <unistd.h>
#include <poll.h>
#include <ndn-cpp-tools/usersync/channel-discovery.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;
using namespace ndntools;

static void
onReceivedChannelList
  (ChannelDiscovery& channelDiscovery,
   const ptr_lib::shared_ptr<Name>& userPrefix);

static void
onError
  (ChannelDiscovery::ErrorCode errorCode, const string& message, bool* enabled);

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

/**
 * Poll stdin and return true if it is ready to ready (e.g. from stdinReadLine).
 */
static bool
isStdinReady()
{
  struct pollfd pollInfo;
  pollInfo.fd = STDIN_FILENO;
  pollInfo.events = POLLIN;

  return poll(&pollInfo, 1, 0) > 0;
}

/**
 * Read a line from from stdin and return a trimmed string.  (We don't use
 * cin because it ignores a blank line.)
 */
static string
stdinReadLine()
{
  char inputBuffer[1000];
  ssize_t nBytes = ::read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer) - 1);
  if (nBytes < 0)
    // Don't expect an error reading from stdin.
    throw runtime_error("stdinReadLine: error reading from STDIN_FILENO");

  inputBuffer[nBytes] = 0;
  string input(inputBuffer);
  trim(input);

  return input;
}

int main(int argc, char** argv)
{
  try {
    const char* home = getenv("HOME");
    if (!home || *home == '\0')
      // Don't expect this to happen;
      home = ".";
    string homeDir(home);
    if (homeDir[homeDir.size() - 1] == '/' || homeDir[homeDir.size() - 1] == '\\')
      // Strip the ending path separator.
      homeDir.erase(homeDir.size() - 1);

    cout << "Enter your username:" << endl;
    string username = stdinReadLine();
    string channelListFilePath = homeDir + '/' + "channelList." + username + ".data";

    KeyChain keyChain;
    Name certificateName = keyChain.getDefaultCertificateName();
    // The default Face will connect using a Unix socket, or to "localhost".
    Face face;
    face.setCommandSigningInfo(keyChain, certificateName);
    bool enabled = true;

    Name applicationDataPrefix("/ndn/edu/ucla/remap");
    applicationDataPrefix.append(username).append("flume").append("discovery");
    Milliseconds syncLifetime = 5000.0;
    ChannelDiscovery channelDiscovery
      (applicationDataPrefix, channelListFilePath, 
       Name("/ndn/broadcast/flume/discovery"), face, keyChain, certificateName,
       syncLifetime, bind(&onReceivedChannelList, _1, _2),
       bind(&onError, _1, _2, &enabled));

    // The main loop to process messages while checking stdin to send an announcement.
    cout << "Enter the new channel name. To remove, prepend '-' such as '-mychannel'." << endl;
    cout << "To quit, enter \"exit\"." << endl;
    while (true) {
      if (isStdinReady()) {
        string channelName = stdinReadLine();
        if (channelName == "exit")
          break;

        bool remove = false;
        if (channelName.size() > 0 && channelName[0] == '-') {
          remove = true;
          channelName = channelName.substr(1);
          trim(channelName);
        }

        if (channelName != "") {
          if (remove)
            channelDiscovery.removeChannel(channelName);
          else
            channelDiscovery.addChannel(channelName);
        }
      }

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
 * This is called when a new channel list is received from another user.
 * @param channelDiscovery The ChannelDiscovery object.
 * @param userPrefix The Name of the user who updated the channel list.
 */
static void
onReceivedChannelList
  (ChannelDiscovery& channelDiscovery,
   const ptr_lib::shared_ptr<Name>& userPrefix)
{
  ptr_lib::shared_ptr<vector<string>> channelList =
    channelDiscovery.getChannelList(*userPrefix);
  cout << "Received channel list from user " << userPrefix->toUri() << ":" << endl;
  if (channelList->size() == 0)
    cout << "  <empty list>" << endl;
  else {
    for (size_t i = 0; i < channelList->size(); ++i)
      cout << "  " << channelList->at(i) << endl;
  }
}

/**
 * This is called to print an error from ChannelDiscovery.
 * @param errorCode The error code.
 * @param message The error message.
 * @param enabled On success or error, set *enabled = false.
 */
static void
onError
  (ChannelDiscovery::ErrorCode errorCode, const string& message, bool* enabled)
{
  *enabled = false;
  cout << message << endl;
}

#else // NDN_CPP_HAVE_PROTOBUF

#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
  cout <<
    "This program uses Protobuf but it is not installed. Install it and ./configure again." << endl;
}

#endif // NDN_CPP_HAVE_PROTOBUF
