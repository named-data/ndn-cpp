# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2002.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# ifndef NDNBOOST_PREPROCESSOR_DETAIL_SPLIT_HPP
# define NDNBOOST_PREPROCESSOR_DETAIL_SPLIT_HPP
#
# include <ndnboost/preprocessor/config/config.hpp>
#
# /* NDNBOOST_PP_SPLIT */
#
# if NDNBOOST_PP_CONFIG_FLAGS() & NDNBOOST_PP_CONFIG_MWCC()
#    define NDNBOOST_PP_SPLIT(n, im) NDNBOOST_PP_SPLIT_I((n, im))
#    define NDNBOOST_PP_SPLIT_I(par) NDNBOOST_PP_SPLIT_II ## par
#    define NDNBOOST_PP_SPLIT_II(n, a, b) NDNBOOST_PP_SPLIT_ ## n(a, b)
# elif NDNBOOST_PP_CONFIG_FLAGS() & NDNBOOST_PP_CONFIG_MSVC()
#    define NDNBOOST_PP_SPLIT(n, im) NDNBOOST_PP_SPLIT_I(n((im)))
#    define NDNBOOST_PP_SPLIT_I(n) NDNBOOST_PP_SPLIT_ID(NDNBOOST_PP_SPLIT_II_ ## n)
#    define NDNBOOST_PP_SPLIT_II_0(s) NDNBOOST_PP_SPLIT_ID(NDNBOOST_PP_SPLIT_0 s)
#    define NDNBOOST_PP_SPLIT_II_1(s) NDNBOOST_PP_SPLIT_ID(NDNBOOST_PP_SPLIT_1 s)
#    define NDNBOOST_PP_SPLIT_ID(id) id
# else
#    define NDNBOOST_PP_SPLIT(n, im) NDNBOOST_PP_SPLIT_I(n)(im)
#    define NDNBOOST_PP_SPLIT_I(n) NDNBOOST_PP_SPLIT_ ## n
# endif
#
# define NDNBOOST_PP_SPLIT_0(a, b) a
# define NDNBOOST_PP_SPLIT_1(a, b) b
#
# endif
