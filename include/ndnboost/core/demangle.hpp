#ifndef NDNBOOST_CORE_DEMANGLE_HPP_INCLUDED
#define NDNBOOST_CORE_DEMANGLE_HPP_INCLUDED

// core::demangle
//
// Copyright 2014 Peter Dimov
// Copyright 2014 Andrey Semashev
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#include <ndnboost/config.hpp>
#include <string>

#if defined(NDNBOOST_HAS_PRAGMA_ONCE)
# pragma once
#endif

#if defined( __clang__ ) && defined( __has_include )
# if __has_include(<cxxabi.h>)
#  define NDNBOOST_CORE_HAS_CXXABI_H
# endif
#elif defined( __GLIBCXX__ ) || defined( __GLIBCPP__ )
# define NDNBOOST_CORE_HAS_CXXABI_H
#endif

#if defined( NDNBOOST_CORE_HAS_CXXABI_H )
# include <cxxabi.h>
# include <cstdlib>
# include <cstddef>
#endif

namespace ndnboost
{

namespace core
{

inline char const * demangle_alloc( char const * name ) NDNBOOST_NOEXCEPT;
inline void demangle_free( char const * name ) NDNBOOST_NOEXCEPT;

class scoped_demangled_name
{
private:
    char const * m_p;

public:
    explicit scoped_demangled_name( char const * name ) NDNBOOST_NOEXCEPT :
        m_p( demangle_alloc( name ) )
    {
    }

    ~scoped_demangled_name() NDNBOOST_NOEXCEPT
    {
        demangle_free( m_p );
    }

    char const * get() const NDNBOOST_NOEXCEPT
    {
        return m_p;
    }

    NDNBOOST_DELETED_FUNCTION(scoped_demangled_name( scoped_demangled_name const& ))
    NDNBOOST_DELETED_FUNCTION(scoped_demangled_name& operator= ( scoped_demangled_name const& ))
};


#if defined( NDNBOOST_CORE_HAS_CXXABI_H )

inline char const * demangle_alloc( char const * name ) NDNBOOST_NOEXCEPT
{
    int status = 0;
    std::size_t size = 0;
    return abi::__cxa_demangle( name, NULL, &size, &status );
}

inline void demangle_free( char const * name ) NDNBOOST_NOEXCEPT
{
    std::free( const_cast< char* >( name ) );
}

inline std::string demangle( char const * name )
{
    scoped_demangled_name demangled_name( name );
    char const * const p = demangled_name.get();
    if( p )
    {
        return p;
    }
    else
    {
        return name;
    }
}

#else

inline char const * demangle_alloc( char const * name ) NDNBOOST_NOEXCEPT
{
    return name;
}

inline void demangle_free( char const * ) NDNBOOST_NOEXCEPT
{
}

inline std::string demangle( char const * name )
{
    return name;
}

#endif

} // namespace core

} // namespace ndnboost

#undef NDNBOOST_CORE_HAS_CXXABI_H

#endif // #ifndef NDNBOOST_CORE_DEMANGLE_HPP_INCLUDED
