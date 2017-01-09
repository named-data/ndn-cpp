/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: From https://github.com/named-data/ndn-cxx/blob/master/src/util/config-file.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <ndn-cpp/common.hpp>
#include "config-file.hpp"

using namespace std;

namespace ndn {

ConfigFile::ConfigFile()
  : path_(findConfigFile())
{
  if (!path_.empty())
    parse();
}

ConfigFile::Error::~Error() throw()
{
}

string
ConfigFile::findConfigFile()
{
  // Note: We don't use <filesystem> support because it is not "header-only" and
  // requires linking to libraries.
  // TODO: Handle non-unix file system paths which don't use '/'.
  const char* home = getenv("HOME");
  if (home && *home != '\0') {
    string homeDir(home);
    if (homeDir[homeDir.size() - 1] == '/' || homeDir[homeDir.size() - 1] == '\\')
      // Strip the ending path separator.
      homeDir.erase(homeDir.size() - 1);

    string filePath = homeDir + '/' + ".ndn/client.conf";
    ifstream file(filePath.c_str());
    if (file.good())
      return filePath;
  }

#ifdef NDN_CPP_SYSCONFDIR
  {
    string sysConfDir(NDN_CPP_SYSCONFDIR);
    if (sysConfDir[sysConfDir.size() - 1] == '/' || sysConfDir[sysConfDir.size() - 1] == '\\')
      // Strip the ending path separator.
      sysConfDir.erase(sysConfDir.size() - 1);

    string filePath = sysConfDir + '/' + "ndn/client.conf";
    ifstream file(filePath.c_str());
    if (file.good())
      return filePath;
  }
#endif

  {
    string filePath("/etc/ndn/client.conf");
    ifstream file(filePath.c_str());
    if (file.good())
      return filePath;
  }

  return "";
}

void
ConfigFile::parse()
{
  if (path_.empty())
    throw Error("ConfigFile::parse: Failed to locate the configuration file for parsing");

  ifstream input(path_.c_str());
  string line;
  while (getline(input, line)) {
    ndn_trim(line);
    if (line.empty() || line[0] == ';')
      // Skip empty lines and comments.
      continue;

    size_t iSeparator = line.find('=');
    if (iSeparator == string::npos)
      continue;

    string key = line.substr(0, iSeparator);
    string value = line.substr(iSeparator + 1);
    ndn_trim(key);
    ndn_trim(value);

    config_[key] = value;
  }
}

}
