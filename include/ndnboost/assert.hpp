//
//  ndnboost/assert.hpp - NDNBOOST_ASSERT(expr)
//                     NDNBOOST_ASSERT_MSG(expr, msg)
//                     NDNBOOST_VERIFY(expr)
//                     NDNBOOST_VERIFY_MSG(expr, msg)
//
//  Copyright (c) 2001, 2002 Peter Dimov and Multi Media Ltd.
//  Copyright (c) 2007, 2014 Peter Dimov
//  Copyright (c) Beman Dawes 2011
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//
//  Note: There are no include guards. This is intentional.
//
//  See http://www.boost.org/libs/assert/assert.html for documentation.
//

//
// Stop inspect complaining about use of 'assert':
//
// boostinspect:naassert_macro
//

//
// NDNBOOST_ASSERT, NDNBOOST_ASSERT_MSG
//

#undef NDNBOOST_ASSERT
#undef NDNBOOST_ASSERT_MSG

#if defined(NDNBOOST_DISABLE_ASSERTS) || ( defined(NDNBOOST_ENABLE_ASSERT_DEBUG_HANDLER) && defined(NDEBUG) )

# define NDNBOOST_ASSERT(expr) ((void)0)
# define NDNBOOST_ASSERT_MSG(expr, msg) ((void)0)

#elif defined(NDNBOOST_ENABLE_ASSERT_HANDLER) || ( defined(NDNBOOST_ENABLE_ASSERT_DEBUG_HANDLER) && !defined(NDEBUG) )

#include <ndnboost/config.hpp> // for NDNBOOST_LIKELY
#include <ndnboost/current_function.hpp>

namespace ndnboost
{
    void assertion_failed(char const * expr, char const * function, char const * file, long line); // user defined
    void assertion_failed_msg(char const * expr, char const * msg, char const * function, char const * file, long line); // user defined
} // namespace ndnboost

#define NDNBOOST_ASSERT(expr) (NDNBOOST_LIKELY(!!(expr))? ((void)0): ::ndnboost::assertion_failed(#expr, NDNBOOST_CURRENT_FUNCTION, __FILE__, __LINE__))
#define NDNBOOST_ASSERT_MSG(expr, msg) (NDNBOOST_LIKELY(!!(expr))? ((void)0): ::ndnboost::assertion_failed_msg(#expr, msg, NDNBOOST_CURRENT_FUNCTION, __FILE__, __LINE__))

#else

# include <assert.h> // .h to support old libraries w/o <cassert> - effect is the same

# define NDNBOOST_ASSERT(expr) assert(expr)
# define NDNBOOST_ASSERT_MSG(expr, msg) assert((expr)&&(msg))

#endif

//
// NDNBOOST_VERIFY, NDNBOOST_VERIFY_MSG
//

#undef NDNBOOST_VERIFY
#undef NDNBOOST_VERIFY_MSG

#if defined(NDNBOOST_DISABLE_ASSERTS) || ( !defined(NDNBOOST_ENABLE_ASSERT_HANDLER) && defined(NDEBUG) )

# define NDNBOOST_VERIFY(expr) ((void)(expr))
# define NDNBOOST_VERIFY_MSG(expr, msg) ((void)(expr))

#else

# define NDNBOOST_VERIFY(expr) NDNBOOST_ASSERT(expr)
# define NDNBOOST_VERIFY_MSG(expr, msg) NDNBOOST_ASSERT_MSG(expr,msg)

#endif
