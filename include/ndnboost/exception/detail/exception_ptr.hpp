//Copyright (c) 2006-2009 Emil Dotchevski and Reverge Studios, Inc.

//Distributed under the Boost Software License, Version 1.0. (See accompanying
//file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef NDNBOOST_UUID_618474C2DE1511DEB74A388C56D89593
#define NDNBOOST_UUID_618474C2DE1511DEB74A388C56D89593
#if (__GNUC__*100+__GNUC_MINOR__>301) && !defined(NDNBOOST_EXCEPTION_ENABLE_WARNINGS)
#pragma GCC system_header
#endif
#if defined(_MSC_VER) && !defined(NDNBOOST_EXCEPTION_ENABLE_WARNINGS)
#pragma warning(push,1)
#endif

#include <ndnboost/config.hpp>
#ifdef NDNBOOST_NO_EXCEPTIONS
#error This header requires exception handling to be enabled.
#endif
#include <ndnboost/exception/exception.hpp>
#include <ndnboost/exception/info.hpp>
#include <ndnboost/exception/diagnostic_information.hpp>
#include <ndnboost/exception/detail/type_info.hpp>
#include <ndnboost/exception/detail/clone_current_exception.hpp>
//#ifndef NDNBOOST_NO_RTTI
//#include <ndnboost/units/detail/utility.hpp>
//#endif
#include <ndnboost/shared_ptr.hpp>
#include <stdexcept>
#include <new>
#include <ios>
#include <stdlib.h>

namespace
ndnboost
    {
    class exception_ptr;
    NDNBOOST_NORETURN void rethrow_exception( exception_ptr const & );
    exception_ptr current_exception();

    class
    exception_ptr
        {
        typedef ndnboost::shared_ptr<exception_detail::clone_base const> impl;
        impl ptr_;
        friend void rethrow_exception( exception_ptr const & );
        typedef exception_detail::clone_base const * (impl::*unspecified_bool_type)() const;
        public:
        exception_ptr()
            {
            }
        explicit
        exception_ptr( impl const & ptr ):
            ptr_(ptr)
            {
            }
        bool
        operator==( exception_ptr const & other ) const
            {
            return ptr_==other.ptr_;
            }
        bool
        operator!=( exception_ptr const & other ) const
            {
            return ptr_!=other.ptr_;
            }
        operator unspecified_bool_type() const
            {
            return ptr_?&impl::get:0;
            }
        };

    template <class T>
    inline
    exception_ptr
    copy_exception( T const & e )
        {
        try
            {
            throw enable_current_exception(e);
            }
        catch(
        ... )
            {
            return current_exception();
            }
        }

#ifndef NDNBOOST_NO_RTTI
    typedef error_info<struct tag_original_exception_type,std::type_info const *> original_exception_type;

    inline
    std::string
    to_string( original_exception_type const & x )
        {
        return /*units::detail::demangle*/(x.value()->name());
        }
#endif

    namespace
    exception_detail
        {
        struct
        bad_alloc_:
            ndnboost::exception,
            std::bad_alloc
                {
                ~bad_alloc_() throw() { }
                };

        struct
        bad_exception_:
            ndnboost::exception,
            std::bad_exception
                {
                ~bad_exception_() throw() { }
                };

        template <class Exception>
        exception_ptr
        get_static_exception_object()
            {
            Exception ba;
            exception_detail::clone_impl<Exception> c(ba);
#ifndef NDNBOOST_EXCEPTION_DISABLE
            c <<
                throw_function(NDNBOOST_CURRENT_FUNCTION) <<
                throw_file(__FILE__) <<
                throw_line(__LINE__);
#endif
            static exception_ptr ep(shared_ptr<exception_detail::clone_base const>(new exception_detail::clone_impl<Exception>(c)));
            return ep;
            }

        template <class Exception>
        struct
        exception_ptr_static_exception_object
            {
            static exception_ptr const e;
            };

        template <class Exception>
        exception_ptr const
        exception_ptr_static_exception_object<Exception>::
        e = get_static_exception_object<Exception>();
        }

#if defined(__GNUC__)
# if (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUC__ > 4)
#  pragma GCC visibility push (default)
# endif
#endif
    class
    unknown_exception:
        public ndnboost::exception,
        public std::exception
        {
        public:

        unknown_exception()
            {
            }

        explicit
        unknown_exception( std::exception const & e )
            {
            add_original_type(e);
            }

        explicit
        unknown_exception( ndnboost::exception const & e ):
            ndnboost::exception(e)
            {
            add_original_type(e);
            }

        ~unknown_exception() throw()
            {
            }

        private:

        template <class E>
        void
        add_original_type( E const & e )
            {
#ifndef NDNBOOST_NO_RTTI
            (*this) << original_exception_type(&typeid(e));
#endif
            }
        };
#if defined(__GNUC__)
# if (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUC__ > 4)
#  pragma GCC visibility pop
# endif
#endif

    namespace
    exception_detail
        {
        template <class T>
        class
        current_exception_std_exception_wrapper:
            public T,
            public ndnboost::exception
            {
            public:

            explicit
            current_exception_std_exception_wrapper( T const & e1 ):
                T(e1)
                {
                add_original_type(e1);
                }

            current_exception_std_exception_wrapper( T const & e1, ndnboost::exception const & e2 ):
                T(e1),
                ndnboost::exception(e2)
                {
                add_original_type(e1);
                }

            ~current_exception_std_exception_wrapper() throw()
                {
                }

            private:

            template <class E>
            void
            add_original_type( E const & e )
                {
#ifndef NDNBOOST_NO_RTTI
                (*this) << original_exception_type(&typeid(e));
#endif
                }
            };

#ifdef NDNBOOST_NO_RTTI
        template <class T>
        ndnboost::exception const *
        get_boost_exception( T const * )
            {
            try
                {
                throw;
                }
            catch(
            ndnboost::exception & x )
                {
                return &x;
                }
            catch(...)
                {
                return 0;
                }
            }
#else
        template <class T>
        ndnboost::exception const *
        get_boost_exception( T const * x )
            {
            return dynamic_cast<ndnboost::exception const *>(x);
            }
#endif

        template <class T>
        inline
        exception_ptr
        current_exception_std_exception( T const & e1 )
            {
            if( ndnboost::exception const * e2 = get_boost_exception(&e1) )
                return ndnboost::copy_exception(current_exception_std_exception_wrapper<T>(e1,*e2));
            else
                return ndnboost::copy_exception(current_exception_std_exception_wrapper<T>(e1));
            }

        inline
        exception_ptr
        current_exception_unknown_exception()
            {
            return ndnboost::copy_exception(unknown_exception());
            }

        inline
        exception_ptr
        current_exception_unknown_boost_exception( ndnboost::exception const & e )
            {
            return ndnboost::copy_exception(unknown_exception(e));
            }

        inline
        exception_ptr
        current_exception_unknown_std_exception( std::exception const & e )
            {
            if( ndnboost::exception const * be = get_boost_exception(&e) )
                return current_exception_unknown_boost_exception(*be);
            else
                return ndnboost::copy_exception(unknown_exception(e));
            }

        inline
        exception_ptr
        current_exception_impl()
            {
            exception_detail::clone_base const * e=0;
            switch(
            exception_detail::clone_current_exception(e) )
                {
                case exception_detail::clone_current_exception_result::
                success:
                    {
                    NDNBOOST_ASSERT(e!=0);
                    return exception_ptr(shared_ptr<exception_detail::clone_base const>(e));
                    }
                case exception_detail::clone_current_exception_result::
                bad_alloc:
                    {
                    NDNBOOST_ASSERT(!e);
                    return exception_detail::exception_ptr_static_exception_object<bad_alloc_>::e;
                    }
                case exception_detail::clone_current_exception_result::
                bad_exception:
                    {
                    NDNBOOST_ASSERT(!e);
                    return exception_detail::exception_ptr_static_exception_object<bad_exception_>::e;
                    }
                default:
                    NDNBOOST_ASSERT(0);
                case exception_detail::clone_current_exception_result::
                not_supported:
                    {
                    NDNBOOST_ASSERT(!e);
                    try
                        {
                        throw;
                        }
                    catch(
                    exception_detail::clone_base & e )
                        {
                        return exception_ptr(shared_ptr<exception_detail::clone_base const>(e.clone()));
                        }
                    catch(
                    std::domain_error & e )
                        {
                        return exception_detail::current_exception_std_exception(e);
                        }
                    catch(
                    std::invalid_argument & e )
                        {
                        return exception_detail::current_exception_std_exception(e);
                        }
                    catch(
                    std::length_error & e )
                        {
                        return exception_detail::current_exception_std_exception(e);
                        }
                    catch(
                    std::out_of_range & e )
                        {
                        return exception_detail::current_exception_std_exception(e);
                        }
                    catch(
                    std::logic_error & e )
                        {
                        return exception_detail::current_exception_std_exception(e);
                        }
                    catch(
                    std::range_error & e )
                        {
                        return exception_detail::current_exception_std_exception(e);
                        }
                    catch(
                    std::overflow_error & e )
                        {
                        return exception_detail::current_exception_std_exception(e);
                        }
                    catch(
                    std::underflow_error & e )
                        {
                        return exception_detail::current_exception_std_exception(e);
                        }
                    catch(
                    std::ios_base::failure & e )
                        {
                        return exception_detail::current_exception_std_exception(e);
                        }
                    catch(
                    std::runtime_error & e )
                        {
                        return exception_detail::current_exception_std_exception(e);
                        }
                    catch(
                    std::bad_alloc & e )
                        {
                        return exception_detail::current_exception_std_exception(e);
                        }
#ifndef NDNBOOST_NO_TYPEID
                    catch(
                    std::bad_cast & e )
                        {
                        return exception_detail::current_exception_std_exception(e);
                        }
                    catch(
                    std::bad_typeid & e )
                        {
                        return exception_detail::current_exception_std_exception(e);
                        }
#endif
                    catch(
                    std::bad_exception & e )
                        {
                        return exception_detail::current_exception_std_exception(e);
                        }
                    catch(
                    std::exception & e )
                        {
                        return exception_detail::current_exception_unknown_std_exception(e);
                        }
                    catch(
                    ndnboost::exception & e )
                        {
                        return exception_detail::current_exception_unknown_boost_exception(e);
                        }
                    catch(
                    ... )
                        {
                        return exception_detail::current_exception_unknown_exception();
                        }
                    }
                }
            }
        }

    inline
    exception_ptr
    current_exception()
        {
        exception_ptr ret;
        try
            {
            ret=exception_detail::current_exception_impl();
            }
        catch(
        std::bad_alloc & )
            {
            ret=exception_detail::exception_ptr_static_exception_object<exception_detail::bad_alloc_>::e;
            }
        catch(
        ... )
            {
            ret=exception_detail::exception_ptr_static_exception_object<exception_detail::bad_exception_>::e;
            }
        NDNBOOST_ASSERT(ret);
        return ret;
        }

    NDNBOOST_NORETURN
    inline
    void
    rethrow_exception( exception_ptr const & p )
        {
        NDNBOOST_ASSERT(p);
        p.ptr_->rethrow();
        NDNBOOST_ASSERT(0);
        #if defined(UNDER_CE)
            // some CE platforms don't define ::abort()
            exit(-1);
        #else
            abort();
        #endif
        }

    inline
    std::string
    diagnostic_information( exception_ptr const & p, bool verbose=true )
        {
        if( p )
            try
                {
                rethrow_exception(p);
                }
            catch(
            ... )
                {
                return current_exception_diagnostic_information(verbose);
                }
        return "<empty>";
        }

    inline
    std::string
    to_string( exception_ptr const & p )
        {
        std::string s='\n'+diagnostic_information(p);
        std::string padding("  ");
        std::string r;
        bool f=false;
        for( std::string::const_iterator i=s.begin(),e=s.end(); i!=e; ++i )
            {
            if( f )
                r+=padding;
            char c=*i;
            r+=c;
            f=(c=='\n');
            }
        return r;
        }
    }

#if defined(_MSC_VER) && !defined(NDNBOOST_EXCEPTION_ENABLE_WARNINGS)
#pragma warning(pop)
#endif
#endif
