
//  (C) Copyright Edward Diener 2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_FTCLASS_HPP)
#define NDNBOOST_TTI_DETAIL_FTCLASS_HPP

#include <ndnboost/function_types/parameter_types.hpp>
#include <ndnboost/mpl/at.hpp>
#include <ndnboost/mpl/identity.hpp>
#include <ndnboost/mpl/int.hpp>
#include <ndnboost/mpl/quote.hpp>

namespace ndnboost
  {
  namespace tti
    {
    namespace detail
      {
      template<class NDNBOOST_TTI_DETAIL_TP_F>
      struct class_type :
          ndnboost::mpl::at
            <
            typename
            ndnboost::function_types::parameter_types
              <
              NDNBOOST_TTI_DETAIL_TP_F,
              ndnboost::mpl::quote1
                <
                ndnboost::mpl::identity
                >
              >::type,
              ndnboost::mpl::int_<0>
            >
        {
        };
      }
    }
  }
  
#endif // NDNBOOST_TTI_DETAIL_FTCLASS_HPP
