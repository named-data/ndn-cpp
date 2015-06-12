
//  (C) Copyright Edward Diener 2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_LAMBDA_HPP)
#define NDNBOOST_TTI_DETAIL_LAMBDA_HPP

#include <ndnboost/mpl/or.hpp>
#include <ndnboost/tti/detail/dmetafunc.hpp>
#include <ndnboost/tti/detail/dplaceholder.hpp>
#include <ndnboost/tti/gen/namespace_gen.hpp>

namespace ndnboost
  {
  namespace tti
    {
    namespace detail
      {
      template <class NDNBOOST_TTI_DETAIL_TP_T>
      struct is_lambda_expression :
        ndnboost::mpl::or_
          <
          NDNBOOST_TTI_NAMESPACE::detail::is_metafunction_class<NDNBOOST_TTI_DETAIL_TP_T>,
          NDNBOOST_TTI_NAMESPACE::detail::is_placeholder_expression<NDNBOOST_TTI_DETAIL_TP_T>
          >
        {
        };
      }
    }
  }
  
#endif // NDNBOOST_TTI_DETAIL_LAMBDA_HPP
