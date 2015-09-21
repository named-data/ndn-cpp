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

#ifndef NDN_FORWARDING_FLAGS_HPP
#define NDN_FORWARDING_FLAGS_HPP

#include "c/forwarding-flags.h"

namespace ndn {

/**
 * A ForwardingFlags object holds the flags which specify how the forwarding daemon should forward an interest for
 * a registered prefix.  We use a separate ForwardingFlags object to retain future compatibility if the daemon forwarding
 * bits are changed, amended or deprecated.
 */
class ForwardingFlags : public ndn_ForwardingFlags {
public:
  /**
   * Create a new ForwardingFlags with "childInherit" set and all other flags cleared.
   */
  ForwardingFlags();

  ForwardingFlags(const struct ndn_ForwardingFlags &forwardingFlagsStruct)
  : ndn_ForwardingFlags(forwardingFlagsStruct)
  {
  }

  /**
   * Get the value of the "childInherit" flag.
   * @return true if the flag is set, false if it is cleared.
   */
  bool getChildInherit() const { return childInherit != 0; }

  /**
   * Get the value of the "capture" flag.
   * @return true if the flag is set, false if it is cleared.
   */
  bool getCapture() const { return capture != 0; }

  /**
   * Set the value of the "childInherit" flag
   * @param childInherit true to set the flag, false to clear it.
   */
  void setChildInherit(bool childInherit) { this->childInherit = childInherit ? 1 : 0; }

  /**
   * Set the value of the "capture" flag
   * @param capture true to set the flag, false to clear it.
   */
  void setCapture(bool capture) { this->capture = capture ? 1 : 0; }
};

}

#endif
