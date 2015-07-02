//  cv_status.hpp
//
// Copyright (C) 2011 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef NDNBOOST_THREAD_CV_STATUS_HPP
#define NDNBOOST_THREAD_CV_STATUS_HPP

#include <ndnboost/core/scoped_enum.hpp>

namespace ndnboost
{

  // enum class cv_status;
  NDNBOOST_SCOPED_ENUM_DECLARE_BEGIN(cv_status)
  {
    no_timeout,
    timeout
  }
  NDNBOOST_SCOPED_ENUM_DECLARE_END(cv_status)
}

#endif // header
