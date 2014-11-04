# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Edward Diener 2011.                                    *
#  *     (C) Copyright Paul Mensonides 2011.                                  *
#  *     Distributed under the Boost Software License, Version 1.0. (See      *
#  *     accompanying file LICENSE_1_0.txt or copy at                         *
#  *     http://www.boost.org/LICENSE_1_0.txt)                                *
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef NDNBOOST_PREPROCESSOR_TUPLE_SIZE_HPP
# define NDNBOOST_PREPROCESSOR_TUPLE_SIZE_HPP
#
# include <ndnboost/preprocessor/cat.hpp>
# include <ndnboost/preprocessor/config/config.hpp>
# include <ndnboost/preprocessor/variadic/size.hpp>
#
# if NDNBOOST_PP_VARIADICS
#    if NDNBOOST_PP_VARIADICS_MSVC
#        define NDNBOOST_PP_TUPLE_SIZE(tuple) NDNBOOST_PP_CAT(NDNBOOST_PP_VARIADIC_SIZE tuple,)
#    else
#        define NDNBOOST_PP_TUPLE_SIZE(tuple) NDNBOOST_PP_VARIADIC_SIZE tuple
#    endif
# endif
#
# endif
