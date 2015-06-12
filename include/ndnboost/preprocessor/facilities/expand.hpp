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
# ifndef NDNBOOST_PREPROCESSOR_FACILITIES_EXPAND_HPP
# define NDNBOOST_PREPROCESSOR_FACILITIES_EXPAND_HPP
#
# include <ndnboost/preprocessor/config/config.hpp>
#
# if ~NDNBOOST_PP_CONFIG_FLAGS() & NDNBOOST_PP_CONFIG_MWCC() && ~NDNBOOST_PP_CONFIG_FLAGS() & NDNBOOST_PP_CONFIG_DMC()
#    define NDNBOOST_PP_EXPAND(x) NDNBOOST_PP_EXPAND_I(x)
# else
#    define NDNBOOST_PP_EXPAND(x) NDNBOOST_PP_EXPAND_OO((x))
#    define NDNBOOST_PP_EXPAND_OO(par) NDNBOOST_PP_EXPAND_I ## par
# endif
#
# define NDNBOOST_PP_EXPAND_I(x) x
#
# endif
