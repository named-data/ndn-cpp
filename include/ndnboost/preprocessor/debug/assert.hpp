# /* Copyright (C) 2001
#  * Housemarque Oy
#  * http://www.housemarque.com
#  *
#  * Distributed under the Boost Software License, Version 1.0. (See
#  * accompanying file LICENSE_1_0.txt or copy at
#  * http://www.boost.org/LICENSE_1_0.txt)
#  */
#
# /* Revised by Paul Mensonides (2002) */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef NDNBOOST_PREPROCESSOR_DEBUG_ASSERT_HPP
# define NDNBOOST_PREPROCESSOR_DEBUG_ASSERT_HPP
#
# include <ndnboost/preprocessor/config/config.hpp>
# include <ndnboost/preprocessor/control/expr_iif.hpp>
# include <ndnboost/preprocessor/control/iif.hpp>
# include <ndnboost/preprocessor/logical/not.hpp>
# include <ndnboost/preprocessor/tuple/eat.hpp>
#
# /* NDNBOOST_PP_ASSERT */
#
# if ~NDNBOOST_PP_CONFIG_FLAGS() & NDNBOOST_PP_CONFIG_EDG()
#    define NDNBOOST_PP_ASSERT NDNBOOST_PP_ASSERT_D
# else
#    define NDNBOOST_PP_ASSERT(cond) NDNBOOST_PP_ASSERT_D(cond)
# endif
#
# define NDNBOOST_PP_ASSERT_D(cond) NDNBOOST_PP_IIF(NDNBOOST_PP_NOT(cond), NDNBOOST_PP_ASSERT_ERROR, NDNBOOST_PP_TUPLE_EAT_1)(...)
# define NDNBOOST_PP_ASSERT_ERROR(x, y, z)
#
# /* NDNBOOST_PP_ASSERT_MSG */
#
# if ~NDNBOOST_PP_CONFIG_FLAGS() & NDNBOOST_PP_CONFIG_EDG()
#    define NDNBOOST_PP_ASSERT_MSG NDNBOOST_PP_ASSERT_MSG_D
# else
#    define NDNBOOST_PP_ASSERT_MSG(cond, msg) NDNBOOST_PP_ASSERT_MSG_D(cond, msg)
# endif
#
# define NDNBOOST_PP_ASSERT_MSG_D(cond, msg) NDNBOOST_PP_EXPR_IIF(NDNBOOST_PP_NOT(cond), msg)
#
# endif
