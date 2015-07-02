# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2003.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef NDNBOOST_PREPROCESSOR_FACILITIES_IS_EMPTY_HPP
# define NDNBOOST_PREPROCESSOR_FACILITIES_IS_EMPTY_HPP
#
# include <ndnboost/preprocessor/config/config.hpp>
# include <ndnboost/preprocessor/cat.hpp>
# include <ndnboost/preprocessor/tuple/elem.hpp>
# include <ndnboost/preprocessor/facilities/empty.hpp>
# include <ndnboost/preprocessor/detail/split.hpp>
#
# /* NDNBOOST_PP_IS_EMPTY */
#
# if ~NDNBOOST_PP_CONFIG_FLAGS() & NDNBOOST_PP_CONFIG_MSVC() && ~NDNBOOST_PP_CONFIG_FLAGS() & NDNBOOST_PP_CONFIG_MWCC()
#    define NDNBOOST_PP_IS_EMPTY(x) NDNBOOST_PP_IS_EMPTY_I(x NDNBOOST_PP_IS_EMPTY_HELPER)
#    define NDNBOOST_PP_IS_EMPTY_I(contents) NDNBOOST_PP_TUPLE_ELEM(2, 1, (NDNBOOST_PP_IS_EMPTY_DEF_ ## contents()))
#    define NDNBOOST_PP_IS_EMPTY_DEF_NDNBOOST_PP_IS_EMPTY_HELPER 1, 1 NDNBOOST_PP_EMPTY
#    define NDNBOOST_PP_IS_EMPTY_HELPER() , 0
# else
#    if NDNBOOST_PP_CONFIG_FLAGS() & NDNBOOST_PP_CONFIG_MSVC()
#        define NDNBOOST_PP_IS_EMPTY(x) NDNBOOST_PP_IS_EMPTY_I(NDNBOOST_PP_IS_EMPTY_HELPER x ())
#        define NDNBOOST_PP_IS_EMPTY_I(test) NDNBOOST_PP_IS_EMPTY_II(NDNBOOST_PP_SPLIT(0, NDNBOOST_PP_CAT(NDNBOOST_PP_IS_EMPTY_DEF_, test)))
#        define NDNBOOST_PP_IS_EMPTY_II(id) id
#    else
#        define NDNBOOST_PP_IS_EMPTY(x) NDNBOOST_PP_IS_EMPTY_I((NDNBOOST_PP_IS_EMPTY_HELPER x ()))
#        define NDNBOOST_PP_IS_EMPTY_I(par) NDNBOOST_PP_IS_EMPTY_II ## par
#        define NDNBOOST_PP_IS_EMPTY_II(test) NDNBOOST_PP_SPLIT(0, NDNBOOST_PP_CAT(NDNBOOST_PP_IS_EMPTY_DEF_, test))
#    endif
#    define NDNBOOST_PP_IS_EMPTY_HELPER() 1
#    define NDNBOOST_PP_IS_EMPTY_DEF_1 1, NDNBOOST_PP_NIL
#    define NDNBOOST_PP_IS_EMPTY_DEF_NDNBOOST_PP_IS_EMPTY_HELPER 0, NDNBOOST_PP_NIL
# endif
#
# endif
