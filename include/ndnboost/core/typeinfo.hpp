#ifndef NDNBOOST_CORE_TYPEINFO_HPP_INCLUDED
#define NDNBOOST_CORE_TYPEINFO_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//  core::typeinfo, NDNBOOST_CORE_TYPEID
//
//  Copyright 2007, 2014 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <ndnboost/config.hpp>

#if defined( NDNBOOST_NO_TYPEID )

#include <ndnboost/current_function.hpp>
#include <functional>

namespace ndnboost
{

namespace core
{

class typeinfo
{
private:

    typeinfo( typeinfo const& );
    typeinfo& operator=( typeinfo const& );

    char const * name_;

public:

    explicit typeinfo( char const * name ): name_( name )
    {
    }

    bool operator==( typeinfo const& rhs ) const
    {
        return this == &rhs;
    }

    bool operator!=( typeinfo const& rhs ) const
    {
        return this != &rhs;
    }

    bool before( typeinfo const& rhs ) const
    {
        return std::less< typeinfo const* >()( this, &rhs );
    }

    char const* name() const
    {
        return name_;
    }
};

inline char const * demangled_name( core::typeinfo const & ti )
{
    return ti.name();
}

} // namespace core

namespace detail
{

template<class T> struct core_typeid_
{
    static ndnboost::core::typeinfo ti_;

    static char const * name()
    {
        return NDNBOOST_CURRENT_FUNCTION;
    }
};

#if defined(__SUNPRO_CC)
// see #4199, the Sun Studio compiler gets confused about static initialization 
// constructor arguments. But an assignment works just fine. 
template<class T> ndnboost::core::typeinfo core_typeid_< T >::ti_ = core_typeid_< T >::name();
#else
template<class T> ndnboost::core::typeinfo core_typeid_< T >::ti_(core_typeid_< T >::name());
#endif

template<class T> struct core_typeid_< T & >: core_typeid_< T >
{
};

template<class T> struct core_typeid_< T const >: core_typeid_< T >
{
};

template<class T> struct core_typeid_< T volatile >: core_typeid_< T >
{
};

template<class T> struct core_typeid_< T const volatile >: core_typeid_< T >
{
};

} // namespace detail

} // namespace ndnboost

#define NDNBOOST_CORE_TYPEID(T) (ndnboost::detail::core_typeid_<T>::ti_)

#else

#include <ndnboost/core/demangle.hpp>
#include <typeinfo>

namespace ndnboost
{

namespace core
{

#if defined( NDNBOOST_NO_STD_TYPEINFO )

typedef ::type_info typeinfo;

#else

typedef std::type_info typeinfo;

#endif

inline std::string demangled_name( core::typeinfo const & ti )
{
    return core::demangle( ti.name() );
}

} // namespace core

} // namespace ndnboost

#define NDNBOOST_CORE_TYPEID(T) typeid(T)

#endif

#endif  // #ifndef NDNBOOST_CORE_TYPEINFO_HPP_INCLUDED
