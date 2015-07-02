
//  (C) Copyright Edward Diener 2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_METAFUNC_HPP)
#define NDNBOOST_TTI_DETAIL_METAFUNC_HPP

#include <ndnboost/mpl/has_xxx.hpp>
#include <ndnboost/tti/gen/namespace_gen.hpp>

namespace ndnboost
  {
  namespace tti
    {
    namespace detail
      {
      NDNBOOST_MPL_HAS_XXX_TEMPLATE_NAMED_DEF(is_metafunction_class_apply, apply, false)
      template <class NDNBOOST_TTI_DETAIL_TP_T>
      struct is_metafunction_class :
        NDNBOOST_TTI_NAMESPACE::detail::is_metafunction_class_apply<NDNBOOST_TTI_DETAIL_TP_T>
        {
        };
      }
    }
  }
  
#endif // NDNBOOST_TTI_DETAIL_METAFUNC_HPP
