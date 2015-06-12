//  GetLastError.hpp  --------------------------------------------------------------//

//  Copyright 2010 Vicente J. Botet Escriba

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt


#ifndef NDNBOOST_DETAIL_WINAPI_GETLASTERROR_HPP
#define NDNBOOST_DETAIL_WINAPI_GETLASTERROR_HPP

#include <ndnboost/detail/winapi/basic_types.hpp>

#ifdef NDNBOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace ndnboost {
namespace detail {
namespace winapi {
#if defined( NDNBOOST_USE_WINDOWS_H )
    using ::GetLastError;
#else
    extern "C" __declspec(dllimport) DWORD_ WINAPI
        GetLastError();
#endif
}
}
}

#endif // NDNBOOST_DETAIL_WINAPI_GETLASTERROR_HPP
