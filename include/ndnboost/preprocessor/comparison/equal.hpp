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
# ifndef NDNBOOST_PREPROCESSOR_COMPARISON_EQUAL_HPP
# define NDNBOOST_PREPROCESSOR_COMPARISON_EQUAL_HPP
#
# include <ndnboost/preprocessor/comparison/not_equal.hpp>
# include <ndnboost/preprocessor/config/config.hpp>
# include <ndnboost/preprocessor/logical/compl.hpp>
#
# /* NDNBOOST_PP_EQUAL */
#
# if ~NDNBOOST_PP_CONFIG_FLAGS() & NDNBOOST_PP_CONFIG_EDG()
#    define NDNBOOST_PP_EQUAL(x, y) NDNBOOST_PP_COMPL(NDNBOOST_PP_NOT_EQUAL(x, y))
# else
#    define NDNBOOST_PP_EQUAL(x, y) NDNBOOST_PP_EQUAL_I(x, y)
#    define NDNBOOST_PP_EQUAL_I(x, y) NDNBOOST_PP_COMPL(NDNBOOST_PP_NOT_EQUAL(x, y))
# endif
#
# /* NDNBOOST_PP_EQUAL_D */
#
# define NDNBOOST_PP_EQUAL_D(d, x, y) NDNBOOST_PP_EQUAL(x, y)
#
# endif
