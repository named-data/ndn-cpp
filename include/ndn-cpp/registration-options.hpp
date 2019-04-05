/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019 Regents of the University of California.
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

#ifndef NDN_REGISTRATION_OPTIONS_HPP
#define NDN_REGISTRATION_OPTIONS_HPP

#include "lite/registration-options-lite.hpp"

namespace ndn {

/**
 * A RegistrationOptions holds the options used when registering with the
 * forwarder to specify how to forward an interest and other options. We use a
 * separate RegistrationOptions object to retain future compatibility if the
 * format of the registration command is changed.
 * (This class was renamed from ForwardingFlags, which is deprecated.)
 */
class RegistrationOptions {
public:
  /**
   * Create a new RegistrationOptions with "childInherit" set and all other
   * flags cleared.
   */
  RegistrationOptions() {}

  /**
   * Get the value of the "childInherit" flag.
   * @return true if the flag is set, false if it is cleared.
   */
  bool
  getChildInherit() const { return flags_.getChildInherit(); }

  /**
   * Get the value of the "capture" flag.
   * @return true if the flag is set, false if it is cleared.
   */
  bool
  getCapture() const { return flags_.getCapture(); }

  /**
   * Get the origin value.
   * @return The origin value, or -1 if not specified.
   */
  int
  getOrigin() const { return flags_.getOrigin(); }

  /**
   * Set the value of the "childInherit" flag
   * @param childInherit true to set the flag, false to clear it.
   * @return This RegistrationOptions so that you can chain calls to update values.
   */
  RegistrationOptions&
  setChildInherit(bool childInherit)
  {
    flags_.setChildInherit(childInherit);
    return *this;
  }

  /**
   * Set the value of the "capture" flag
   * @param capture true to set the flag, false to clear it.
   * @return This RegistrationOptions so that you can chain calls to update values.
   */
  RegistrationOptions&
  setCapture(bool capture)
  {
    flags_.setCapture(capture);
    return *this;
  }

  /**
   * Set the origin value.
   * @param origin The new origin value, or -1 for not specified.
   * @return This RegistrationOptions so that you can chain calls to update values.
   */
  RegistrationOptions&
  setOrigin(int origin)
  {
    flags_.setOrigin(origin);
    return *this;
  }

  /**
   * Set registrationOptionsLite to have the values in this meta info object.
   * @param registrationOptionsLite The RegistrationOptionsLite object which receives
   * the values.
   */
  void
  get(RegistrationOptionsLite& registrationOptionsLite) const
  {
    registrationOptionsLite = flags_;
  }

  /**
   * Set the values in this RegistrationOptions by copying from registrationOptionsLite.
   * @param registrationOptionsLite A RegistrationOptionsLite object.
   */
  void
  set(const RegistrationOptionsLite& registrationOptionsLite)
  {
    flags_ = registrationOptionsLite;
  }

protected:
  RegistrationOptionsLite flags_;
};


}

#endif
