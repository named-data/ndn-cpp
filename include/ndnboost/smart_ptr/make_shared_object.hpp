#ifndef NDNBOOST_SMART_PTR_MAKE_SHARED_OBJECT_HPP_INCLUDED
#define NDNBOOST_SMART_PTR_MAKE_SHARED_OBJECT_HPP_INCLUDED

//  make_shared_object.hpp
//
//  Copyright (c) 2007, 2008, 2012 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//
//  See http://www.boost.org/libs/smart_ptr/make_shared.html
//  for documentation.

#include <ndnboost/config.hpp>
#include <ndnboost/smart_ptr/shared_ptr.hpp>
#include <ndnboost/smart_ptr/detail/sp_forward.hpp>
#include <ndnboost/type_traits/type_with_alignment.hpp>
#include <ndnboost/type_traits/alignment_of.hpp>
#include <cstddef>
#include <new>

namespace ndnboost
{

namespace detail
{

template< std::size_t N, std::size_t A > struct sp_aligned_storage
{
    union type
    {
        char data_[ N ];
        typename ndnboost::type_with_alignment< A >::type align_;
    };
};

template< class T > class sp_ms_deleter
{
private:

    typedef typename sp_aligned_storage< sizeof( T ), ::ndnboost::alignment_of< T >::value >::type storage_type;

    bool initialized_;
    storage_type storage_;

private:

    void destroy()
    {
        if( initialized_ )
        {
#if defined( __GNUC__ )

            // fixes incorrect aliasing warning
            T * p = reinterpret_cast< T* >( storage_.data_ );
            p->~T();

#else

            reinterpret_cast< T* >( storage_.data_ )->~T();

#endif

            initialized_ = false;
        }
    }

public:

    sp_ms_deleter() NDNBOOST_NOEXCEPT : initialized_( false )
    {
    }

    template<class A> explicit sp_ms_deleter( A const & ) NDNBOOST_NOEXCEPT : initialized_( false )
    {
    }

    // optimization: do not copy storage_
    sp_ms_deleter( sp_ms_deleter const & ) NDNBOOST_NOEXCEPT : initialized_( false )
    {
    }

    ~sp_ms_deleter()
    {
        destroy();
    }

    void operator()( T * )
    {
        destroy();
    }

    static void operator_fn( T* ) // operator() can't be static
    {
    }

    void * address() NDNBOOST_NOEXCEPT
    {
        return storage_.data_;
    }

    void set_initialized() NDNBOOST_NOEXCEPT
    {
        initialized_ = true;
    }
};

template< class T, class A > class sp_as_deleter
{
private:

    typedef typename sp_aligned_storage< sizeof( T ), ::ndnboost::alignment_of< T >::value >::type storage_type;

    storage_type storage_;
    A a_;
    bool initialized_;

private:

    void destroy()
    {
        if( initialized_ )
        {
            T * p = reinterpret_cast< T* >( storage_.data_ );

#if !defined( NDNBOOST_NO_CXX11_ALLOCATOR )

            std::allocator_traits<A>::destroy( a_, p );

#else

            p->~T();

#endif

            initialized_ = false;
        }
    }

public:

    sp_as_deleter( A const & a ) NDNBOOST_NOEXCEPT : a_( a ), initialized_( false )
    {
    }

    // optimization: do not copy storage_
    sp_as_deleter( sp_as_deleter const & r ) NDNBOOST_NOEXCEPT : a_( r.a_), initialized_( false )
    {
    }

    ~sp_as_deleter()
    {
        destroy();
    }

    void operator()( T * )
    {
        destroy();
    }

    static void operator_fn( T* ) // operator() can't be static
    {
    }

    void * address() NDNBOOST_NOEXCEPT
    {
        return storage_.data_;
    }

    void set_initialized() NDNBOOST_NOEXCEPT
    {
        initialized_ = true;
    }
};

template< class T > struct sp_if_not_array
{
    typedef ndnboost::shared_ptr< T > type;
};

#if !defined( NDNBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION )

template< class T > struct sp_if_not_array< T[] >
{
};

#if !defined( __BORLANDC__ ) || !NDNBOOST_WORKAROUND( __BORLANDC__, < 0x600 )

template< class T, std::size_t N > struct sp_if_not_array< T[N] >
{
};

#endif

#endif

} // namespace detail

#if !defined( NDNBOOST_NO_FUNCTION_TEMPLATE_ORDERING )
# define NDNBOOST_SP_MSD( T ) ndnboost::detail::sp_inplace_tag< ndnboost::detail::sp_ms_deleter< T > >()
#else
# define NDNBOOST_SP_MSD( T ) ndnboost::detail::sp_ms_deleter< T >()
#endif

// _noinit versions

template< class T > typename ndnboost::detail::sp_if_not_array< T >::type make_shared_noinit()
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T;
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A > typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared_noinit( A const & a )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T;
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

#if !defined( NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES ) && !defined( NDNBOOST_NO_CXX11_RVALUE_REFERENCES )

// Variadic templates, rvalue reference

template< class T, class... Args > typename ndnboost::detail::sp_if_not_array< T >::type make_shared( Args && ... args )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( ndnboost::detail::sp_forward<Args>( args )... );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class... Args > typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, Args && ... args )
{
#if !defined( NDNBOOST_NO_CXX11_ALLOCATOR )

    typedef typename std::allocator_traits<A>::template rebind_alloc<T> A2;
    A2 a2( a );

    typedef ndnboost::detail::sp_as_deleter< T, A2 > D;

    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), ndnboost::detail::sp_inplace_tag<D>(), a2 );

#else

    typedef ndnboost::detail::sp_ms_deleter< T > D;

    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), ndnboost::detail::sp_inplace_tag<D>(), a );

#endif

    D * pd = static_cast< D* >( pt._internal_get_untyped_deleter() );
    void * pv = pd->address();

#if !defined( NDNBOOST_NO_CXX11_ALLOCATOR )

    std::allocator_traits<A2>::construct( a2, static_cast< T* >( pv ), ndnboost::detail::sp_forward<Args>( args )... );

#else

    ::new( pv ) T( ndnboost::detail::sp_forward<Args>( args )... );

#endif

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

#else // !defined( NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES ) && !defined( NDNBOOST_NO_CXX11_RVALUE_REFERENCES )

// Common zero-argument versions

template< class T > typename ndnboost::detail::sp_if_not_array< T >::type make_shared()
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T();
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A > typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T();
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

#if !defined( NDNBOOST_NO_CXX11_RVALUE_REFERENCES )

// For example MSVC 10.0

template< class T, class A1 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 && a1 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T(
        ndnboost::detail::sp_forward<A1>( a1 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 && a1 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( 
        ndnboost::detail::sp_forward<A1>( a1 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 && a1, A2 && a2 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T(
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 && a1, A2 && a2 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( 
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2, class A3 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 && a1, A2 && a2, A3 && a3 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T(
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 ), 
        ndnboost::detail::sp_forward<A3>( a3 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2, class A3 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 && a1, A2 && a2, A3 && a3 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( 
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 ), 
        ndnboost::detail::sp_forward<A3>( a3 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2, class A3, class A4 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 && a1, A2 && a2, A3 && a3, A4 && a4 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T(
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 ), 
        ndnboost::detail::sp_forward<A3>( a3 ), 
        ndnboost::detail::sp_forward<A4>( a4 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2, class A3, class A4 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 && a1, A2 && a2, A3 && a3, A4 && a4 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( 
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 ), 
        ndnboost::detail::sp_forward<A3>( a3 ), 
        ndnboost::detail::sp_forward<A4>( a4 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2, class A3, class A4, class A5 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T(
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 ), 
        ndnboost::detail::sp_forward<A3>( a3 ), 
        ndnboost::detail::sp_forward<A4>( a4 ), 
        ndnboost::detail::sp_forward<A5>( a5 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2, class A3, class A4, class A5 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( 
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 ), 
        ndnboost::detail::sp_forward<A3>( a3 ), 
        ndnboost::detail::sp_forward<A4>( a4 ), 
        ndnboost::detail::sp_forward<A5>( a5 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2, class A3, class A4, class A5, class A6 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5, A6 && a6 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T(
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 ), 
        ndnboost::detail::sp_forward<A3>( a3 ), 
        ndnboost::detail::sp_forward<A4>( a4 ), 
        ndnboost::detail::sp_forward<A5>( a5 ), 
        ndnboost::detail::sp_forward<A6>( a6 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2, class A3, class A4, class A5, class A6 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5, A6 && a6 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( 
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 ), 
        ndnboost::detail::sp_forward<A3>( a3 ), 
        ndnboost::detail::sp_forward<A4>( a4 ), 
        ndnboost::detail::sp_forward<A5>( a5 ), 
        ndnboost::detail::sp_forward<A6>( a6 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5, A6 && a6, A7 && a7 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T(
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 ), 
        ndnboost::detail::sp_forward<A3>( a3 ), 
        ndnboost::detail::sp_forward<A4>( a4 ), 
        ndnboost::detail::sp_forward<A5>( a5 ), 
        ndnboost::detail::sp_forward<A6>( a6 ), 
        ndnboost::detail::sp_forward<A7>( a7 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2, class A3, class A4, class A5, class A6, class A7 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5, A6 && a6, A7 && a7 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( 
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 ), 
        ndnboost::detail::sp_forward<A3>( a3 ), 
        ndnboost::detail::sp_forward<A4>( a4 ), 
        ndnboost::detail::sp_forward<A5>( a5 ), 
        ndnboost::detail::sp_forward<A6>( a6 ), 
        ndnboost::detail::sp_forward<A7>( a7 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5, A6 && a6, A7 && a7, A8 && a8 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T(
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 ), 
        ndnboost::detail::sp_forward<A3>( a3 ), 
        ndnboost::detail::sp_forward<A4>( a4 ), 
        ndnboost::detail::sp_forward<A5>( a5 ), 
        ndnboost::detail::sp_forward<A6>( a6 ), 
        ndnboost::detail::sp_forward<A7>( a7 ), 
        ndnboost::detail::sp_forward<A8>( a8 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5, A6 && a6, A7 && a7, A8 && a8 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( 
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 ), 
        ndnboost::detail::sp_forward<A3>( a3 ), 
        ndnboost::detail::sp_forward<A4>( a4 ), 
        ndnboost::detail::sp_forward<A5>( a5 ), 
        ndnboost::detail::sp_forward<A6>( a6 ), 
        ndnboost::detail::sp_forward<A7>( a7 ), 
        ndnboost::detail::sp_forward<A8>( a8 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5, A6 && a6, A7 && a7, A8 && a8, A9 && a9 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T(
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 ), 
        ndnboost::detail::sp_forward<A3>( a3 ), 
        ndnboost::detail::sp_forward<A4>( a4 ), 
        ndnboost::detail::sp_forward<A5>( a5 ), 
        ndnboost::detail::sp_forward<A6>( a6 ), 
        ndnboost::detail::sp_forward<A7>( a7 ), 
        ndnboost::detail::sp_forward<A8>( a8 ), 
        ndnboost::detail::sp_forward<A9>( a9 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5, A6 && a6, A7 && a7, A8 && a8, A9 && a9 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( 
        ndnboost::detail::sp_forward<A1>( a1 ), 
        ndnboost::detail::sp_forward<A2>( a2 ), 
        ndnboost::detail::sp_forward<A3>( a3 ), 
        ndnboost::detail::sp_forward<A4>( a4 ), 
        ndnboost::detail::sp_forward<A5>( a5 ), 
        ndnboost::detail::sp_forward<A6>( a6 ), 
        ndnboost::detail::sp_forward<A7>( a7 ), 
        ndnboost::detail::sp_forward<A8>( a8 ), 
        ndnboost::detail::sp_forward<A9>( a9 )
        );

    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

#else // !defined( NDNBOOST_NO_CXX11_RVALUE_REFERENCES )

// C++03 version

template< class T, class A1 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 const & a1 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 const & a1 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 const & a1, A2 const & a2 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 const & a1, A2 const & a2 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2, class A3 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 const & a1, A2 const & a2, A3 const & a3 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2, a3 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2, class A3 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 const & a1, A2 const & a2, A3 const & a3 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2, a3 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2, class A3, class A4 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2, a3, a4 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2, class A3, class A4 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2, a3, a4 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2, class A3, class A4, class A5 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2, a3, a4, a5 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2, class A3, class A4, class A5 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2, a3, a4, a5 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2, class A3, class A4, class A5, class A6 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2, a3, a4, a5, a6 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2, class A3, class A4, class A5, class A6 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2, a3, a4, a5, a6 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2, a3, a4, a5, a6, a7 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2, class A3, class A4, class A5, class A6, class A7 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2, a3, a4, a5, a6, a7 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2, a3, a4, a5, a6, a7, a8 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2, a3, a4, a5, a6, a7, a8 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9 >
typename ndnboost::detail::sp_if_not_array< T >::type make_shared( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8, A9 const & a9 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ) );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2, a3, a4, a5, a6, a7, a8, a9 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

template< class T, class A, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9 >
typename ndnboost::detail::sp_if_not_array< T >::type allocate_shared( A const & a, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8, A9 const & a9 )
{
    ndnboost::shared_ptr< T > pt( static_cast< T* >( 0 ), NDNBOOST_SP_MSD( T ), a );

    ndnboost::detail::sp_ms_deleter< T > * pd = static_cast<ndnboost::detail::sp_ms_deleter< T > *>( pt._internal_get_untyped_deleter() );

    void * pv = pd->address();

    ::new( pv ) T( a1, a2, a3, a4, a5, a6, a7, a8, a9 );
    pd->set_initialized();

    T * pt2 = static_cast< T* >( pv );

    ndnboost::detail::sp_enable_shared_from_this( &pt, pt2, pt2 );
    return ndnboost::shared_ptr< T >( pt, pt2 );
}

#endif // !defined( NDNBOOST_NO_CXX11_RVALUE_REFERENCES )

#endif // !defined( NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES ) && !defined( NDNBOOST_NO_CXX11_RVALUE_REFERENCES )

#undef NDNBOOST_SP_MSD

} // namespace ndnboost

#endif // #ifndef NDNBOOST_SMART_PTR_MAKE_SHARED_OBJECT_HPP_INCLUDED
