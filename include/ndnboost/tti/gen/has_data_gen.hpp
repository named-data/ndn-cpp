
//  (C) Copyright Edward Diener 2012
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DATA_GEN_HPP)
#define NDNBOOST_TTI_DATA_GEN_HPP

#include <ndnboost/preprocessor/cat.hpp>

/*

  The succeeding comments in this file are in doxygen format.

*/

/** \file
*/

/// Generates the macro metafunction name for NDNBOOST_TTI_HAS_DATA.
/**
    name  = the name of the member data.

    returns = the generated macro metafunction name.
*/
#define NDNBOOST_TTI_HAS_DATA_GEN(name) \
  NDNBOOST_PP_CAT(has_data_,name) \
/**/

#endif // NDNBOOST_TTI_DATA_GEN_HPP
