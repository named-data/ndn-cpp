//  (C) Copyright 2008-10 Anthony Williams
//  (C) Copyright 2011-2012 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef NDNBOOST_THREAD_FUTURE_ERROR_CODE_HPP
#define NDNBOOST_THREAD_FUTURE_ERROR_CODE_HPP

#include <ndnboost/thread/detail/config.hpp>
#include <ndnboost/core/scoped_enum.hpp>
#include <ndnboost/system/error_code.hpp>
#include <ndnboost/type_traits/integral_constant.hpp>

namespace ndnboost
{

  //enum class future_errc
  NDNBOOST_SCOPED_ENUM_DECLARE_BEGIN(future_errc)
  {
      broken_promise = 1,
      future_already_retrieved,
      promise_already_satisfied,
      no_state
  }
  NDNBOOST_SCOPED_ENUM_DECLARE_END(future_errc)

  namespace system
  {
    template <>
    struct NDNBOOST_SYMBOL_VISIBLE is_error_code_enum< ::ndnboost::future_errc> : public true_type {};

    #ifdef NDNBOOST_NO_CXX11_SCOPED_ENUMS
    template <>
    struct NDNBOOST_SYMBOL_VISIBLE is_error_code_enum< ::ndnboost::future_errc::enum_type> : public true_type { };
    #endif
  } // system

  NDNBOOST_THREAD_DECL
  const system::error_category& future_category() NDNBOOST_NOEXCEPT;

  namespace system
  {
    inline
    error_code
    make_error_code(future_errc e) NDNBOOST_NOEXCEPT
    {
        return error_code(underlying_cast<int>(e), ndnboost::future_category());
    }

    inline
    error_condition
    make_error_condition(future_errc e) NDNBOOST_NOEXCEPT
    {
        return error_condition(underlying_cast<int>(e), ndnboost::future_category());
    }
  } // system
} // boost

#endif // header
