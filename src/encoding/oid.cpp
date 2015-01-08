/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
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

#include <stdlib.h>
#include <sstream>

#include <ndn-cpp/encoding/oid.hpp>

using namespace std;

namespace ndn {

OID::OID(const string& oid)
{
  string str = oid + ".";

  size_t pos = 0;
  size_t ppos = 0;

  while(string::npos != pos){
    ppos = pos;

    pos = str.find_first_of('.', pos);
    if(pos == string::npos)
      break;

    oid_.push_back(atoi(str.substr(ppos, pos - ppos).c_str()));

    pos++;
  }
}

void
OID::setIntegerList(const int* value, size_t valueLength)
{
  oid_.clear();
  for (size_t i = 0; i < valueLength; ++i)
    oid_.push_back(value[i]);
}

string OID::toString() const
{
  ostringstream convert;

  vector<int>::const_iterator it = oid_.begin();
  for(; it < oid_.end(); it++){
    if(it != oid_.begin())
      convert << ".";
    convert << *it;
  }

  return convert.str();
}

bool OID::equal(const OID& oid) const
{
  vector<int>::const_iterator i = oid_.begin();
  vector<int>::const_iterator j = oid.oid_.begin();

  for (; i != oid_.end () && j != oid.oid_.end (); i++, j++) {
    if(*i != *j)
      return false;
  }

  if (i == oid_.end () && j == oid.oid_.end ())
    return true;
  else
    return false;
}

}
