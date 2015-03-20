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

#ifndef NDN_COMMON_HPP
#define NDN_COMMON_HPP

#include <vector>
#include <string>
// common.h include ndn-cpp-config.h.
#include "c/common.h"

// Depending on where ./configure found shared_ptr and the options --with-std-shared-ptr
//   and --with-boost-shared-ptr, define the ptr_lib namespace.
// We always use ndn::ptr_lib.
#if NDN_CPP_HAVE_STD_SHARED_PTR && NDN_CPP_WITH_STD_SHARED_PTR
#include <memory>
namespace ndn { namespace ptr_lib = std; }
#elif NDN_CPP_HAVE_BOOST_SHARED_PTR && NDN_CPP_WITH_BOOST_SHARED_PTR
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
namespace ndn { namespace ptr_lib = boost; }
#else
/* Use the boost header files in this distribution that were extracted with:
cd <BOOST DEVELOPMENT DIRECTORY WITH boost SUBDIRECTORY>
dist/bin/bcp --namespace=ndnboost shared_ptr make_shared enable_shared_from_this weak_ptr function bind any iostreams <NDN-CPP ROOT>/include
cd <NDN-CPP ROOT>/include
rm -rf boost.css boost.png Jamroot libs
mv boost ndnboost
cd ndnboost
# Replace when including files.
(unset LANG; find . -type f -exec sed -i '' 's/\<boost\//\<ndnboost\//g' {} +)
(unset LANG; find . -type f -exec sed -i '' 's/\"boost\//\"ndnboost\//g' {} +)
(unset LANG; find . -type f -exec sed -i '' 's/ boost\// ndnboost\//g' {} +)
(unset LANG; find . -type f -exec sed -i '' 's/(boost\//(ndnboost\//g' {} +)
# Replace macro definitions.
(unset LANG; find . -type f -exec sed -i '' 's/BOOST_/NDNBOOST_/g' {} +)
# Replace header include guards which don't start with BOOST_ .  This may result in some with NDNBOOST twice, but that is OK.
(unset LANG; find . -type f -exec sed -i '' 's/_DWA/_NDNBOOST_DWA/g' {} +)
(unset LANG; find . -type f -exec sed -i '' 's/ UUID_/ NDNBOOST_UUID_/g' {} +)
(unset LANG; find . -type f -exec sed -i '' 's/ FILE_boost/ FILE_ndnboost/g' {} +)
# Replace the mpl_ barrier namespace.  This should only change file adl_barrier.hpp.
(unset LANG; find . -type f -exec sed -i '' 's/ mpl_/ ndnboost_mpl_/g' {} +)
 */
#include <ndnboost/shared_ptr.hpp>
#include <ndnboost/make_shared.hpp>
#include <ndnboost/enable_shared_from_this.hpp>
namespace ndn { namespace ptr_lib = ndnboost; }
#endif

// Depending on where ./configure found function and the options --with-std-function
//   and --with-boost-function, define the func_lib namespace.
// We always use ndn::func_lib.
#if NDN_CPP_HAVE_STD_FUNCTION && NDN_CPP_WITH_STD_FUNCTION
#include <functional>
// Define the func_lib namespace explicitly to pull in _1, _2, etc.
namespace ndn { namespace func_lib {
  using std::function;
  using std::mem_fn;
  using std::bind;
  using std::ref;

  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  using std::placeholders::_4;
  using std::placeholders::_5;
  using std::placeholders::_6;
  using std::placeholders::_7;
  using std::placeholders::_8;
  using std::placeholders::_9;

  // Define this namespace for backwards compatibility with code that pulls in _1, etc. with:
  // using namespace ndn::func_lib::placeholders;
  namespace placeholders {}
} }
#elif NDN_CPP_HAVE_BOOST_FUNCTION && NDN_CPP_WITH_BOOST_FUNCTION
#include <boost/function.hpp>
#include <boost/bind.hpp>
namespace ndn { namespace func_lib = boost; }
#else
// Use the boost header files in this distribution that were extracted as above:
#include <ndnboost/function.hpp>
#include <ndnboost/bind.hpp>
namespace ndn { namespace func_lib = ndnboost; }
#endif

namespace ndn {

/**
 * A time interval represented as the number of milliseconds.
 */
typedef double Milliseconds;

/**
 * The calendar time represented as the number of milliseconds since 1/1/1970.
 */
typedef double MillisecondsSince1970;

/**
 * Return the hex representation of the bytes in array.
 * @param array The array of bytes.
 * @param arrayLength The number of bytes in array.
 * @return The hex string.
 */
std::string
toHex(const uint8_t* array, size_t arrayLength);

/**
 * Return the hex representation of the bytes in array.
 * @param array The array of bytes.
 * @return The hex string.
 */
static __inline std::string
toHex(const std::vector<uint8_t>& array)
{
  return toHex(&array[0], array.size());
}

}

#endif
