//  Copyright (C) Christof Meerwald 2003
//  Copyright (C) Dan Watkins 2003
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  Digital Mars C++ compiler setup:
#define NDNBOOST_COMPILER __DMC_VERSION_STRING__

#define NDNBOOST_HAS_LONG_LONG
#define NDNBOOST_HAS_PRAGMA_ONCE

#if !defined(NDNBOOST_STRICT_CONFIG)
#define NDNBOOST_NO_MEMBER_TEMPLATE_FRIENDS
#define NDNBOOST_NO_OPERATORS_IN_NAMESPACE
#define NDNBOOST_NO_UNREACHABLE_RETURN_DETECTION
#define NDNBOOST_NO_SFINAE
#define NDNBOOST_NO_USING_TEMPLATE
#define NDNBOOST_FUNCTION_SCOPE_USING_DECLARATION_BREAKS_ADL
#endif

//
// has macros:
#define NDNBOOST_HAS_DIRENT_H
#define NDNBOOST_HAS_STDINT_H
#define NDNBOOST_HAS_WINTHREADS

#if (__DMC__ >= 0x847)
#define NDNBOOST_HAS_EXPM1
#define NDNBOOST_HAS_LOG1P
#endif

//
// Is this really the best way to detect whether the std lib is in namespace std?
//
#ifdef __cplusplus
#include <cstddef>
#endif
#if !defined(__STL_IMPORT_VENDOR_CSTD) && !defined(_STLP_IMPORT_VENDOR_CSTD)
#  define NDNBOOST_NO_STDC_NAMESPACE
#endif


// check for exception handling support:
#if !defined(_CPPUNWIND) && !defined(NDNBOOST_NO_EXCEPTIONS)
#  define NDNBOOST_NO_EXCEPTIONS
#endif

//
// C++0x features
//
#define NDNBOOST_NO_CXX11_AUTO_DECLARATIONS
#define NDNBOOST_NO_CXX11_AUTO_MULTIDECLARATIONS
#define NDNBOOST_NO_CXX11_CHAR16_T
#define NDNBOOST_NO_CXX11_CHAR32_T
#define NDNBOOST_NO_CXX11_CONSTEXPR
#define NDNBOOST_NO_CXX11_DECLTYPE
#define NDNBOOST_NO_CXX11_DECLTYPE_N3276
#define NDNBOOST_NO_CXX11_DEFAULTED_FUNCTIONS
#define NDNBOOST_NO_CXX11_DELETED_FUNCTIONS
#define NDNBOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#define NDNBOOST_NO_CXX11_EXTERN_TEMPLATE
#define NDNBOOST_NO_CXX11_HDR_INITIALIZER_LIST
#define NDNBOOST_NO_CXX11_LAMBDAS
#define NDNBOOST_NO_CXX11_LOCAL_CLASS_TEMPLATE_PARAMETERS
#define NDNBOOST_NO_CXX11_NOEXCEPT
#define NDNBOOST_NO_CXX11_NULLPTR
#define NDNBOOST_NO_CXX11_RANGE_BASED_FOR
#define NDNBOOST_NO_CXX11_RAW_LITERALS
#define NDNBOOST_NO_CXX11_RVALUE_REFERENCES
#define NDNBOOST_NO_CXX11_SCOPED_ENUMS
#define NDNBOOST_NO_SFINAE_EXPR
#define NDNBOOST_NO_CXX11_STATIC_ASSERT
#define NDNBOOST_NO_CXX11_TEMPLATE_ALIASES
#define NDNBOOST_NO_CXX11_UNICODE_LITERALS
#define NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES
#define NDNBOOST_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
#define NDNBOOST_NO_CXX11_USER_DEFINED_LITERALS
#define NDNBOOST_NO_CXX11_ALIGNAS
#define NDNBOOST_NO_CXX11_TRAILING_RESULT_TYPES
#define NDNBOOST_NO_CXX11_INLINE_NAMESPACES
#define NDNBOOST_NO_CXX11_REF_QUALIFIERS

#if (__DMC__ <= 0x840)
#error "Compiler not supported or configured - please reconfigure"
#endif
//
// last known and checked version is ...:
#if (__DMC__ > 0x848)
#  if defined(NDNBOOST_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  endif
#endif
