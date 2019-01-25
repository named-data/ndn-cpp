/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/tpm/helper-osx.hpp
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

#ifndef NDN_HELPER_OSX_HPP
#define NDN_HELPER_OSX_HPP

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_OSX_SECURITY 1.
#include "../../ndn-cpp-config.h"
#if NDN_CPP_HAVE_OSX_SECURITY

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <CoreServices/CoreServices.h>

namespace ndn {

/**
 * CFReleaser is a helper class to wrap CoreFoundation object pointers.
 *
 * The class is similar in spirit to shared_ptr, but uses CoreFoundation
 * mechanisms to retain/release object.
 *
 * Original implementation by Christopher Hunt and it was borrowed from
 * http://www.cocoabuilder.com/archive/cocoa/130776-auto-cfrelease-and.html
 */
template<class T>
class CFReleaser
{
public:
  //////////////////////////////
  // Construction/destruction //

  CFReleaser()
    : m_typeRef(0)
  {
  }

  CFReleaser(const T& typeRef)
    : m_typeRef(typeRef)
  {
  }

  CFReleaser(const CFReleaser& inReleaser)
    : m_typeRef(0)
  {
    retain(inReleaser.m_typeRef);
  }

  CFReleaser&
  operator=(const T& typeRef)
  {
    if (typeRef != m_typeRef) {
      release();
      m_typeRef = typeRef;
    }
    return *this;
  }

  CFReleaser&
  operator=(const CFReleaser& inReleaser)
  {
    retain(inReleaser.m_typeRef);
    return *this;
  }

  ~CFReleaser()
  {
    release();
  }

  ////////////
  // Access //

  // operator const T&() const
  // {
  //   return m_typeRef;
  // }

  // operator T&()
  // {
  //   return m_typeRef;
  // }

  const T&
  get() const
  {
    return m_typeRef;
  }

  T&
  get()
  {
    return m_typeRef;
  }

  ///////////////////
  // Miscellaneous //

  void
  retain(const T& typeRef)
  {
    if (typeRef != 0) {
      CFRetain(typeRef);
    }
    release();
    m_typeRef = typeRef;
  }

  void
  retain()
  {
    T typeRef = m_typeRef;
    m_typeRef = nullptr;
    retain(typeRef);
  }

  void release()
  {
    if (m_typeRef != 0) {
      CFRelease(m_typeRef);
      m_typeRef = 0;
    }
  };

private:
  T m_typeRef;
};

typedef CFReleaser<SecKeyRef> KeyRefOsx;

}

#endif // NDN_CPP_HAVE_OSX_SECURITY

#endif
