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
# ifndef NDNBOOST_PREPROCESSOR_ARRAY_ENUM_HPP
# define NDNBOOST_PREPROCESSOR_ARRAY_ENUM_HPP
#
# include <ndnboost/preprocessor/cat.hpp>
# include <ndnboost/preprocessor/config/config.hpp>
# include <ndnboost/preprocessor/tuple/rem.hpp>
#
# /* NDNBOOST_PP_ARRAY_ENUM */
#
# if NDNBOOST_PP_CONFIG_FLAGS() & NDNBOOST_PP_CONFIG_MSVC()
#    define NDNBOOST_PP_ARRAY_ENUM(array) NDNBOOST_PP_ARRAY_ENUM_I(NDNBOOST_PP_TUPLE_REM_CTOR, array)
#    define NDNBOOST_PP_ARRAY_ENUM_I(m, args) NDNBOOST_PP_ARRAY_ENUM_II(m, args)
#    define NDNBOOST_PP_ARRAY_ENUM_II(m, args) NDNBOOST_PP_CAT(m ## args,)
# elif NDNBOOST_PP_CONFIG_FLAGS() & NDNBOOST_PP_CONFIG_MWCC()
#    define NDNBOOST_PP_ARRAY_ENUM(array) NDNBOOST_PP_ARRAY_ENUM_I(array)
#    define NDNBOOST_PP_ARRAY_ENUM_I(array) NDNBOOST_PP_TUPLE_REM_CTOR ## array
# else
#    define NDNBOOST_PP_ARRAY_ENUM(array) NDNBOOST_PP_TUPLE_REM_CTOR array
# endif
#
# endif
