/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019-2020 Regents of the University of California.
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

#ifndef NDN_PSYNC_MISSING_DATA_INFO_HPP
#define NDN_PSYNC_MISSING_DATA_INFO_HPP

#include "../name.hpp"

namespace ndn {

class PSyncMissingDataInfo
{
public:
  PSyncMissingDataInfo
    (const Name& prefix, int lowSequenceNo, int highSequenceNo)
  : prefix_(prefix), lowSequenceNo_(lowSequenceNo), highSequenceNo_(highSequenceNo)
  {}

  Name prefix_;
  int lowSequenceNo_;
  int highSequenceNo_;
};

}

#endif
