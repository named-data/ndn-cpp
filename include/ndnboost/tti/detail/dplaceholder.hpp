
//  (C) Copyright Edward Diener 2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_PLACEHOLDER_HPP)
#define NDNBOOST_TTI_DETAIL_PLACEHOLDER_HPP

#include <ndnboost/mpl/lambda.hpp>
#include <ndnboost/mpl/not.hpp>
#include <ndnboost/type_traits/is_same.hpp>

namespace ndnboost
  {
  namespace tti
    {
    namespace detail
      {
      template <class NDNBOOST_TTI_DETAIL_TP_T>
      struct is_placeholder_expression :
        ndnboost::mpl::not_
          <
          ndnboost::is_same
            <
            typename ndnboost::mpl::lambda<NDNBOOST_TTI_DETAIL_TP_T>::type,
            NDNBOOST_TTI_DETAIL_TP_T
            >
          >
        {
        };
      }
    }
  }
  
#endif // NDNBOOST_TTI_DETAIL_PLACEHOLDER_HPP
