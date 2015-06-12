
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_PTMF_HPP)
#define NDNBOOST_TTI_DETAIL_PTMF_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/mpl/push_front.hpp>
#include <ndnboost/function_types/member_function_pointer.hpp>

namespace ndnboost
  {
  namespace tti
    {
    namespace detail
      {
      template
        <
        class NDNBOOST_TTI_DETAIL_TP_T,
        class NDNBOOST_TTI_DETAIL_TP_R,
        class NDNBOOST_TTI_DETAIL_TP_FS,
        class NDNBOOST_TTI_DETAIL_TP_TAG
        >
      struct ptmf_seq
        {
        typedef typename
        ndnboost::function_types::member_function_pointer
          <
          typename
          ndnboost::mpl::push_front
            <
            typename
            ndnboost::mpl::push_front<NDNBOOST_TTI_DETAIL_TP_FS,NDNBOOST_TTI_DETAIL_TP_T>::type,
            NDNBOOST_TTI_DETAIL_TP_R
            >::type,
          NDNBOOST_TTI_DETAIL_TP_TAG
          >::type type;
        };
      }
    }
  }
  
#endif // NDNBOOST_TTI_DETAIL_PTMF_HPP
