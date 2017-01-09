/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2017 Regents of the University of California.
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

#include <stdexcept>
#include <ndn-cpp/common.hpp>
#include <ndn-cpp/exclude.hpp>

using namespace std;

namespace ndn {

void
Exclude::get(ExcludeLite& excludeLite) const
{
  excludeLite.clear();
  for (size_t i = 0; i < entries_.size(); ++i) {
    ndn_Error error;
    if (entries_[i].getType() == ndn_Exclude_COMPONENT) {
      NameLite::Component component;
      entries_[i].getComponent().get(component);
      if ((error = excludeLite.appendComponent(component)))
        throw runtime_error(ndn_getErrorString(error));
    }
    else {
      if ((error = excludeLite.appendAny()))
        throw runtime_error(ndn_getErrorString(error));
    }
  }
}

void
Exclude::set(const ExcludeLite& excludeLite)
{
  clear();
  for (size_t i = 0; i < excludeLite.size(); ++i) {
    const ExcludeLite::Entry& entry = excludeLite.get(i);

    if (entry.getType() == ndn_Exclude_COMPONENT)
      appendComponent(Name::Component(entry.getComponent()));
    else if (entry.getType() == ndn_Exclude_ANY)
      appendAny();
    else
      throw runtime_error("unrecognized ndn_ExcludeType");
  }
}

bool
Exclude::matches(const Name::Component& component) const
{
  size_t i;
  for (i = 0; i < entries_.size(); ++i) {
    if (entries_[i].getType() == ndn_Exclude_ANY) {
      const Entry* lowerBound = 0;
      if (i > 0)
        lowerBound = &entries_[i - 1];

      // Find the upper bound, possibly skipping over multiple ANY in a row.
      size_t iUpperBound;
      const Entry* upperBound = 0;
      for (iUpperBound = i + 1; iUpperBound < entries_.size(); ++iUpperBound) {
        if (entries_[iUpperBound].getType() == ndn_Exclude_COMPONENT) {
          upperBound = &entries_[iUpperBound];
          break;
        }
      }

      // If lowerBound != 0, we already checked component equals lowerBound on the last pass.
      // If upperBound != 0, we will check component equals upperBound on the next pass.
      if (upperBound != 0) {
        if (lowerBound != 0) {
          if (component > lowerBound->getComponent() &&
              component < upperBound->getComponent())
            return true;
        }
        else {
          if (component < upperBound->getComponent())
            return true;
        }

        // Make i equal iUpperBound on the next pass.
        i = iUpperBound - 1;
      }
      else {
        if (lowerBound != 0) {
          if (component > lowerBound->getComponent())
            return true;
        }
        else
          // entries_ has only ANY.
          return true;
      }
    }
    else {
      if (component == entries_[i].getComponent())
        return true;
    }
  }

  return false;
}

string
Exclude::toUri() const
{
  if (entries_.size() == 0)
    return "";

  ostringstream result;
  for (unsigned i = 0; i < entries_.size(); ++i) {
    if (i > 0)
      result << ",";

    if (entries_[i].getType() == ndn_Exclude_ANY)
      result << "*";
    else
      entries_[i].getComponent().toEscapedString(result);
  }

  return result.str();
}

}
