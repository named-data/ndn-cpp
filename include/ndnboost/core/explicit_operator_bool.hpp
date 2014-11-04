/*
 *          Copyright Andrey Semashev 2007 - 2013.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

/*!
 * \file   explicit_operator_bool.hpp
 * \author Andrey Semashev
 * \date   08.03.2009
 *
 * This header defines a compatibility macro that implements an unspecified
 * \c bool operator idiom, which is superseded with explicit conversion operators in
 * C++11.
 */

#ifndef NDNBOOST_CORE_EXPLICIT_OPERATOR_BOOL_HPP
#define NDNBOOST_CORE_EXPLICIT_OPERATOR_BOOL_HPP

#include <ndnboost/config.hpp>

#ifdef NDNBOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if !defined(NDNBOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)

/*!
 * \brief The macro defines an explicit operator of conversion to \c bool
 *
 * The macro should be used inside the definition of a class that has to
 * support the conversion. The class should also implement <tt>operator!</tt>,
 * in terms of which the conversion operator will be implemented.
 */
#define NDNBOOST_EXPLICIT_OPERATOR_BOOL()\
    NDNBOOST_FORCEINLINE explicit operator bool () const\
    {\
        return !this->operator! ();\
    }

/*!
 * \brief The macro defines a noexcept explicit operator of conversion to \c bool
 *
 * The macro should be used inside the definition of a class that has to
 * support the conversion. The class should also implement <tt>operator!</tt>,
 * in terms of which the conversion operator will be implemented.
 */
#define NDNBOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()\
    NDNBOOST_FORCEINLINE explicit operator bool () const NDNBOOST_NOEXCEPT\
    {\
        return !this->operator! ();\
    }

/*!
 * \brief The macro defines a constexpr explicit operator of conversion to \c bool
 *
 * The macro should be used inside the definition of a class that has to
 * support the conversion. The class should also implement <tt>operator!</tt>,
 * in terms of which the conversion operator will be implemented.
 */
#define NDNBOOST_CONSTEXPR_EXPLICIT_OPERATOR_BOOL()\
    NDNBOOST_FORCEINLINE NDNBOOST_CONSTEXPR explicit operator bool () const NDNBOOST_NOEXCEPT\
    {\
        return !this->operator! ();\
    }

#else // !defined(NDNBOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)

#if (defined(__SUNPRO_CC) && (__SUNPRO_CC <= 0x530)) && !defined(NDNBOOST_NO_COMPILER_CONFIG)
// Sun C++ 5.3 can't handle the safe_bool idiom, so don't use it
#define NDNBOOST_NO_UNSPECIFIED_BOOL
#endif // (defined(__SUNPRO_CC) && (__SUNPRO_CC <= 0x530)) && !defined(NDNBOOST_NO_COMPILER_CONFIG)

#if !defined(NDNBOOST_NO_UNSPECIFIED_BOOL)

namespace ndnboost {

namespace detail {

#if !defined(_MSC_VER) && !defined(__IBMCPP__)

    struct unspecified_bool
    {
        // NOTE TO THE USER: If you see this in error messages then you tried
        // to apply an unsupported operator on the object that supports
        // explicit conversion to bool.
        struct OPERATORS_NOT_ALLOWED;
        static void true_value(OPERATORS_NOT_ALLOWED*) {}
    };
    typedef void (*unspecified_bool_type)(unspecified_bool::OPERATORS_NOT_ALLOWED*);

#else

    // MSVC and VACPP are too eager to convert pointer to function to void* even though they shouldn't
    struct unspecified_bool
    {
        // NOTE TO THE USER: If you see this in error messages then you tried
        // to apply an unsupported operator on the object that supports
        // explicit conversion to bool.
        struct OPERATORS_NOT_ALLOWED;
        void true_value(OPERATORS_NOT_ALLOWED*) {}
    };
    typedef void (unspecified_bool::*unspecified_bool_type)(unspecified_bool::OPERATORS_NOT_ALLOWED*);

#endif

} // namespace detail

} // namespace ndnboost

#define NDNBOOST_EXPLICIT_OPERATOR_BOOL()\
    NDNBOOST_FORCEINLINE operator ndnboost::detail::unspecified_bool_type () const\
    {\
        return (!this->operator! () ? &ndnboost::detail::unspecified_bool::true_value : (ndnboost::detail::unspecified_bool_type)0);\
    }

#define NDNBOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()\
    NDNBOOST_FORCEINLINE operator ndnboost::detail::unspecified_bool_type () const NDNBOOST_NOEXCEPT\
    {\
        return (!this->operator! () ? &ndnboost::detail::unspecified_bool::true_value : (ndnboost::detail::unspecified_bool_type)0);\
    }

#define NDNBOOST_CONSTEXPR_EXPLICIT_OPERATOR_BOOL()\
    NDNBOOST_FORCEINLINE NDNBOOST_CONSTEXPR operator ndnboost::detail::unspecified_bool_type () const NDNBOOST_NOEXCEPT\
    {\
        return (!this->operator! () ? &ndnboost::detail::unspecified_bool::true_value : (ndnboost::detail::unspecified_bool_type)0);\
    }

#else // !defined(NDNBOOST_NO_UNSPECIFIED_BOOL)

#define NDNBOOST_EXPLICIT_OPERATOR_BOOL()\
    NDNBOOST_FORCEINLINE operator bool () const\
    {\
        return !this->operator! ();\
    }

#define NDNBOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()\
    NDNBOOST_FORCEINLINE operator bool () const NDNBOOST_NOEXCEPT\
    {\
        return !this->operator! ();\
    }

#define NDNBOOST_CONSTEXPR_EXPLICIT_OPERATOR_BOOL()\
    NDNBOOST_FORCEINLINE NDNBOOST_CONSTEXPR operator bool () const NDNBOOST_NOEXCEPT\
    {\
        return !this->operator! ();\
    }

#endif // !defined(NDNBOOST_NO_UNSPECIFIED_BOOL)

#endif // !defined(NDNBOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)

#endif // NDNBOOST_CORE_EXPLICIT_OPERATOR_BOOL_HPP
