/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Yingdi Yu <yingdi@cs.ucla.edu>
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

#ifndef NDN_OID_HPP
#define NDN_OID_HPP

#include <vector>
#include <string>

namespace ndn {

class OID {
public:
  OID ()
  {
  }

  OID(const std::string& oid);

  OID(const std::vector<int>& oid)
  : oid_(oid)
  {
  }

  OID(const int* integerList, size_t integerListLength)
  {
    setIntegerList(integerList, integerListLength);
  }

  const std::vector<int> &
  getIntegerList() const
  {
    return oid_;
  }

  void
  setIntegerList(const std::vector<int>& value)
  {
    oid_ = value;
  }

  void
  setIntegerList(const int* value, size_t valueLength);

  std::string
  toString() const;

  bool operator == (const OID& oid) const
  {
    return equal(oid);
  }

  bool operator != (const OID& oid) const
  {
    return !equal(oid);
  }

private:
  bool equal(const OID& oid) const;

  std::vector<int> oid_;
};

}

#endif
