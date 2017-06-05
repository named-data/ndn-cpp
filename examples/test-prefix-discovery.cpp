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

/* This tests the PrefixDiscovery class which periodically expresses the
 * interest to the local NFD for the routable prefix list. This requires a local
 * running NFD. This programs runs indefinitely until you stop it (^C),
 */

#include <cstdlib>
#include <unistd.h>
#include <ndn-cpp-tools/usersync/prefix-discovery.hpp>

using namespace std;
using namespace ndn;
using namespace ndntools;

static void
onPrefixes(const ptr_lib::shared_ptr<vector<Name> >& prefixes);

int main(int argc, char** argv)
{
  try {
    // The default Face will connect using a Unix socket, or to "localhost".
    Face face;

    PrefixDiscovery prefixDiscovery(&onPrefixes, &face);
    prefixDiscovery.start();

    while (true) {
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
 * This is called when a new prefix list (or the first list) is reported by the
 * local NFD.
 * @param prefix The new prefix list.
 */
static void
onPrefixes(const ptr_lib::shared_ptr<vector<Name> >& prefixes)
{
  cout << "Got prefix(es):";
  for (size_t i = 0; i < prefixes->size(); ++i)
    cout << " " << (*prefixes)[i].toUri();
  cout << endl;
}
