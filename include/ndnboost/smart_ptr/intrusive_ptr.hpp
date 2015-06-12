#ifndef NDNBOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED
#define NDNBOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED

//
//  intrusive_ptr.hpp
//
//  Copyright (c) 2001, 2002 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/intrusive_ptr.html for documentation.
//

#include <ndnboost/config.hpp>

#include <ndnboost/assert.hpp>
#include <ndnboost/detail/workaround.hpp>
#include <ndnboost/smart_ptr/detail/sp_convertible.hpp>
#include <ndnboost/smart_ptr/detail/sp_nullptr_t.hpp>

#include <ndnboost/config/no_tr1/functional.hpp>           // for std::less

#if !defined(NDNBOOST_NO_IOSTREAM)
#if !defined(NDNBOOST_NO_IOSFWD)
#include <iosfwd>               // for std::basic_ostream
#else
#include <ostream>
#endif
#endif


namespace ndnboost
{

//
//  intrusive_ptr
//
//  A smart pointer that uses intrusive reference counting.
//
//  Relies on unqualified calls to
//  
//      void intrusive_ptr_add_ref(T * p);
//      void intrusive_ptr_release(T * p);
//
//          (p != 0)
//
//  The object is responsible for destroying itself.
//

template<class T> class intrusive_ptr
{
private:

    typedef intrusive_ptr this_type;

public:

    typedef T element_type;

    intrusive_ptr() NDNBOOST_NOEXCEPT : px( 0 )
    {
    }

    intrusive_ptr( T * p, bool add_ref = true ): px( p )
    {
        if( px != 0 && add_ref ) intrusive_ptr_add_ref( px );
    }

#if !defined(NDNBOOST_NO_MEMBER_TEMPLATES) || defined(NDNBOOST_MSVC6_MEMBER_TEMPLATES)

    template<class U>
#if !defined( NDNBOOST_SP_NO_SP_CONVERTIBLE )

    intrusive_ptr( intrusive_ptr<U> const & rhs, typename ndnboost::detail::sp_enable_if_convertible<U,T>::type = ndnboost::detail::sp_empty() )

#else

    intrusive_ptr( intrusive_ptr<U> const & rhs )

#endif
    : px( rhs.get() )
    {
        if( px != 0 ) intrusive_ptr_add_ref( px );
    }

#endif

    intrusive_ptr(intrusive_ptr const & rhs): px( rhs.px )
    {
        if( px != 0 ) intrusive_ptr_add_ref( px );
    }

    ~intrusive_ptr()
    {
        if( px != 0 ) intrusive_ptr_release( px );
    }

#if !defined(NDNBOOST_NO_MEMBER_TEMPLATES) || defined(NDNBOOST_MSVC6_MEMBER_TEMPLATES)

    template<class U> intrusive_ptr & operator=(intrusive_ptr<U> const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

#endif

// Move support

#if !defined( NDNBOOST_NO_CXX11_RVALUE_REFERENCES )

    intrusive_ptr(intrusive_ptr && rhs) NDNBOOST_NOEXCEPT : px( rhs.px )
    {
        rhs.px = 0;
    }

    intrusive_ptr & operator=(intrusive_ptr && rhs) NDNBOOST_NOEXCEPT
    {
        this_type( static_cast< intrusive_ptr && >( rhs ) ).swap(*this);
        return *this;
    }

#endif

    intrusive_ptr & operator=(intrusive_ptr const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    intrusive_ptr & operator=(T * rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    void reset() NDNBOOST_NOEXCEPT
    {
        this_type().swap( *this );
    }

    void reset( T * rhs )
    {
        this_type( rhs ).swap( *this );
    }

    void reset( T * rhs, bool add_ref )
    {
        this_type( rhs, add_ref ).swap( *this );
    }

    T * get() const NDNBOOST_NOEXCEPT
    {
        return px;
    }

    T * detach() NDNBOOST_NOEXCEPT
    {
        T * ret = px;
        px = 0;
        return ret;
    }

    T & operator*() const
    {
        NDNBOOST_ASSERT( px != 0 );
        return *px;
    }

    T * operator->() const
    {
        NDNBOOST_ASSERT( px != 0 );
        return px;
    }

// implicit conversion to "bool"
#include <ndnboost/smart_ptr/detail/operator_bool.hpp>

    void swap(intrusive_ptr & rhs) NDNBOOST_NOEXCEPT
    {
        T * tmp = px;
        px = rhs.px;
        rhs.px = tmp;
    }

private:

    T * px;
};

template<class T, class U> inline bool operator==(intrusive_ptr<T> const & a, intrusive_ptr<U> const & b)
{
    return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(intrusive_ptr<T> const & a, intrusive_ptr<U> const & b)
{
    return a.get() != b.get();
}

template<class T, class U> inline bool operator==(intrusive_ptr<T> const & a, U * b)
{
    return a.get() == b;
}

template<class T, class U> inline bool operator!=(intrusive_ptr<T> const & a, U * b)
{
    return a.get() != b;
}

template<class T, class U> inline bool operator==(T * a, intrusive_ptr<U> const & b)
{
    return a == b.get();
}

template<class T, class U> inline bool operator!=(T * a, intrusive_ptr<U> const & b)
{
    return a != b.get();
}

#if __GNUC__ == 2 && __GNUC_MINOR__ <= 96

// Resolve the ambiguity between our op!= and the one in rel_ops

template<class T> inline bool operator!=(intrusive_ptr<T> const & a, intrusive_ptr<T> const & b)
{
    return a.get() != b.get();
}

#endif

#if !defined( NDNBOOST_NO_CXX11_NULLPTR )

template<class T> inline bool operator==( intrusive_ptr<T> const & p, ndnboost::detail::sp_nullptr_t ) NDNBOOST_NOEXCEPT
{
    return p.get() == 0;
}

template<class T> inline bool operator==( ndnboost::detail::sp_nullptr_t, intrusive_ptr<T> const & p ) NDNBOOST_NOEXCEPT
{
    return p.get() == 0;
}

template<class T> inline bool operator!=( intrusive_ptr<T> const & p, ndnboost::detail::sp_nullptr_t ) NDNBOOST_NOEXCEPT
{
    return p.get() != 0;
}

template<class T> inline bool operator!=( ndnboost::detail::sp_nullptr_t, intrusive_ptr<T> const & p ) NDNBOOST_NOEXCEPT
{
    return p.get() != 0;
}

#endif

template<class T> inline bool operator<(intrusive_ptr<T> const & a, intrusive_ptr<T> const & b)
{
    return std::less<T *>()(a.get(), b.get());
}

template<class T> void swap(intrusive_ptr<T> & lhs, intrusive_ptr<T> & rhs)
{
    lhs.swap(rhs);
}

// mem_fn support

template<class T> T * get_pointer(intrusive_ptr<T> const & p)
{
    return p.get();
}

template<class T, class U> intrusive_ptr<T> static_pointer_cast(intrusive_ptr<U> const & p)
{
    return static_cast<T *>(p.get());
}

template<class T, class U> intrusive_ptr<T> const_pointer_cast(intrusive_ptr<U> const & p)
{
    return const_cast<T *>(p.get());
}

template<class T, class U> intrusive_ptr<T> dynamic_pointer_cast(intrusive_ptr<U> const & p)
{
    return dynamic_cast<T *>(p.get());
}

// operator<<

#if !defined(NDNBOOST_NO_IOSTREAM)

#if defined(NDNBOOST_NO_TEMPLATED_IOSTREAMS) || ( defined(__GNUC__) &&  (__GNUC__ < 3) )

template<class Y> std::ostream & operator<< (std::ostream & os, intrusive_ptr<Y> const & p)
{
    os << p.get();
    return os;
}

#else

// in STLport's no-iostreams mode no iostream symbols can be used
#ifndef _STLP_NO_IOSTREAMS

# if defined(NDNBOOST_MSVC) && NDNBOOST_WORKAROUND(NDNBOOST_MSVC, < 1300 && __SGI_STL_PORT)
// MSVC6 has problems finding std::basic_ostream through the using declaration in namespace _STL
using std::basic_ostream;
template<class E, class T, class Y> basic_ostream<E, T> & operator<< (basic_ostream<E, T> & os, intrusive_ptr<Y> const & p)
# else
template<class E, class T, class Y> std::basic_ostream<E, T> & operator<< (std::basic_ostream<E, T> & os, intrusive_ptr<Y> const & p)
# endif 
{
    os << p.get();
    return os;
}

#endif // _STLP_NO_IOSTREAMS

#endif // __GNUC__ < 3

#endif // !defined(NDNBOOST_NO_IOSTREAM)

// hash_value

template< class T > struct hash;

template< class T > std::size_t hash_value( ndnboost::intrusive_ptr<T> const & p )
{
    return ndnboost::hash< T* >()( p.get() );
}

} // namespace ndnboost

#endif  // #ifndef NDNBOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED
