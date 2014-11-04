#ifndef NDNBOOST_LEXICAL_CAST_INCLUDED
#define NDNBOOST_LEXICAL_CAST_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER)
# pragma once
#endif

// Boost lexical_cast.hpp header  -------------------------------------------//
//
// See http://www.boost.org/libs/conversion for documentation.
// See end of this header for rights and permissions.
//
// what:  lexical_cast custom keyword cast
// who:   contributed by Kevlin Henney,
//        enhanced with contributions from Terje Slettebo,
//        with additional fixes and suggestions from Gennaro Prota,
//        Beman Dawes, Dave Abrahams, Daryle Walker, Peter Dimov,
//        Alexander Nasonov, Antony Polukhin, Justin Viiret, Michael Hofmann,
//        Cheng Yang, Matthew Bradbury, David W. Birdsall, Pavel Korzh and other Boosters
// when:  November 2000, March 2003, June 2005, June 2006, March 2011 - 2014

#include <ndnboost/config.hpp>
#if defined(NDNBOOST_NO_STRINGSTREAM) || defined(NDNBOOST_NO_STD_WSTRING)
#define NDNBOOST_LCAST_NO_WCHAR_T
#endif

#include <climits>
#include <cstddef>
#include <string>
#include <cstring>
#include <cstdio>
#include <typeinfo>
#include <exception>
#include <ndnboost/limits.hpp>
#include <ndnboost/mpl/if.hpp>
#include <ndnboost/throw_exception.hpp>
#include <ndnboost/type_traits/ice.hpp>
#include <ndnboost/type_traits/is_pointer.hpp>
#include <ndnboost/static_assert.hpp>
#include <ndnboost/detail/lcast_precision.hpp>
#include <ndnboost/detail/workaround.hpp>


#ifndef NDNBOOST_NO_STD_LOCALE
#   include <locale>
#else
#   ifndef NDNBOOST_LEXICAL_CAST_ASSUME_C_LOCALE
        // Getting error at this point means, that your STL library is old/lame/misconfigured.
        // If nothing can be done with STL library, define NDNBOOST_LEXICAL_CAST_ASSUME_C_LOCALE,
        // but beware: lexical_cast will understand only 'C' locale delimeters and thousands
        // separators.
#       error "Unable to use <locale> header. Define NDNBOOST_LEXICAL_CAST_ASSUME_C_LOCALE to force "
#       error "ndnboost::lexical_cast to use only 'C' locale during conversions."
#   endif
#endif

#ifdef NDNBOOST_NO_STRINGSTREAM
#include <strstream>
#else
#include <sstream>
#endif

#ifdef NDNBOOST_NO_TYPEID
#define NDNBOOST_LCAST_THROW_BAD_CAST(S, T) throw_exception(bad_lexical_cast())
#else
#define NDNBOOST_LCAST_THROW_BAD_CAST(Source, Target) \
    throw_exception(bad_lexical_cast(typeid(Source), typeid(Target)))
#endif

namespace ndnboost
{
    // exception used to indicate runtime lexical_cast failure
    class NDNBOOST_SYMBOL_VISIBLE bad_lexical_cast :
    // workaround MSVC bug with std::bad_cast when _HAS_EXCEPTIONS == 0 
#if defined(NDNBOOST_MSVC) && defined(_HAS_EXCEPTIONS) && !_HAS_EXCEPTIONS 
        public std::exception 
#else 
        public std::bad_cast 
#endif 

#if defined(__BORLANDC__) && NDNBOOST_WORKAROUND( __BORLANDC__, < 0x560 )
        // under bcc32 5.5.1 bad_cast doesn't derive from exception
        , public std::exception
#endif

    {
    public:
        bad_lexical_cast() NDNBOOST_NOEXCEPT
#ifndef NDNBOOST_NO_TYPEID
           : source(&typeid(void)), target(&typeid(void))
#endif
        {}

        virtual const char *what() const NDNBOOST_NOEXCEPT_OR_NOTHROW {
            return "bad lexical cast: "
                   "source type value could not be interpreted as target";
        }

        virtual ~bad_lexical_cast() NDNBOOST_NOEXCEPT_OR_NOTHROW
        {}

#ifndef NDNBOOST_NO_TYPEID
        bad_lexical_cast(
                const std::type_info &source_type_arg,
                const std::type_info &target_type_arg) NDNBOOST_NOEXCEPT
            : source(&source_type_arg), target(&target_type_arg)
        {}

        const std::type_info &source_type() const NDNBOOST_NOEXCEPT {
            return *source;
        }

        const std::type_info &target_type() const NDNBOOST_NOEXCEPT {
            return *target;
        }

    private:
        const std::type_info *source;
        const std::type_info *target;
#endif
    };

    namespace detail // widest_char
    {
        template <typename TargetChar, typename SourceChar>
        struct widest_char
        {
            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::mpl::if_c<
                (sizeof(TargetChar) > sizeof(SourceChar))
                , TargetChar
                , SourceChar >::type type;
        };
    }
} // namespace ndnboost

#if !defined(__SUNPRO_CC) && !defined(__PGIC__)

#include <cmath>
#include <istream>

#ifndef NDNBOOST_NO_CXX11_HDR_ARRAY
#include <array>
#endif

#include <ndnboost/array.hpp>
#include <ndnboost/numeric/conversion/cast.hpp>
#include <ndnboost/type_traits/make_unsigned.hpp>
#include <ndnboost/type_traits/is_signed.hpp>
#include <ndnboost/type_traits/is_integral.hpp>
#include <ndnboost/type_traits/is_arithmetic.hpp>
#include <ndnboost/type_traits/remove_pointer.hpp>
#include <ndnboost/type_traits/has_left_shift.hpp>
#include <ndnboost/type_traits/has_right_shift.hpp>
#include <ndnboost/math/special_functions/sign.hpp>
#include <ndnboost/math/special_functions/fpclassify.hpp>
#include <ndnboost/range/iterator_range_core.hpp>
#include <ndnboost/container/container_fwd.hpp>
#include <ndnboost/integer.hpp>
#include <ndnboost/detail/basic_pointerbuf.hpp>
#include <ndnboost/noncopyable.hpp>
#ifndef NDNBOOST_NO_CWCHAR
#   include <cwchar>
#endif

namespace ndnboost {

    namespace detail // is_character<...>
    {
        // returns true, if T is one of the character types
        template < typename T >
        struct is_character
        {
            typedef ndnboost::type_traits::ice_or<
                    ndnboost::is_same< T, char >::value,
                    #ifndef NDNBOOST_LCAST_NO_WCHAR_T
                        ndnboost::is_same< T, wchar_t >::value,
                    #endif
                    #ifndef NDNBOOST_NO_CXX11_CHAR16_T
                        ndnboost::is_same< T, char16_t >::value,
                    #endif
                    #ifndef NDNBOOST_NO_CXX11_CHAR32_T
                        ndnboost::is_same< T, char32_t >::value,
                    #endif
                    ndnboost::is_same< T, unsigned char >::value,
                    ndnboost::is_same< T, signed char >::value
            > result_type;

            NDNBOOST_STATIC_CONSTANT(bool, value = (result_type::value) );
        };
    }

    namespace detail // normalize_single_byte_char<Char>
    {
        // Converts signed/unsigned char to char
        template < class Char >
        struct normalize_single_byte_char 
        {
            typedef Char type;
        };

        template <>
        struct normalize_single_byte_char< signed char >
        {
            typedef char type;
        };

        template <>
        struct normalize_single_byte_char< unsigned char >
        {
            typedef char type;
        };
    }

    namespace detail // deduce_character_type_later<T>
    {
        // Helper type, meaning that stram character for T must be deduced 
        // at Stage 2 (See deduce_source_char<T> and deduce_target_char<T>)
        template < class T > struct deduce_character_type_later {};
    }

    namespace detail // stream_char_common<T>
    {
        // Selectors to choose stream character type (common for Source and Target)
        // Returns one of char, wchar_t, char16_t, char32_t or deduce_character_type_later<T> types
        // Executed on Stage 1 (See deduce_source_char<T> and deduce_target_char<T>)
        template < typename Type >
        struct stream_char_common: public ndnboost::mpl::if_c<
            ndnboost::detail::is_character< Type >::value,
            Type,
            ndnboost::detail::deduce_character_type_later< Type >
        > {};

        template < typename Char >
        struct stream_char_common< Char* >: public ndnboost::mpl::if_c<
            ndnboost::detail::is_character< Char >::value,
            Char,
            ndnboost::detail::deduce_character_type_later< Char* >
        > {};

        template < typename Char >
        struct stream_char_common< const Char* >: public ndnboost::mpl::if_c<
            ndnboost::detail::is_character< Char >::value,
            Char,
            ndnboost::detail::deduce_character_type_later< const Char* >
        > {};

        template < typename Char >
        struct stream_char_common< ndnboost::iterator_range< Char* > >: public ndnboost::mpl::if_c<
            ndnboost::detail::is_character< Char >::value,
            Char,
            ndnboost::detail::deduce_character_type_later< ndnboost::iterator_range< Char* > >
        > {};
    
        template < typename Char >
        struct stream_char_common< ndnboost::iterator_range< const Char* > >: public ndnboost::mpl::if_c<
            ndnboost::detail::is_character< Char >::value,
            Char,
            ndnboost::detail::deduce_character_type_later< ndnboost::iterator_range< const Char* > >
        > {};

        template < class Char, class Traits, class Alloc >
        struct stream_char_common< std::basic_string< Char, Traits, Alloc > >
        {
            typedef Char type;
        };

        template < class Char, class Traits, class Alloc >
        struct stream_char_common< ndnboost::container::basic_string< Char, Traits, Alloc > >
        {
            typedef Char type;
        };

        template < typename Char, std::size_t N >
        struct stream_char_common< ndnboost::array< Char, N > >: public ndnboost::mpl::if_c<
            ndnboost::detail::is_character< Char >::value,
            Char,
            ndnboost::detail::deduce_character_type_later< ndnboost::array< Char, N > >
        > {};

        template < typename Char, std::size_t N >
        struct stream_char_common< ndnboost::array< const Char, N > >: public ndnboost::mpl::if_c<
            ndnboost::detail::is_character< Char >::value,
            Char,
            ndnboost::detail::deduce_character_type_later< ndnboost::array< const Char, N > >
        > {};

#ifndef NDNBOOST_NO_CXX11_HDR_ARRAY
        template < typename Char, std::size_t N >
        struct stream_char_common< std::array<Char, N > >: public ndnboost::mpl::if_c<
            ndnboost::detail::is_character< Char >::value,
            Char,
            ndnboost::detail::deduce_character_type_later< std::array< Char, N > >
        > {};

        template < typename Char, std::size_t N >
        struct stream_char_common< std::array< const Char, N > >: public ndnboost::mpl::if_c<
            ndnboost::detail::is_character< Char >::value,
            Char,
            ndnboost::detail::deduce_character_type_later< std::array< const Char, N > >
        > {};
#endif

#ifdef NDNBOOST_HAS_INT128
        template <> struct stream_char_common< ndnboost::int128_type >: public ndnboost::mpl::identity< char > {};
        template <> struct stream_char_common< ndnboost::uint128_type >: public ndnboost::mpl::identity< char > {};
#endif

#if !defined(NDNBOOST_LCAST_NO_WCHAR_T) && defined(NDNBOOST_NO_INTRINSIC_WCHAR_T)
        template <>
        struct stream_char_common< wchar_t >
        {
            typedef char type;
        };
#endif
    }

    namespace detail // deduce_source_char_impl<T>
    {
        // If type T is `deduce_character_type_later` type, then tries to deduce
        // character type using ndnboost::has_left_shift<T> metafunction.
        // Otherwise supplied type T is a character type, that must be normalized
        // using normalize_single_byte_char<Char>.
        // Executed at Stage 2  (See deduce_source_char<T> and deduce_target_char<T>)
        template < class Char > 
        struct deduce_source_char_impl
        { 
            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::detail::normalize_single_byte_char< Char >::type type; 
        };
        
        template < class T > 
        struct deduce_source_char_impl< deduce_character_type_later< T > > 
        {
            typedef ndnboost::has_left_shift< std::basic_ostream< char >, T > result_t;

#if defined(NDNBOOST_LCAST_NO_WCHAR_T)
            NDNBOOST_STATIC_ASSERT_MSG((result_t::value), 
                "Source type is not std::ostream`able and std::wostream`s are not supported by your STL implementation");
            typedef char type;
#else
            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::mpl::if_c<
                result_t::value, char, wchar_t
            >::type type;

            NDNBOOST_STATIC_ASSERT_MSG((result_t::value || ndnboost::has_left_shift< std::basic_ostream< type >, T >::value), 
                "Source type is neither std::ostream`able nor std::wostream`able");
#endif
        };
    }

    namespace detail  // deduce_target_char_impl<T>
    {
        // If type T is `deduce_character_type_later` type, then tries to deduce
        // character type using ndnboost::has_right_shift<T> metafunction.
        // Otherwise supplied type T is a character type, that must be normalized
        // using normalize_single_byte_char<Char>.
        // Executed at Stage 2  (See deduce_source_char<T> and deduce_target_char<T>)
        template < class Char > 
        struct deduce_target_char_impl 
        { 
            typedef NDNBOOST_DEDUCED_TYPENAME normalize_single_byte_char< Char >::type type; 
        };
        
        template < class T > 
        struct deduce_target_char_impl< deduce_character_type_later<T> > 
        { 
            typedef ndnboost::has_right_shift<std::basic_istream<char>, T > result_t;

#if defined(NDNBOOST_LCAST_NO_WCHAR_T)
            NDNBOOST_STATIC_ASSERT_MSG((result_t::value), 
                "Target type is not std::istream`able and std::wistream`s are not supported by your STL implementation");
            typedef char type;
#else
            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::mpl::if_c<
                result_t::value, char, wchar_t
            >::type type;
            
            NDNBOOST_STATIC_ASSERT_MSG((result_t::value || ndnboost::has_right_shift<std::basic_istream<wchar_t>, T >::value), 
                "Target type is neither std::istream`able nor std::wistream`able");
#endif
        };
    } 

    namespace detail  // deduce_target_char<T> and deduce_source_char<T>
    {
        // We deduce stream character types in two stages.
        //
        // Stage 1 is common for Target and Source. At Stage 1 we get 
        // non normalized character type (may contain unsigned/signed char)
        // or deduce_character_type_later<T> where T is the original type.
        // Stage 1 is executed by stream_char_common<T>
        //
        // At Stage 2 we normalize character types or try to deduce character 
        // type using metafunctions. 
        // Stage 2 is executed by deduce_target_char_impl<T> and 
        // deduce_source_char_impl<T>
        //
        // deduce_target_char<T> and deduce_source_char<T> functions combine 
        // both stages

        template < class T >
        struct deduce_target_char
        {
            typedef NDNBOOST_DEDUCED_TYPENAME stream_char_common< T >::type stage1_type;
            typedef NDNBOOST_DEDUCED_TYPENAME deduce_target_char_impl< stage1_type >::type stage2_type;

            typedef stage2_type type;
        };

        template < class T >
        struct deduce_source_char
        {
            typedef NDNBOOST_DEDUCED_TYPENAME stream_char_common< T >::type stage1_type;
            typedef NDNBOOST_DEDUCED_TYPENAME deduce_source_char_impl< stage1_type >::type stage2_type;

            typedef stage2_type type;
        };
    }

    namespace detail // extract_char_traits template
    {
        // We are attempting to get char_traits<> from T
        // template parameter. Otherwise we'll be using std::char_traits<Char>
        template < class Char, class T >
        struct extract_char_traits
                : ndnboost::false_type
        {
            typedef std::char_traits< Char > trait_t;
        };

        template < class Char, class Traits, class Alloc >
        struct extract_char_traits< Char, std::basic_string< Char, Traits, Alloc > >
            : ndnboost::true_type
        {
            typedef Traits trait_t;
        };

        template < class Char, class Traits, class Alloc>
        struct extract_char_traits< Char, ndnboost::container::basic_string< Char, Traits, Alloc > >
            : ndnboost::true_type
        {
            typedef Traits trait_t;
        };
    }

    namespace detail // array_to_pointer_decay<T>
    {
        template<class T>
        struct array_to_pointer_decay
        {
            typedef T type;
        };

        template<class T, std::size_t N>
        struct array_to_pointer_decay<T[N]>
        {
            typedef const T * type;
        };
    }

    namespace detail // is_this_float_conversion_optimized<Float, Char>
    {
        // this metafunction evaluates to true, if we have optimized comnversion 
        // from Float type to Char array. 
        // Must be in sync with lexical_stream_limited_src<Char, ...>::shl_real_type(...)
        template <typename Float, typename Char>
        struct is_this_float_conversion_optimized 
        {
            typedef ndnboost::type_traits::ice_and<
                ndnboost::is_float<Float>::value,
#if !defined(NDNBOOST_LCAST_NO_WCHAR_T) && !defined(NDNBOOST_NO_SWPRINTF) && !defined(__MINGW32__)
                ndnboost::type_traits::ice_or<
                    ndnboost::type_traits::ice_eq<sizeof(Char), sizeof(char) >::value,
                    ndnboost::is_same<Char, wchar_t>::value
                >::value
#else
                ndnboost::type_traits::ice_eq<sizeof(Char), sizeof(char) >::value
#endif
            > result_type;

            NDNBOOST_STATIC_CONSTANT(bool, value = (result_type::value) );
        };
    }
    
    namespace detail // lcast_src_length
    {
        // Return max. length of string representation of Source;
        template< class Source,         // Source type of lexical_cast.
                  class Enable = void   // helper type
                >
        struct lcast_src_length
        {
            NDNBOOST_STATIC_CONSTANT(std::size_t, value = 1);
        };

        // Helper for integral types.
        // Notes on length calculation:
        // Max length for 32bit int with grouping "\1" and thousands_sep ',':
        // "-2,1,4,7,4,8,3,6,4,7"
        //  ^                    - is_signed
        //   ^                   - 1 digit not counted by digits10
        //    ^^^^^^^^^^^^^^^^^^ - digits10 * 2
        //
        // Constant is_specialized is used instead of constant 1
        // to prevent buffer overflow in a rare case when
        // <ndnboost/limits.hpp> doesn't add missing specialization for
        // numeric_limits<T> for some integral type T.
        // When is_specialized is false, the whole expression is 0.
        template <class Source>
        struct lcast_src_length<
                    Source, NDNBOOST_DEDUCED_TYPENAME ndnboost::enable_if<ndnboost::is_integral<Source> >::type
                >
        {
#ifndef NDNBOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
            NDNBOOST_STATIC_CONSTANT(std::size_t, value =
                  std::numeric_limits<Source>::is_signed +
                  std::numeric_limits<Source>::is_specialized + /* == 1 */
                  std::numeric_limits<Source>::digits10 * 2
              );
#else
            NDNBOOST_STATIC_CONSTANT(std::size_t, value = 156);
            NDNBOOST_STATIC_ASSERT(sizeof(Source) * CHAR_BIT <= 256);
#endif
        };

#ifndef NDNBOOST_LCAST_NO_COMPILE_TIME_PRECISION
        // Helper for floating point types.
        // -1.23456789e-123456
        // ^                   sign
        //  ^                  leading digit
        //   ^                 decimal point 
        //    ^^^^^^^^         lcast_precision<Source>::value
        //            ^        "e"
        //             ^       exponent sign
        //              ^^^^^^ exponent (assumed 6 or less digits)
        // sign + leading digit + decimal point + "e" + exponent sign == 5
        template<class Source>
        struct lcast_src_length<
                Source, NDNBOOST_DEDUCED_TYPENAME ndnboost::enable_if<ndnboost::is_float<Source> >::type
            >
        {
            NDNBOOST_STATIC_ASSERT(
                    std::numeric_limits<Source>::max_exponent10 <=  999999L &&
                    std::numeric_limits<Source>::min_exponent10 >= -999999L
                );

            NDNBOOST_STATIC_CONSTANT(std::size_t, value =
                    5 + lcast_precision<Source>::value + 6
                );
        };
#endif // #ifndef NDNBOOST_LCAST_NO_COMPILE_TIME_PRECISION
    }

    namespace detail // lexical_cast_stream_traits<Source, Target>
    {
        template <class Source, class Target>
        struct lexical_cast_stream_traits {
            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::detail::array_to_pointer_decay<Source>::type src;
            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::remove_cv<src>::type            no_cv_src;
                
            typedef ndnboost::detail::deduce_source_char<no_cv_src>                           deduce_src_char_metafunc;
            typedef NDNBOOST_DEDUCED_TYPENAME deduce_src_char_metafunc::type           src_char_t;
            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::detail::deduce_target_char<Target>::type target_char_t;
                
            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::detail::widest_char<
                target_char_t, src_char_t
            >::type char_type;

#if !defined(NDNBOOST_NO_CXX11_CHAR16_T) && defined(NDNBOOST_NO_CXX11_UNICODE_LITERALS)
            NDNBOOST_STATIC_ASSERT_MSG(( !ndnboost::is_same<char16_t, src_char_t>::value
                                        && !ndnboost::is_same<char16_t, target_char_t>::value),
                "Your compiler does not have full support for char16_t" );
#endif
#if !defined(NDNBOOST_NO_CXX11_CHAR32_T) && defined(NDNBOOST_NO_CXX11_UNICODE_LITERALS)
            NDNBOOST_STATIC_ASSERT_MSG(( !ndnboost::is_same<char32_t, src_char_t>::value
                                        && !ndnboost::is_same<char32_t, target_char_t>::value),
                "Your compiler does not have full support for char32_t" );
#endif

            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::mpl::if_c<
                ndnboost::detail::extract_char_traits<char_type, Target>::value,
                NDNBOOST_DEDUCED_TYPENAME ndnboost::detail::extract_char_traits<char_type, Target>,
                NDNBOOST_DEDUCED_TYPENAME ndnboost::detail::extract_char_traits<char_type, no_cv_src>
            >::type::trait_t traits;

            typedef ndnboost::type_traits::ice_and<
                ndnboost::is_same<char, src_char_t>::value,                                  // source is not a wide character based type
                ndnboost::type_traits::ice_ne<sizeof(char), sizeof(target_char_t) >::value,  // target type is based on wide character
                ndnboost::type_traits::ice_not<
                    ndnboost::detail::is_character<no_cv_src>::value                     // single character widening is optimized
                >::value                                                                  // and does not requires stringbuffer
            >   is_string_widening_required_t;

            typedef ndnboost::type_traits::ice_not< ndnboost::type_traits::ice_or<
                ndnboost::is_integral<no_cv_src>::value,
                ndnboost::detail::is_this_float_conversion_optimized<no_cv_src, char_type >::value,
                ndnboost::detail::is_character<
                    NDNBOOST_DEDUCED_TYPENAME deduce_src_char_metafunc::stage1_type          // if we did not get character type at stage1
                >::value                                                                  // then we have no optimization for that type
            >::value >   is_source_input_not_optimized_t;

            // If we have an optimized conversion for
            // Source, we do not need to construct stringbuf.
            NDNBOOST_STATIC_CONSTANT(bool, requires_stringbuf = 
                (ndnboost::type_traits::ice_or<
                    is_string_widening_required_t::value, is_source_input_not_optimized_t::value
                >::value)
            );
            
            typedef ndnboost::detail::lcast_src_length<no_cv_src> len_t;
        };
    }

    namespace detail // '0', '-', '+', 'e', 'E' and '.' constants
    {
        template < typename Char >
        struct lcast_char_constants {
            // We check in tests assumption that static casted character is
            // equal to correctly written C++ literal: U'0' == static_cast<char32_t>('0')
            NDNBOOST_STATIC_CONSTANT(Char, zero  = static_cast<Char>('0'));
            NDNBOOST_STATIC_CONSTANT(Char, minus = static_cast<Char>('-'));
            NDNBOOST_STATIC_CONSTANT(Char, plus = static_cast<Char>('+'));
            NDNBOOST_STATIC_CONSTANT(Char, lowercase_e = static_cast<Char>('e'));
            NDNBOOST_STATIC_CONSTANT(Char, capital_e = static_cast<Char>('E'));
            NDNBOOST_STATIC_CONSTANT(Char, c_decimal_separator = static_cast<Char>('.'));
        };
    }

    namespace detail // lcast_to_unsigned
    {
        template<class T>
        inline
        NDNBOOST_DEDUCED_TYPENAME ndnboost::make_unsigned<T>::type lcast_to_unsigned(const T value) NDNBOOST_NOEXCEPT {
            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::make_unsigned<T>::type result_type;
            return value < 0 
                ? static_cast<result_type>(0u - static_cast<result_type>(value)) 
                : static_cast<result_type>(value);
        }
    }

    namespace detail // lcast_put_unsigned
    {
        template <class Traits, class T, class CharT>
        class lcast_put_unsigned: ndnboost::noncopyable {
            typedef NDNBOOST_DEDUCED_TYPENAME Traits::int_type int_type;
            NDNBOOST_DEDUCED_TYPENAME ndnboost::mpl::if_c<
                    (sizeof(int_type) > sizeof(T))
                    , int_type
                    , T
            >::type         m_value;
            CharT*          m_finish;
            CharT    const  m_czero;
            int_type const  m_zero;

        public:
            lcast_put_unsigned(const T n_param, CharT* finish) NDNBOOST_NOEXCEPT 
                : m_value(n_param), m_finish(finish)
                , m_czero(lcast_char_constants<CharT>::zero), m_zero(Traits::to_int_type(m_czero))
            {
#ifndef NDNBOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
                NDNBOOST_STATIC_ASSERT(!std::numeric_limits<T>::is_signed);
#endif
            }

            CharT* convert() {
#ifndef NDNBOOST_LEXICAL_CAST_ASSUME_C_LOCALE
                std::locale loc;
                if (loc == std::locale::classic()) {
                    return main_convert_loop();
                }

                typedef std::numpunct<CharT> numpunct;
                numpunct const& np = NDNBOOST_USE_FACET(numpunct, loc);
                std::string const grouping = np.grouping();
                std::string::size_type const grouping_size = grouping.size();

                if (!grouping_size || grouping[0] <= 0) {
                    return main_convert_loop();
                }

#ifndef NDNBOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
                // Check that ulimited group is unreachable:
                NDNBOOST_STATIC_ASSERT(std::numeric_limits<T>::digits10 < CHAR_MAX);
#endif
                CharT const thousands_sep = np.thousands_sep();
                std::string::size_type group = 0; // current group number
                char last_grp_size = grouping[0];
                char left = last_grp_size;

                do {
                    if (left == 0) {
                        ++group;
                        if (group < grouping_size) {
                            char const grp_size = grouping[group];
                            last_grp_size = (grp_size <= 0 ? static_cast<char>(CHAR_MAX) : grp_size);
                        }

                        left = last_grp_size;
                        --m_finish;
                        Traits::assign(*m_finish, thousands_sep);
                    }

                    --left;
                } while (main_convert_itaration());

                return m_finish;
#else
                return main_convert_loop();
#endif
            }

        private:
            inline bool main_convert_itaration() NDNBOOST_NOEXCEPT {
                --m_finish;
                int_type const digit = static_cast<int_type>(m_value % 10U);
                Traits::assign(*m_finish, Traits::to_char_type(m_zero + digit));
                m_value /= 10;
                return !!m_value; // supressing warnings
            }

            inline CharT* main_convert_loop() NDNBOOST_NOEXCEPT {
                while (main_convert_itaration());
                return m_finish;
            }
        };
    }

    namespace detail // lcast_ret_unsigned
    {
        template <class Traits, class T, class CharT>
        class lcast_ret_unsigned: ndnboost::noncopyable {
            bool m_multiplier_overflowed;
            T m_multiplier;
            T& m_value;
            const CharT* const m_begin;
            const CharT* m_end;
    
        public:
            lcast_ret_unsigned(T& value, const CharT* const begin, const CharT* end) NDNBOOST_NOEXCEPT
                : m_multiplier_overflowed(false), m_multiplier(1), m_value(value), m_begin(begin), m_end(end)
            {
#ifndef NDNBOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
                NDNBOOST_STATIC_ASSERT(!std::numeric_limits<T>::is_signed);

                // GCC when used with flag -std=c++0x may not have std::numeric_limits
                // specializations for __int128 and unsigned __int128 types.
                // Try compilation with -std=gnu++0x or -std=gnu++11.
                //
                // http://gcc.gnu.org/bugzilla/show_bug.cgi?id=40856
                NDNBOOST_STATIC_ASSERT_MSG(std::numeric_limits<T>::is_specialized,
                    "std::numeric_limits are not specialized for integral type passed to ndnboost::lexical_cast"
                );
#endif
            }

            inline bool convert() {
                CharT const czero = lcast_char_constants<CharT>::zero;
                --m_end;
                m_value = static_cast<T>(0);

                if (m_begin > m_end || *m_end < czero || *m_end >= czero + 10)
                    return false;
                m_value = static_cast<T>(*m_end - czero);
                --m_end;

#ifdef NDNBOOST_LEXICAL_CAST_ASSUME_C_LOCALE
                return main_convert_loop();
#else
                std::locale loc;
                if (loc == std::locale::classic()) {
                    return main_convert_loop();
                }

                typedef std::numpunct<CharT> numpunct;
                numpunct const& np = NDNBOOST_USE_FACET(numpunct, loc);
                std::string const& grouping = np.grouping();
                std::string::size_type const grouping_size = grouping.size();

                /* According to Programming languages - C++
                 * we MUST check for correct grouping
                 */
                if (!grouping_size || grouping[0] <= 0) {
                    return main_convert_loop();
                }

                unsigned char current_grouping = 0;
                CharT const thousands_sep = np.thousands_sep();
                char remained = static_cast<char>(grouping[current_grouping] - 1);

                for (;m_end >= m_begin; --m_end)
                {
                    if (remained) {
                        if (!main_convert_itaration()) {
                            return false;
                        }
                        --remained;
                    } else {
                        if ( !Traits::eq(*m_end, thousands_sep) ) //|| begin == end ) return false;
                        {
                            /*
                             * According to Programming languages - C++
                             * Digit grouping is checked. That is, the positions of discarded
                             * separators is examined for consistency with
                             * use_facet<numpunct<charT> >(loc ).grouping()
                             *
                             * BUT what if there is no separators at all and grouping()
                             * is not empty? Well, we have no extraced separators, so we
                             * won`t check them for consistency. This will allow us to
                             * work with "C" locale from other locales
                             */
                            return main_convert_loop();
                        } else {
                            if (m_begin == m_end) return false;
                            if (current_grouping < grouping_size - 1) ++current_grouping;
                            remained = grouping[current_grouping];
                        }
                    }
                } /*for*/

                return true;
#endif
            }

        private:
            // Iteration that does not care about grouping/separators and assumes that all 
            // input characters are digits
            inline bool main_convert_itaration() NDNBOOST_NOEXCEPT {
                CharT const czero = lcast_char_constants<CharT>::zero;
                T const maxv = (std::numeric_limits<T>::max)();

                m_multiplier_overflowed = m_multiplier_overflowed || (maxv/10 < m_multiplier);
                m_multiplier = static_cast<T>(m_multiplier * 10);

                T const dig_value = static_cast<T>(*m_end - czero);
                T const new_sub_value = static_cast<T>(m_multiplier * dig_value);

                // We must correctly handle situations like `000000000000000000000000000001`.
                // So we take care of overflow only if `dig_value` is not '0'.
                if (*m_end < czero || *m_end >= czero + 10  // checking for correct digit
                    || (dig_value && (                      // checking for overflow of ... 
                        m_multiplier_overflowed                             // ... multiplier
                        || static_cast<T>(maxv / dig_value) < m_multiplier  // ... subvalue
                        || static_cast<T>(maxv - new_sub_value) < m_value   // ... whole expression
                    ))
                ) return false;

                m_value = static_cast<T>(m_value + new_sub_value);
                
                return true;
            }

            bool main_convert_loop() NDNBOOST_NOEXCEPT {
                for ( ; m_end >= m_begin; --m_end) {
                    if (!main_convert_itaration()) {
                        return false;
                    }
                }
            
                return true;
            }
        };
    }

    namespace detail
    {
        template <class CharT>
        bool lc_iequal(const CharT* val, const CharT* lcase, const CharT* ucase, unsigned int len) NDNBOOST_NOEXCEPT {
            for( unsigned int i=0; i < len; ++i ) {
                if ( val[i] != lcase[i] && val[i] != ucase[i] ) return false;
            }

            return true;
        }

        /* Returns true and sets the correct value if found NaN or Inf. */
        template <class CharT, class T>
        inline bool parse_inf_nan_impl(const CharT* begin, const CharT* end, T& value
            , const CharT* lc_NAN, const CharT* lc_nan
            , const CharT* lc_INFINITY, const CharT* lc_infinity
            , const CharT opening_brace, const CharT closing_brace) NDNBOOST_NOEXCEPT
        {
            using namespace std;
            if (begin == end) return false;
            const CharT minus = lcast_char_constants<CharT>::minus;
            const CharT plus = lcast_char_constants<CharT>::plus;
            const int inifinity_size = 8; // == sizeof("infinity") - 1

            /* Parsing +/- */
            bool const has_minus = (*begin == minus);
            if (has_minus || *begin == plus) {
                ++ begin;
            }

            if (end - begin < 3) return false;
            if (lc_iequal(begin, lc_nan, lc_NAN, 3)) {
                begin += 3;
                if (end != begin) {
                    /* It is 'nan(...)' or some bad input*/
                    
                    if (end - begin < 2) return false; // bad input
                    -- end;
                    if (*begin != opening_brace || *end != closing_brace) return false; // bad input
                }

                if( !has_minus ) value = std::numeric_limits<T>::quiet_NaN();
                else value = (ndnboost::math::changesign) (std::numeric_limits<T>::quiet_NaN());
                return true;
            } else if (
                ( /* 'INF' or 'inf' */
                  end - begin == 3      // 3 == sizeof('inf') - 1
                  && lc_iequal(begin, lc_infinity, lc_INFINITY, 3)
                )
                ||
                ( /* 'INFINITY' or 'infinity' */
                  end - begin == inifinity_size
                  && lc_iequal(begin, lc_infinity, lc_INFINITY, inifinity_size)
                )
             )
            {
                if( !has_minus ) value = std::numeric_limits<T>::infinity();
                else value = (ndnboost::math::changesign) (std::numeric_limits<T>::infinity());
                return true;
            }

            return false;
        }

        template <class CharT, class T>
        bool put_inf_nan_impl(CharT* begin, CharT*& end, const T& value
                         , const CharT* lc_nan
                         , const CharT* lc_infinity) NDNBOOST_NOEXCEPT
        {
            using namespace std;
            const CharT minus = lcast_char_constants<CharT>::minus;
            if ((ndnboost::math::isnan)(value)) {
                if ((ndnboost::math::signbit)(value)) {
                    *begin = minus;
                    ++ begin;
                }

                memcpy(begin, lc_nan, 3 * sizeof(CharT));
                end = begin + 3;
                return true;
            } else if ((ndnboost::math::isinf)(value)) {
                if ((ndnboost::math::signbit)(value)) {
                    *begin = minus;
                    ++ begin;
                }

                memcpy(begin, lc_infinity, 3 * sizeof(CharT));
                end = begin + 3;
                return true;
            }

            return false;
        }


#ifndef NDNBOOST_LCAST_NO_WCHAR_T
        template <class T>
        bool parse_inf_nan(const wchar_t* begin, const wchar_t* end, T& value) NDNBOOST_NOEXCEPT {
            return parse_inf_nan_impl(begin, end, value
                               , L"NAN", L"nan"
                               , L"INFINITY", L"infinity"
                               , L'(', L')');
        }

        template <class T>
        bool put_inf_nan(wchar_t* begin, wchar_t*& end, const T& value) NDNBOOST_NOEXCEPT {
            return put_inf_nan_impl(begin, end, value, L"nan", L"infinity");
        }

#endif
#if !defined(NDNBOOST_NO_CXX11_CHAR16_T) && !defined(NDNBOOST_NO_CXX11_UNICODE_LITERALS)
        template <class T>
        bool parse_inf_nan(const char16_t* begin, const char16_t* end, T& value) NDNBOOST_NOEXCEPT {
            return parse_inf_nan_impl(begin, end, value
                               , u"NAN", u"nan"
                               , u"INFINITY", u"infinity"
                               , u'(', u')');
        }

        template <class T>
        bool put_inf_nan(char16_t* begin, char16_t*& end, const T& value) NDNBOOST_NOEXCEPT {
            return put_inf_nan_impl(begin, end, value, u"nan", u"infinity");
        }
#endif
#if !defined(NDNBOOST_NO_CXX11_CHAR32_T) && !defined(NDNBOOST_NO_CXX11_UNICODE_LITERALS)
        template <class T>
        bool parse_inf_nan(const char32_t* begin, const char32_t* end, T& value) NDNBOOST_NOEXCEPT {
            return parse_inf_nan_impl(begin, end, value
                               , U"NAN", U"nan"
                               , U"INFINITY", U"infinity"
                               , U'(', U')');
        }

        template <class T>
        bool put_inf_nan(char32_t* begin, char32_t*& end, const T& value) NDNBOOST_NOEXCEPT {
            return put_inf_nan_impl(begin, end, value, U"nan", U"infinity");
        }
#endif

        template <class CharT, class T>
        bool parse_inf_nan(const CharT* begin, const CharT* end, T& value) NDNBOOST_NOEXCEPT {
            return parse_inf_nan_impl(begin, end, value
                               , "NAN", "nan"
                               , "INFINITY", "infinity"
                               , '(', ')');
        }

        template <class CharT, class T>
        bool put_inf_nan(CharT* begin, CharT*& end, const T& value) NDNBOOST_NOEXCEPT {
            return put_inf_nan_impl(begin, end, value, "nan", "infinity");
        }
    }


    namespace detail // lcast_ret_float
    {

// Silence buggy MS warnings like C4244: '+=' : conversion from 'int' to 'unsigned short', possible loss of data 
#if defined(_MSC_VER) && (_MSC_VER == 1400) 
#  pragma warning(push) 
#  pragma warning(disable:4244) 
#endif 
        template <class T>
        struct mantissa_holder_type
        {
            /* Can not be used with this type */
        };

        template <>
        struct mantissa_holder_type<float>
        {
            typedef unsigned int type;
            typedef double       wide_result_t;
        };

        template <>
        struct mantissa_holder_type<double>
        {
#ifndef NDNBOOST_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
            typedef long double  wide_result_t;
#if defined(NDNBOOST_HAS_LONG_LONG)
            typedef ndnboost::ulong_long_type type;
#elif defined(NDNBOOST_HAS_MS_INT64)
            typedef unsigned __int64 type;
#endif
#endif
        };

        template<class Traits, class T, class CharT>
        inline bool lcast_ret_float(T& value, const CharT* begin, const CharT* const end)
        {
            value = static_cast<T>(0);
            if (begin == end) return false;
            if (parse_inf_nan(begin, end, value)) return true;

            CharT const czero = lcast_char_constants<CharT>::zero;
            CharT const minus = lcast_char_constants<CharT>::minus;
            CharT const plus = lcast_char_constants<CharT>::plus;
            CharT const capital_e = lcast_char_constants<CharT>::capital_e;
            CharT const lowercase_e = lcast_char_constants<CharT>::lowercase_e;
            
            /* Getting the plus/minus sign */
            bool const has_minus = Traits::eq(*begin, minus);
            if (has_minus || Traits::eq(*begin, plus)) {
                ++ begin;
                if (begin == end) return false;
            }

#ifndef NDNBOOST_LEXICAL_CAST_ASSUME_C_LOCALE
            std::locale loc;
            typedef std::numpunct<CharT> numpunct;
            numpunct const& np = NDNBOOST_USE_FACET(numpunct, loc);
            std::string const grouping(
                    (loc == std::locale::classic())
                    ? std::string()
                    : np.grouping()
            );
            std::string::size_type const grouping_size = grouping.size();
            CharT const thousands_sep = static_cast<CharT>(grouping_size ? np.thousands_sep() : 0);
            CharT const decimal_point = np.decimal_point();
            bool found_grouping = false;
            std::string::size_type last_grouping_pos = grouping_size - 1;
#else
            CharT const decimal_point = lcast_char_constants<CharT>::c_decimal_separator;
#endif

            bool found_decimal = false;
            bool found_number_before_exp = false;
            typedef int pow_of_10_t;
            pow_of_10_t pow_of_10 = 0;

            typedef NDNBOOST_DEDUCED_TYPENAME mantissa_holder_type<T>::type mantissa_type;
            mantissa_type mantissa=0;
            bool is_mantissa_full = false;
            char length_since_last_delim = 0;

            while (begin != end) {
                if (found_decimal) {
                    /* We allow no thousand_separators after decimal point */

                    const mantissa_type tmp_sub_value = static_cast<mantissa_type>(*begin - czero);
                    if (Traits::eq(*begin, lowercase_e) || Traits::eq(*begin, capital_e)) break;
                    if ( *begin < czero || *begin >= czero + 10 ) return false;
                    if (    is_mantissa_full
                            || ((std::numeric_limits<mantissa_type>::max)() - tmp_sub_value) / 10u  < mantissa
                            ) {
                        is_mantissa_full = true;
                        ++ begin;
                        continue;
                    }

                    -- pow_of_10;
                    mantissa = static_cast<mantissa_type>(mantissa * 10 + tmp_sub_value);

                    found_number_before_exp = true;
                } else {

                    if (*begin >= czero && *begin < czero + 10) {

                        /* Checking for mantissa overflow. If overflow will
                         * occur, them we only increase multiplyer
                         */
                        const mantissa_type tmp_sub_value = static_cast<mantissa_type>(*begin - czero);
                        if(     is_mantissa_full
                                || ((std::numeric_limits<mantissa_type>::max)() - tmp_sub_value) / 10u  < mantissa
                            )
                        {
                            is_mantissa_full = true;
                            ++ pow_of_10;
                        } else {
                            mantissa = static_cast<mantissa_type>(mantissa * 10 + tmp_sub_value);
                        }

                        found_number_before_exp = true;
                        ++ length_since_last_delim;
                    } else if (Traits::eq(*begin, decimal_point) || Traits::eq(*begin, lowercase_e) || Traits::eq(*begin, capital_e)) {
#ifndef NDNBOOST_LEXICAL_CAST_ASSUME_C_LOCALE
                        /* If ( we need to check grouping
                         *      and (   grouping missmatches
                         *              or grouping position is incorrect
                         *              or we are using the grouping position 0 twice
                         *           )
                         *    ) then return error
                         */
                        if( grouping_size && found_grouping
                            && (
                                   length_since_last_delim != grouping[0]
                                   || last_grouping_pos>1
                                   || (last_grouping_pos==0 && grouping_size>1)
                                )
                           ) return false;
#endif

                        if (Traits::eq(*begin, decimal_point)) {
                            ++ begin;
                            found_decimal = true;
                            if (!found_number_before_exp && begin==end) return false;
                            continue;
                        } else {
                            if (!found_number_before_exp) return false;
                            break;
                        }
                    }
#ifndef NDNBOOST_LEXICAL_CAST_ASSUME_C_LOCALE
                    else if (grouping_size && Traits::eq(*begin, thousands_sep)){
                        if(found_grouping)
                        {
                            /* It is not he first time, when we find thousands separator,
                             * so we need to chek, is the distance between two groupings
                             * equal to grouping[last_grouping_pos] */

                            if (length_since_last_delim != grouping[last_grouping_pos] )
                            {
                                if (!last_grouping_pos) return false;
                                else
                                {
                                    -- last_grouping_pos;
                                    if (length_since_last_delim != grouping[last_grouping_pos]) return false;
                                }
                            } else
                                /* We are calling the grouping[0] twice, when grouping size is more than 1 */
                                if (grouping_size>1u && last_grouping_pos+1<grouping_size) return false;

                        } else {
                            /* Delimiter at the begining ',000' */
                            if (!length_since_last_delim) return false;

                            found_grouping = true;
                            if (length_since_last_delim > grouping[last_grouping_pos] ) return false;
                        }

                        length_since_last_delim = 0;
                        ++ begin;

                        /* Delimiter at the end '100,' */
                        if (begin == end) return false;
                        continue;
                    }
#endif
                    else return false;
                }

                ++begin;
            }

            // Exponent found
            if (begin != end && (Traits::eq(*begin, lowercase_e) || Traits::eq(*begin, capital_e))) {
                ++ begin;
                if (begin == end) return false;

                bool const exp_has_minus = Traits::eq(*begin, minus);
                if (exp_has_minus || Traits::eq(*begin, plus)) {
                    ++ begin;
                    if (begin == end) return false;
                }

                pow_of_10_t exp_pow_of_10 = 0;
                while (begin != end) {
                    pow_of_10_t const sub_value = *begin - czero;

                    if ( *begin < czero || *begin >= czero + 10
                         || ((std::numeric_limits<pow_of_10_t>::max)() - sub_value) / 10 < exp_pow_of_10)
                        return false;

                    exp_pow_of_10 *= 10;
                    exp_pow_of_10 += sub_value;
                    ++ begin;
                };

                if (exp_has_minus) {
                    if ((std::numeric_limits<pow_of_10_t>::min)() + exp_pow_of_10 > pow_of_10)
                        return false;   // failed overflow check
                    pow_of_10 -= exp_pow_of_10;
                } else {
                    if ((std::numeric_limits<pow_of_10_t>::max)() - exp_pow_of_10 < pow_of_10)
                        return false;   // failed overflow check
                    pow_of_10 += exp_pow_of_10;
                }
            }

            /* We need a more accurate algorithm... We can not use current algorithm
             * with long doubles (and with doubles if sizeof(double)==sizeof(long double)).
             */
            typedef NDNBOOST_DEDUCED_TYPENAME mantissa_holder_type<T>::wide_result_t wide_result_t;
            const wide_result_t result = std::pow(static_cast<wide_result_t>(10.0), pow_of_10) * mantissa;
            value = static_cast<T>( has_minus ? (ndnboost::math::changesign)(result) : result);

            return !((ndnboost::math::isinf)(value) || (ndnboost::math::isnan)(value));
        }
// Unsilence buggy MS warnings like C4244: '+=' : conversion from 'int' to 'unsigned short', possible loss of data 
#if defined(_MSC_VER) && (_MSC_VER == 1400) 
#  pragma warning(pop) 
#endif 
    }

    namespace detail // basic_unlockedbuf
    {
        // acts as a stream buffer which wraps around a pair of pointers
        // and gives acces to internals
        template <class BufferType, class CharT>
        class basic_unlockedbuf : public basic_pointerbuf<CharT, BufferType> {
        public:
           typedef basic_pointerbuf<CharT, BufferType> base_type;
           typedef NDNBOOST_DEDUCED_TYPENAME base_type::streamsize streamsize;

#ifndef NDNBOOST_NO_USING_TEMPLATE
            using base_type::pptr;
            using base_type::pbase;
            using base_type::setbuf;
#else
            charT* pptr() const { return base_type::pptr(); }
            charT* pbase() const { return base_type::pbase(); }
            BufferType* setbuf(char_type* s, streamsize n) { return base_type::setbuf(s, n); }
#endif
        };
    }

    namespace detail
    {
        struct do_not_construct_out_stream_t{};
        
        template <class CharT, class Traits>
        struct out_stream_helper_trait {
#if defined(NDNBOOST_NO_STRINGSTREAM)
            typedef std::ostrstream                                 out_stream_t;
            typedef void                                            buffer_t;
#elif defined(NDNBOOST_NO_STD_LOCALE)
            typedef std::ostringstream                              out_stream_t;
            typedef basic_unlockedbuf<std::streambuf, char>         buffer_t;
#else
            typedef std::basic_ostringstream<CharT, Traits> 
                out_stream_t;
            typedef basic_unlockedbuf<std::basic_streambuf<CharT, Traits>, CharT>  
                buffer_t;
#endif
        };   
    }

    namespace detail // optimized stream wrappers
    {
        template< class CharT // a result of widest_char transformation
                , class Traits
                , bool RequiresStringbuffer
                , std::size_t CharacterBufferSize
                >
        class lexical_istream_limited_src: ndnboost::noncopyable {
            typedef NDNBOOST_DEDUCED_TYPENAME out_stream_helper_trait<CharT, Traits>::buffer_t
                buffer_t;

            typedef NDNBOOST_DEDUCED_TYPENAME out_stream_helper_trait<CharT, Traits>::out_stream_t
                out_stream_t;
    
            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::mpl::if_c<
                RequiresStringbuffer,
                out_stream_t,
                do_not_construct_out_stream_t
            >::type deduced_out_stream_t;

            // A string representation of Source is written to `buffer`.
            deduced_out_stream_t out_stream;
            CharT   buffer[CharacterBufferSize];

            // After the `operator <<`  finishes, `[start, finish)` is
            // the range to output by `operator >>` 
            const CharT*  start;
            const CharT*  finish;

        public:
            lexical_istream_limited_src() NDNBOOST_NOEXCEPT
              : start(buffer)
              , finish(buffer + CharacterBufferSize)
            {}
    
            const CharT* cbegin() const NDNBOOST_NOEXCEPT {
                return start;
            }

            const CharT* cend() const NDNBOOST_NOEXCEPT {
                return finish;
            }

        private:
            // Undefined:
            lexical_istream_limited_src(lexical_istream_limited_src const&);
            void operator=(lexical_istream_limited_src const&);

/************************************ HELPER FUNCTIONS FOR OPERATORS << ( ... ) ********************************/
            bool shl_char(CharT ch) NDNBOOST_NOEXCEPT {
                Traits::assign(buffer[0], ch);
                finish = start + 1;
                return true;
            }

#ifndef NDNBOOST_LCAST_NO_WCHAR_T
            template <class T>
            bool shl_char(T ch) {
                NDNBOOST_STATIC_ASSERT_MSG(( sizeof(T) <= sizeof(CharT)) ,
                    "ndnboost::lexical_cast does not support narrowing of char types."
                    "Use ndnboost::locale instead" );
#ifndef NDNBOOST_LEXICAL_CAST_ASSUME_C_LOCALE
                std::locale loc;
                CharT const w = NDNBOOST_USE_FACET(std::ctype<CharT>, loc).widen(ch);
#else
                CharT const w = static_cast<CharT>(ch);
#endif
                Traits::assign(buffer[0], w);
                finish = start + 1;
                return true;
            }
#endif

            bool shl_char_array(CharT const* str) NDNBOOST_NOEXCEPT {
                start = str;
                finish = start + Traits::length(str);
                return true;
            }

            template <class T>
            bool shl_char_array(T const* str) {
                NDNBOOST_STATIC_ASSERT_MSG(( sizeof(T) <= sizeof(CharT)),
                    "ndnboost::lexical_cast does not support narrowing of char types."
                    "Use ndnboost::locale instead" );
                return shl_input_streamable(str);
            }
            
            bool shl_char_array_limited(CharT const* str, std::size_t max_size) NDNBOOST_NOEXCEPT {
                start = str;
                finish = std::find(start, start + max_size, Traits::to_char_type(0));
                return true;
            }

            template<typename InputStreamable>
            bool shl_input_streamable(InputStreamable& input) {
#if defined(NDNBOOST_NO_STRINGSTREAM) || defined(NDNBOOST_NO_STD_LOCALE)
                // If you have compilation error at this point, than your STL library
                // does not support such conversions. Try updating it.
                NDNBOOST_STATIC_ASSERT((ndnboost::is_same<char, CharT>::value));
#endif

#ifndef NDNBOOST_NO_EXCEPTIONS
                out_stream.exceptions(std::ios::badbit);
                try {
#endif
                bool const result = !(out_stream << input).fail();
                const buffer_t* const p = static_cast<buffer_t*>(
                    static_cast<std::basic_streambuf<CharT, Traits>*>(out_stream.rdbuf())
                );
                start = p->pbase();
                finish = p->pptr();
                return result;
#ifndef NDNBOOST_NO_EXCEPTIONS
                } catch (const ::std::ios_base::failure& /*f*/) {
                    return false;
                }
#endif
            }

            template <class T>
            inline bool shl_unsigned(const T n) {
                CharT* tmp_finish = buffer + CharacterBufferSize;
                start = lcast_put_unsigned<Traits, T, CharT>(n, tmp_finish).convert();
                finish = tmp_finish;
                return true;
            }

            template <class T>
            inline bool shl_signed(const T n) {
                CharT* tmp_finish = buffer + CharacterBufferSize;
                typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::make_unsigned<T>::type utype;
                CharT* tmp_start = lcast_put_unsigned<Traits, utype, CharT>(lcast_to_unsigned(n), tmp_finish).convert();
                if (n < 0) {
                    --tmp_start;
                    CharT const minus = lcast_char_constants<CharT>::minus;
                    Traits::assign(*tmp_start, minus);
                }
                start = tmp_start;
                finish = tmp_finish;
                return true;
            }

            template <class T, class SomeCharT>
            bool shl_real_type(const T& val, SomeCharT* /*begin*/) {
                lcast_set_precision(out_stream, &val);
                return shl_input_streamable(val);
            }

            bool shl_real_type(float val, char* begin) {
                using namespace std;
                const double val_as_double = val;
                finish = start +
#if defined(_MSC_VER) && (_MSC_VER >= 1400) && !defined(__SGI_STL_PORT) && !defined(_STLPORT_VERSION)
                    sprintf_s(begin, CharacterBufferSize,
#else
                    sprintf(begin, 
#endif
                    "%.*g", static_cast<int>(ndnboost::detail::lcast_get_precision<float>()), val_as_double);
                return finish > start;
            }

            bool shl_real_type(double val, char* begin) {
                using namespace std;
                finish = start +
#if defined(_MSC_VER) && (_MSC_VER >= 1400) && !defined(__SGI_STL_PORT) && !defined(_STLPORT_VERSION)
                    sprintf_s(begin, CharacterBufferSize,
#else
                    sprintf(begin, 
#endif
                    "%.*g", static_cast<int>(ndnboost::detail::lcast_get_precision<double>()), val);
                return finish > start;
            }

#ifndef __MINGW32__
            bool shl_real_type(long double val, char* begin) {
                using namespace std;
                finish = start +
#if defined(_MSC_VER) && (_MSC_VER >= 1400) && !defined(__SGI_STL_PORT) && !defined(_STLPORT_VERSION)
                    sprintf_s(begin, CharacterBufferSize,
#else
                    sprintf(begin, 
#endif
                    "%.*Lg", static_cast<int>(ndnboost::detail::lcast_get_precision<long double>()), val );
                return finish > start;
            }
#endif


#if !defined(NDNBOOST_LCAST_NO_WCHAR_T) && !defined(NDNBOOST_NO_SWPRINTF) && !defined(__MINGW32__)
            bool shl_real_type(float val, wchar_t* begin) {
                using namespace std;
                const double val_as_double = val;
                finish = start + swprintf(begin, CharacterBufferSize,
                                       L"%.*g",
                                       static_cast<int>(ndnboost::detail::lcast_get_precision<float >()),
                                       val_as_double );
                return finish > start;
            }

            bool shl_real_type(double val, wchar_t* begin) {
                using namespace std;
                finish = start + swprintf(begin, CharacterBufferSize,
                                          L"%.*g", static_cast<int>(ndnboost::detail::lcast_get_precision<double >()), val );
                return finish > start;
            }

            bool shl_real_type(long double val, wchar_t* begin) {
                using namespace std;
                finish = start + swprintf(begin, CharacterBufferSize,
                                          L"%.*Lg", static_cast<int>(ndnboost::detail::lcast_get_precision<long double >()), val );
                return finish > start;
            }
#endif
            template <class T>
            bool shl_real(T val) {
                CharT* tmp_finish = buffer + CharacterBufferSize;
                if (put_inf_nan(buffer, tmp_finish, val)) {
                    finish = tmp_finish;
                    return true;
                }

                return shl_real_type(val, static_cast<CharT*>(buffer));
            }

/************************************ OPERATORS << ( ... ) ********************************/
        public:
            template<class Alloc>
            bool operator<<(std::basic_string<CharT,Traits,Alloc> const& str) NDNBOOST_NOEXCEPT {
                start = str.data();
                finish = start + str.length();
                return true;
            }

            template<class Alloc>
            bool operator<<(ndnboost::container::basic_string<CharT,Traits,Alloc> const& str) NDNBOOST_NOEXCEPT {
                start = str.data();
                finish = start + str.length();
                return true;
            }

            bool operator<<(bool value) NDNBOOST_NOEXCEPT {
                CharT const czero = lcast_char_constants<CharT>::zero;
                Traits::assign(buffer[0], Traits::to_char_type(czero + value));
                finish = start + 1;
                return true;
            }

            template <class C>
            NDNBOOST_DEDUCED_TYPENAME ndnboost::disable_if<ndnboost::is_const<C>, bool>::type 
            operator<<(const iterator_range<C*>& rng) NDNBOOST_NOEXCEPT {
                return (*this) << iterator_range<const C*>(rng.begin(), rng.end());
            }
            
            bool operator<<(const iterator_range<const CharT*>& rng) NDNBOOST_NOEXCEPT {
                start = rng.begin();
                finish = rng.end();
                return true; 
            }

            bool operator<<(const iterator_range<const signed char*>& rng) NDNBOOST_NOEXCEPT {
                return (*this) << iterator_range<const char*>(
                    reinterpret_cast<const char*>(rng.begin()),
                    reinterpret_cast<const char*>(rng.end())
                );
            }

            bool operator<<(const iterator_range<const unsigned char*>& rng) NDNBOOST_NOEXCEPT {
                return (*this) << iterator_range<const char*>(
                    reinterpret_cast<const char*>(rng.begin()),
                    reinterpret_cast<const char*>(rng.end())
                );
            }

            bool operator<<(char ch)                    { return shl_char(ch); }
            bool operator<<(unsigned char ch)           { return ((*this) << static_cast<char>(ch)); }
            bool operator<<(signed char ch)             { return ((*this) << static_cast<char>(ch)); }
#if !defined(NDNBOOST_LCAST_NO_WCHAR_T)
            bool operator<<(wchar_t const* str)         { return shl_char_array(str); }
            bool operator<<(wchar_t * str)              { return shl_char_array(str); }
#ifndef NDNBOOST_NO_INTRINSIC_WCHAR_T
            bool operator<<(wchar_t ch)                 { return shl_char(ch); }
#endif
#endif
#if !defined(NDNBOOST_NO_CXX11_CHAR16_T) && !defined(NDNBOOST_NO_CXX11_UNICODE_LITERALS)
            bool operator<<(char16_t ch)                { return shl_char(ch); }
            bool operator<<(char16_t * str)             { return shl_char_array(str); }
            bool operator<<(char16_t const * str)       { return shl_char_array(str); }
#endif
#if !defined(NDNBOOST_NO_CXX11_CHAR32_T) && !defined(NDNBOOST_NO_CXX11_UNICODE_LITERALS)
            bool operator<<(char32_t ch)                { return shl_char(ch); }
            bool operator<<(char32_t * str)             { return shl_char_array(str); }
            bool operator<<(char32_t const * str)       { return shl_char_array(str); }
#endif
            bool operator<<(unsigned char const* ch)    { return ((*this) << reinterpret_cast<char const*>(ch)); }
            bool operator<<(unsigned char * ch)         { return ((*this) << reinterpret_cast<char *>(ch)); }
            bool operator<<(signed char const* ch)      { return ((*this) << reinterpret_cast<char const*>(ch)); }
            bool operator<<(signed char * ch)           { return ((*this) << reinterpret_cast<char *>(ch)); }
            bool operator<<(char const* str)            { return shl_char_array(str); }
            bool operator<<(char* str)                  { return shl_char_array(str); }
            bool operator<<(short n)                    { return shl_signed(n); }
            bool operator<<(int n)                      { return shl_signed(n); }
            bool operator<<(long n)                     { return shl_signed(n); }
            bool operator<<(unsigned short n)           { return shl_unsigned(n); }
            bool operator<<(unsigned int n)             { return shl_unsigned(n); }
            bool operator<<(unsigned long n)            { return shl_unsigned(n); }

#if defined(NDNBOOST_HAS_LONG_LONG)
            bool operator<<(ndnboost::ulong_long_type n)   { return shl_unsigned(n); }
            bool operator<<(ndnboost::long_long_type n)    { return shl_signed(n); }
#elif defined(NDNBOOST_HAS_MS_INT64)
            bool operator<<(unsigned __int64 n)         { return shl_unsigned(n); }
            bool operator<<(         __int64 n)         { return shl_signed(n); }
#endif

#ifdef NDNBOOST_HAS_INT128
            bool operator<<(const ndnboost::uint128_type& n)   { return shl_unsigned(n); }
            bool operator<<(const ndnboost::int128_type& n)    { return shl_signed(n); }
#endif
            bool operator<<(float val)                  { return shl_real(val); }
            bool operator<<(double val)                 { return shl_real(val); }
            bool operator<<(long double val)            {
#ifndef __MINGW32__
                return shl_real(val);
#else
                return shl_real(static_cast<double>(val));
#endif
            }
            
            // Adding constness to characters. Constness does not change layout
            template <class C, std::size_t N>
            NDNBOOST_DEDUCED_TYPENAME ndnboost::disable_if<ndnboost::is_const<C>, bool>::type
            operator<<(ndnboost::array<C, N> const& input) NDNBOOST_NOEXCEPT { 
                NDNBOOST_STATIC_ASSERT_MSG(
                    (sizeof(ndnboost::array<const C, N>) == sizeof(ndnboost::array<C, N>)),
                    "ndnboost::array<C, N> and ndnboost::array<const C, N> must have exactly the same layout."
                );
                return ((*this) << reinterpret_cast<ndnboost::array<const C, N> const& >(input)); 
            }

            template <std::size_t N>
            bool operator<<(ndnboost::array<const CharT, N> const& input) NDNBOOST_NOEXCEPT { 
                return shl_char_array_limited(input.begin(), N); 
            }

            template <std::size_t N>
            bool operator<<(ndnboost::array<const unsigned char, N> const& input) NDNBOOST_NOEXCEPT { 
                return ((*this) << reinterpret_cast<ndnboost::array<const char, N> const& >(input)); 
            }

            template <std::size_t N>
            bool operator<<(ndnboost::array<const signed char, N> const& input) NDNBOOST_NOEXCEPT { 
                return ((*this) << reinterpret_cast<ndnboost::array<const char, N> const& >(input)); 
            }
 
#ifndef NDNBOOST_NO_CXX11_HDR_ARRAY
            // Making a Boost.Array from std::array
            template <class C, std::size_t N>
            bool operator<<(std::array<C, N> const& input) NDNBOOST_NOEXCEPT { 
                NDNBOOST_STATIC_ASSERT_MSG(
                    (sizeof(std::array<C, N>) == sizeof(ndnboost::array<C, N>)),
                    "std::array and ndnboost::array must have exactly the same layout. "
                    "Bug in implementation of std::array or ndnboost::array."
                );
                return ((*this) << reinterpret_cast<ndnboost::array<C, N> const& >(input)); 
            }
#endif
            template <class InStreamable>
            bool operator<<(const InStreamable& input)  { return shl_input_streamable(input); }
        };


        template <class CharT, class Traits>
        class lexical_ostream_limited_src: ndnboost::noncopyable {
            //`[start, finish)` is the range to output by `operator >>` 
            const CharT*        start;
            const CharT* const  finish;

        public:
            lexical_ostream_limited_src(const CharT* begin, const CharT* end) NDNBOOST_NOEXCEPT
              : start(begin)
              , finish(end)
            {}

/************************************ HELPER FUNCTIONS FOR OPERATORS >> ( ... ) ********************************/
        private:
            template <typename Type>
            bool shr_unsigned(Type& output) {
                if (start == finish) return false;
                CharT const minus = lcast_char_constants<CharT>::minus;
                CharT const plus = lcast_char_constants<CharT>::plus;
                bool const has_minus = Traits::eq(minus, *start);

                /* We won`t use `start' any more, so no need in decrementing it after */
                if (has_minus || Traits::eq(plus, *start)) {
                    ++start;
                }

                bool const succeed = lcast_ret_unsigned<Traits, Type, CharT>(output, start, finish).convert();

                if (has_minus) {
                    output = static_cast<Type>(0u - output);
                }

                return succeed;
            }

            template <typename Type>
            bool shr_signed(Type& output) {
                if (start == finish) return false;
                CharT const minus = lcast_char_constants<CharT>::minus;
                CharT const plus = lcast_char_constants<CharT>::plus;
                typedef NDNBOOST_DEDUCED_TYPENAME make_unsigned<Type>::type utype;
                utype out_tmp = 0;
                bool const has_minus = Traits::eq(minus, *start);

                /* We won`t use `start' any more, so no need in decrementing it after */
                if (has_minus || Traits::eq(plus, *start)) {
                    ++start;
                }

                bool succeed = lcast_ret_unsigned<Traits, utype, CharT>(out_tmp, start, finish).convert();
                if (has_minus) {
                    utype const comp_val = (static_cast<utype>(1) << std::numeric_limits<Type>::digits);
                    succeed = succeed && out_tmp<=comp_val;
                    output = static_cast<Type>(0u - out_tmp);
                } else {
                    utype const comp_val = static_cast<utype>((std::numeric_limits<Type>::max)());
                    succeed = succeed && out_tmp<=comp_val;
                    output = static_cast<Type>(out_tmp);
                }
                return succeed;
            }

            template<typename InputStreamable>
            bool shr_using_base_class(InputStreamable& output)
            {
                NDNBOOST_STATIC_ASSERT_MSG(
                    (!ndnboost::is_pointer<InputStreamable>::value),
                    "ndnboost::lexical_cast can not convert to pointers"
                );

#if defined(NDNBOOST_NO_STRINGSTREAM) || defined(NDNBOOST_NO_STD_LOCALE)
                NDNBOOST_STATIC_ASSERT_MSG((ndnboost::is_same<char, CharT>::value),
                    "ndnboost::lexical_cast can not convert, because your STL library does not "
                    "support such conversions. Try updating it."
                );
#endif
                typedef NDNBOOST_DEDUCED_TYPENAME out_stream_helper_trait<CharT, Traits>::buffer_t
                    buffer_t;

#if defined(NDNBOOST_NO_STRINGSTREAM)
                std::istrstream stream(start, finish - start);
#else

                buffer_t buf;
                // Usually `istream` and `basic_istream` do not modify 
                // content of buffer; `buffer_t` assures that this is true
                buf.setbuf(const_cast<CharT*>(start), finish - start);
#if defined(NDNBOOST_NO_STD_LOCALE)
                std::istream stream(&buf);
#else
                std::basic_istream<CharT, Traits> stream(&buf);
#endif // NDNBOOST_NO_STD_LOCALE
#endif // NDNBOOST_NO_STRINGSTREAM

#ifndef NDNBOOST_NO_EXCEPTIONS
                stream.exceptions(std::ios::badbit);
                try {
#endif
                stream.unsetf(std::ios::skipws);
                lcast_set_precision(stream, static_cast<InputStreamable*>(0));

                return (stream >> output) 
                    && (stream.get() == Traits::eof());

#ifndef NDNBOOST_NO_EXCEPTIONS
                } catch (const ::std::ios_base::failure& /*f*/) {
                    return false;
                }
#endif
            }

            template<class T>
            inline bool shr_xchar(T& output) NDNBOOST_NOEXCEPT {
                NDNBOOST_STATIC_ASSERT_MSG(( sizeof(CharT) == sizeof(T) ),
                    "ndnboost::lexical_cast does not support narrowing of character types."
                    "Use ndnboost::locale instead" );
                bool const ok = (finish - start == 1);
                if (ok) {
                    CharT out;
                    Traits::assign(out, *start);
                    output = static_cast<T>(out);
                }
                return ok;
            }

            template <std::size_t N, class ArrayT>
            bool shr_std_array(ArrayT& output) NDNBOOST_NOEXCEPT {
                using namespace std;
                const std::size_t size = static_cast<std::size_t>(finish - start);
                if (size > N - 1) { // `-1` because we need to store \0 at the end 
                    return false;
                }

                memcpy(&output[0], start, size * sizeof(CharT));
                output[size] = Traits::to_char_type(0);
                return true;
            }

/************************************ OPERATORS >> ( ... ) ********************************/
        public:
            bool operator>>(unsigned short& output)             { return shr_unsigned(output); }
            bool operator>>(unsigned int& output)               { return shr_unsigned(output); }
            bool operator>>(unsigned long int& output)          { return shr_unsigned(output); }
            bool operator>>(short& output)                      { return shr_signed(output); }
            bool operator>>(int& output)                        { return shr_signed(output); }
            bool operator>>(long int& output)                   { return shr_signed(output); }
#if defined(NDNBOOST_HAS_LONG_LONG)
            bool operator>>(ndnboost::ulong_long_type& output)     { return shr_unsigned(output); }
            bool operator>>(ndnboost::long_long_type& output)      { return shr_signed(output); }
#elif defined(NDNBOOST_HAS_MS_INT64)
            bool operator>>(unsigned __int64& output)           { return shr_unsigned(output); }
            bool operator>>(__int64& output)                    { return shr_signed(output); }
#endif

#ifdef NDNBOOST_HAS_INT128
            bool operator>>(ndnboost::uint128_type& output)        { return shr_unsigned(output); }
            bool operator>>(ndnboost::int128_type& output)         { return shr_signed(output); }
#endif

            bool operator>>(char& output)                       { return shr_xchar(output); }
            bool operator>>(unsigned char& output)              { return shr_xchar(output); }
            bool operator>>(signed char& output)                { return shr_xchar(output); }
#if !defined(NDNBOOST_LCAST_NO_WCHAR_T) && !defined(NDNBOOST_NO_INTRINSIC_WCHAR_T)
            bool operator>>(wchar_t& output)                    { return shr_xchar(output); }
#endif
#if !defined(NDNBOOST_NO_CXX11_CHAR16_T) && !defined(NDNBOOST_NO_CXX11_UNICODE_LITERALS)
            bool operator>>(char16_t& output)                   { return shr_xchar(output); }
#endif
#if !defined(NDNBOOST_NO_CXX11_CHAR32_T) && !defined(NDNBOOST_NO_CXX11_UNICODE_LITERALS)
            bool operator>>(char32_t& output)                   { return shr_xchar(output); }
#endif
            template<class Alloc>
            bool operator>>(std::basic_string<CharT,Traits,Alloc>& str) { 
                str.assign(start, finish); return true; 
            }

            template<class Alloc>
            bool operator>>(ndnboost::container::basic_string<CharT,Traits,Alloc>& str) { 
                str.assign(start, finish); return true; 
            }

            template <std::size_t N>
            bool operator>>(ndnboost::array<CharT, N>& output) NDNBOOST_NOEXCEPT { 
                return shr_std_array<N>(output); 
            }

            template <std::size_t N>
            bool operator>>(ndnboost::array<unsigned char, N>& output) NDNBOOST_NOEXCEPT { 
                return ((*this) >> reinterpret_cast<ndnboost::array<char, N>& >(output)); 
            }

            template <std::size_t N>
            bool operator>>(ndnboost::array<signed char, N>& output) NDNBOOST_NOEXCEPT { 
                return ((*this) >> reinterpret_cast<ndnboost::array<char, N>& >(output)); 
            }
 
#ifndef NDNBOOST_NO_CXX11_HDR_ARRAY
            template <class C, std::size_t N>
            bool operator>>(std::array<C, N>& output) NDNBOOST_NOEXCEPT { 
                NDNBOOST_STATIC_ASSERT_MSG(
                    (sizeof(ndnboost::array<C, N>) == sizeof(ndnboost::array<C, N>)),
                    "std::array<C, N> and ndnboost::array<C, N> must have exactly the same layout."
                );
                return ((*this) >> reinterpret_cast<ndnboost::array<C, N>& >(output));
            }
#endif

            bool operator>>(bool& output) NDNBOOST_NOEXCEPT {
                output = false; // Suppress warning about uninitalized variable

                if (start == finish) return false;
                CharT const zero = lcast_char_constants<CharT>::zero;
                CharT const plus = lcast_char_constants<CharT>::plus;
                CharT const minus = lcast_char_constants<CharT>::minus;

                const CharT* const dec_finish = finish - 1;
                output = Traits::eq(*dec_finish, zero + 1);
                if (!output && !Traits::eq(*dec_finish, zero)) {
                    return false; // Does not ends on '0' or '1'
                }

                if (start == dec_finish) return true;

                // We may have sign at the beginning
                if (Traits::eq(plus, *start) || (Traits::eq(minus, *start) && !output)) {
                    ++ start;
                }

                // Skipping zeros
                while (start != dec_finish) {
                    if (!Traits::eq(zero, *start)) {
                        return false; // Not a zero => error
                    }

                    ++ start;
                }

                return true;
            }

            bool operator>>(float& output) { return lcast_ret_float<Traits>(output,start,finish); }

        private:
            // Not optimised converter
            template <class T>
            bool float_types_converter_internal(T& output, int /*tag*/) {
                if (parse_inf_nan(start, finish, output)) return true;
                bool const return_value = shr_using_base_class(output);

                /* Some compilers and libraries successfully
                 * parse 'inf', 'INFINITY', '1.0E', '1.0E-'...
                 * We are trying to provide a unified behaviour,
                 * so we just forbid such conversions (as some
                 * of the most popular compilers/libraries do)
                 * */
                CharT const minus = lcast_char_constants<CharT>::minus;
                CharT const plus = lcast_char_constants<CharT>::plus;
                CharT const capital_e = lcast_char_constants<CharT>::capital_e;
                CharT const lowercase_e = lcast_char_constants<CharT>::lowercase_e;
                if ( return_value &&
                     (
                        Traits::eq(*(finish-1), lowercase_e)                   // 1.0e
                        || Traits::eq(*(finish-1), capital_e)                  // 1.0E
                        || Traits::eq(*(finish-1), minus)                      // 1.0e- or 1.0E-
                        || Traits::eq(*(finish-1), plus)                       // 1.0e+ or 1.0E+
                     )
                ) return false;

                return return_value;
            }

            // Optimised converter
            bool float_types_converter_internal(double& output, char /*tag*/) {
                return lcast_ret_float<Traits>(output, start, finish);
            }
        public:

            bool operator>>(double& output) {
                /*
                 * Some compilers implement long double as double. In that case these types have
                 * same size, same precision, same max and min values... And it means,
                 * that current implementation of lcast_ret_float cannot be used for type
                 * double, because it will give a big precision loss.
                 * */
                ndnboost::mpl::if_c<
#if (defined(NDNBOOST_HAS_LONG_LONG) || defined(NDNBOOST_HAS_MS_INT64)) && !defined(NDNBOOST_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS)
                    ndnboost::type_traits::ice_eq< sizeof(double), sizeof(long double) >::value,
#else
                     1,
#endif
                    int,
                    char
                >::type tag = 0;

                return float_types_converter_internal(output, tag);
            }

            bool operator>>(long double& output) {
                int tag = 0;
                return float_types_converter_internal(output, tag);
            }

            // Generic istream-based algorithm.
            // lcast_streambuf_for_target<InputStreamable>::value is true.
            template <typename InputStreamable>
            bool operator>>(InputStreamable& output) { 
                return shr_using_base_class(output); 
            }
        };
    }

    namespace detail
    {
        template<typename T>
        struct is_stdstring
            : ndnboost::false_type
        {};

        template<typename CharT, typename Traits, typename Alloc>
        struct is_stdstring< std::basic_string<CharT, Traits, Alloc> >
            : ndnboost::true_type
        {};

        template<typename CharT, typename Traits, typename Alloc>
        struct is_stdstring< ndnboost::container::basic_string<CharT, Traits, Alloc> >
            : ndnboost::true_type
        {};

        template<typename Target, typename Source>
        struct is_arithmetic_and_not_xchars
        {
            NDNBOOST_STATIC_CONSTANT(bool, value = (
                ndnboost::type_traits::ice_and<
                    ndnboost::type_traits::ice_not<
                        ndnboost::detail::is_character<Target>::value
                    >::value,
                    ndnboost::type_traits::ice_not<
                        ndnboost::detail::is_character<Source>::value
                    >::value,
                    ndnboost::is_arithmetic<Source>::value,
                    ndnboost::is_arithmetic<Target>::value       
                >::value
            ));
        };

        /*
         * is_xchar_to_xchar<Target, Source>::value is true, 
         * Target and Souce are char types of the same size 1 (char, signed char, unsigned char).
         */
        template<typename Target, typename Source>
        struct is_xchar_to_xchar 
        {
            NDNBOOST_STATIC_CONSTANT(bool, value = (
                ndnboost::type_traits::ice_and<
                     ndnboost::type_traits::ice_eq<sizeof(Source), sizeof(Target)>::value,
                     ndnboost::type_traits::ice_eq<sizeof(Source), sizeof(char)>::value,
                     ndnboost::detail::is_character<Target>::value,
                     ndnboost::detail::is_character<Source>::value
                >::value
            ));
        };

        template<typename Target, typename Source>
        struct is_char_array_to_stdstring
            : ndnboost::false_type
        {};

        template<typename CharT, typename Traits, typename Alloc>
        struct is_char_array_to_stdstring< std::basic_string<CharT, Traits, Alloc>, CharT* >
            : ndnboost::true_type
        {};

        template<typename CharT, typename Traits, typename Alloc>
        struct is_char_array_to_stdstring< std::basic_string<CharT, Traits, Alloc>, const CharT* >
            : ndnboost::true_type
        {};

        template<typename CharT, typename Traits, typename Alloc>
        struct is_char_array_to_stdstring< ndnboost::container::basic_string<CharT, Traits, Alloc>, CharT* >
            : ndnboost::true_type
        {};

        template<typename CharT, typename Traits, typename Alloc>
        struct is_char_array_to_stdstring< ndnboost::container::basic_string<CharT, Traits, Alloc>, const CharT* >
            : ndnboost::true_type
        {};

        template<typename Target, typename Source>
        struct lexical_converter_impl
        {
            typedef lexical_cast_stream_traits<Source, Target>  stream_trait;

            typedef detail::lexical_istream_limited_src<
                NDNBOOST_DEDUCED_TYPENAME stream_trait::char_type,
                NDNBOOST_DEDUCED_TYPENAME stream_trait::traits,
                stream_trait::requires_stringbuf,
                stream_trait::len_t::value + 1
            > i_interpreter_type;

            typedef detail::lexical_ostream_limited_src<
                NDNBOOST_DEDUCED_TYPENAME stream_trait::char_type,
                NDNBOOST_DEDUCED_TYPENAME stream_trait::traits
            > o_interpreter_type;

            static inline bool try_convert(const Source& arg, Target& result) {
                i_interpreter_type i_interpreter;

                // Disabling ADL, by directly specifying operators.
                if (!(i_interpreter.operator <<(arg)))
                    return false;

                o_interpreter_type out(i_interpreter.cbegin(), i_interpreter.cend());

                // Disabling ADL, by directly specifying operators.
                if(!(out.operator >>(result)))
                    return false;

                return true;
            }
        };

        template <typename Target, typename Source>
        struct copy_converter_impl
        {
// MSVC fail to forward an array (DevDiv#555157 "SILENT BAD CODEGEN triggered by perfect forwarding",
// fixed in 2013 RTM).
#if !defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES) && (!defined(NDNBOOST_MSVC) || NDNBOOST_MSVC >= 1800)
            template <class T>
            static inline bool try_convert(T&& arg, Target& result) {
                result = static_cast<T&&>(arg); // eqaul to `result = std::forward<T>(arg);`
                return true;
            }
#else
            static inline bool try_convert(const Source& arg, Target& result) {
                result = arg;
                return true;
            }
#endif
        };

        template <class Source >
        struct detect_precision_loss
        {
            typedef Source source_type;
            typedef ndnboost::numeric::Trunc<Source> Rounder;
            typedef NDNBOOST_DEDUCED_TYPENAME mpl::if_<
                ndnboost::is_arithmetic<Source>, Source, Source const&
            >::type argument_type ;

            static inline source_type nearbyint(argument_type s, bool& is_ok) NDNBOOST_NOEXCEPT {
                const source_type near_int = Rounder::nearbyint(s);
                if (near_int && is_ok) {
                    const source_type orig_div_round = s / near_int;
                    const source_type eps = std::numeric_limits<source_type>::epsilon();

                    is_ok = !((orig_div_round > 1 ? orig_div_round - 1 : 1 - orig_div_round) > eps);
                }

                return s;
            }

            typedef typename Rounder::round_style round_style;
        };

        template <typename Base, class Source>
        struct fake_precision_loss: public Base
        {
            typedef Source source_type ;
            typedef NDNBOOST_DEDUCED_TYPENAME mpl::if_<
                ndnboost::is_arithmetic<Source>, Source, Source const&
            >::type argument_type ;

            static inline source_type nearbyint(argument_type s, bool& /*is_ok*/) NDNBOOST_NOEXCEPT {
                return s;
            }
        };

        struct nothrow_overflow_handler
        {
            inline bool operator() ( ndnboost::numeric::range_check_result r ) const NDNBOOST_NOEXCEPT {
                return (r == ndnboost::numeric::cInRange);
            }
        };

        template <typename Target, typename Source>
        inline bool noexcept_numeric_convert(const Source& arg, Target& result) NDNBOOST_NOEXCEPT {
            typedef ndnboost::numeric::converter<
                    Target,
                    Source,
                    ndnboost::numeric::conversion_traits<Target, Source >,
                    nothrow_overflow_handler,
                    detect_precision_loss<Source >
            > converter_orig_t;

            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::mpl::if_c<
                ndnboost::is_base_of< detect_precision_loss<Source >, converter_orig_t >::value,
                converter_orig_t,
                fake_precision_loss<converter_orig_t, Source>
            >::type converter_t;

            bool res = nothrow_overflow_handler()(converter_t::out_of_range(arg));
            result = converter_t::low_level_convert(converter_t::nearbyint(arg, res));
            return res;
        }

        template <typename Target, typename Source>
        struct lexical_cast_dynamic_num_not_ignoring_minus
        {
            static inline bool try_convert(const Source &arg, Target& result) NDNBOOST_NOEXCEPT {
                return noexcept_numeric_convert<Target, Source >(arg, result);
            }
        };

        template <typename Target, typename Source>
        struct lexical_cast_dynamic_num_ignoring_minus
        {
            static inline bool try_convert(const Source &arg, Target& result) NDNBOOST_NOEXCEPT {
                typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::mpl::eval_if_c<
                        ndnboost::is_float<Source>::value,
                        ndnboost::mpl::identity<Source>,
                        ndnboost::make_unsigned<Source>
                >::type usource_t;
        
                if (arg < 0) {
                    const bool res = noexcept_numeric_convert<Target, usource_t>(0u - arg, result);
                    result = static_cast<Target>(0u - result);
                    return res;
                } else {
                    return noexcept_numeric_convert<Target, usource_t>(arg, result);
                }
            }
        };

        /*
         * lexical_cast_dynamic_num follows the rules:
         * 1) If Source can be converted to Target without precision loss and
         * without overflows, then assign Source to Target and return
         *
         * 2) If Source is less than 0 and Target is an unsigned integer,
         * then negate Source, check the requirements of rule 1) and if
         * successful, assign static_casted Source to Target and return
         *
         * 3) Otherwise throw a bad_lexical_cast exception
         *
         *
         * Rule 2) required because ndnboost::lexical_cast has the behavior of
         * stringstream, which uses the rules of scanf for conversions. And
         * in the C99 standard for unsigned input value minus sign is
         * optional, so if a negative number is read, no errors will arise
         * and the result will be the two's complement.
         */
        template <typename Target, typename Source>
        struct dynamic_num_converter_impl
        {
            static inline bool try_convert(const Source &arg, Target& result) NDNBOOST_NOEXCEPT {
                typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::mpl::if_c<
                    ndnboost::type_traits::ice_and<
                        ndnboost::is_unsigned<Target>::value,
                        ndnboost::type_traits::ice_or<
                            ndnboost::is_signed<Source>::value,
                            ndnboost::is_float<Source>::value
                        >::value,
                        ndnboost::type_traits::ice_not<
                            ndnboost::is_same<Source, bool>::value
                        >::value,
                        ndnboost::type_traits::ice_not<
                            ndnboost::is_same<Target, bool>::value
                        >::value
                    >::value,
                    lexical_cast_dynamic_num_ignoring_minus<Target, Source>,
                    lexical_cast_dynamic_num_not_ignoring_minus<Target, Source>
                >::type caster_type;

                return caster_type::try_convert(arg, result);
            }
        };
    }

    namespace conversion { namespace detail {

        template <typename Target, typename Source>
        inline bool try_lexical_convert(const Source& arg, Target& result)
        {
            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::detail::array_to_pointer_decay<Source>::type src;

            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::type_traits::ice_or<
                ndnboost::detail::is_xchar_to_xchar<Target, src >::value,
                ndnboost::detail::is_char_array_to_stdstring<Target, src >::value,
                ndnboost::type_traits::ice_and<
                     ndnboost::is_same<Target, src >::value,
                     ndnboost::detail::is_stdstring<Target >::value
                >::value,
                ndnboost::type_traits::ice_and<
                     ndnboost::is_same<Target, src >::value,
                     ndnboost::detail::is_character<Target >::value
                >::value
            > shall_we_copy_t;

            typedef ndnboost::detail::is_arithmetic_and_not_xchars<Target, src >
                shall_we_copy_with_dynamic_check_t;

            // We do evaluate second `if_` lazily to avoid unnecessary instantiations
            // of `shall_we_copy_with_dynamic_check_t` and improve compilation times.
            typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::mpl::if_c<
                shall_we_copy_t::value,
                ndnboost::mpl::identity<ndnboost::detail::copy_converter_impl<Target, src > >,
                ndnboost::mpl::if_<
                     shall_we_copy_with_dynamic_check_t,
                     ndnboost::detail::dynamic_num_converter_impl<Target, src >,
                     ndnboost::detail::lexical_converter_impl<Target, src >
                >
            >::type caster_type_lazy;

            typedef NDNBOOST_DEDUCED_TYPENAME caster_type_lazy::type caster_type;

            return caster_type::try_convert(arg, result);
        }

        template <typename Target, typename CharacterT>
        inline bool try_lexical_convert(const CharacterT* chars, std::size_t count, Target& result)
        {
            NDNBOOST_STATIC_ASSERT_MSG(
                ndnboost::detail::is_character<CharacterT>::value,
                "This overload of try_lexical_convert is meant to be used only with arrays of characters."
            );
            return ::ndnboost::conversion::detail::try_lexical_convert(
                ::ndnboost::iterator_range<const CharacterT*>(chars, chars + count), result
            );
        }

    }} // namespace conversion::detail

    namespace conversion {
        // ADL barrier
        using ::ndnboost::conversion::detail::try_lexical_convert;
    }

    template <typename Target, typename Source>
    inline Target lexical_cast(const Source &arg)
    {
        Target result;

        if (!ndnboost::conversion::detail::try_lexical_convert(arg, result))
            NDNBOOST_LCAST_THROW_BAD_CAST(Source, Target);

        return result;
    }

    template <typename Target>
    inline Target lexical_cast(const char* chars, std::size_t count)
    {
        return ::ndnboost::lexical_cast<Target>(
            ::ndnboost::iterator_range<const char*>(chars, chars + count)
        );
    }

    template <typename Target>
    inline Target lexical_cast(const unsigned char* chars, std::size_t count)
    {
        return ::ndnboost::lexical_cast<Target>(
            ::ndnboost::iterator_range<const unsigned char*>(chars, chars + count)
        );
    }

    template <typename Target>
    inline Target lexical_cast(const signed char* chars, std::size_t count)
    {
        return ::ndnboost::lexical_cast<Target>(
            ::ndnboost::iterator_range<const signed char*>(chars, chars + count)
        );
    }

#ifndef NDNBOOST_LCAST_NO_WCHAR_T
    template <typename Target>
    inline Target lexical_cast(const wchar_t* chars, std::size_t count)
    {
        return ::ndnboost::lexical_cast<Target>(
            ::ndnboost::iterator_range<const wchar_t*>(chars, chars + count)
        );
    }
#endif
#ifndef NDNBOOST_NO_CXX11_CHAR16_T
    template <typename Target>
    inline Target lexical_cast(const char16_t* chars, std::size_t count)
    {
        return ::ndnboost::lexical_cast<Target>(
            ::ndnboost::iterator_range<const char16_t*>(chars, chars + count)
        );
    }
#endif
#ifndef NDNBOOST_NO_CXX11_CHAR32_T
    template <typename Target>
    inline Target lexical_cast(const char32_t* chars, std::size_t count)
    {
        return ::ndnboost::lexical_cast<Target>(
            ::ndnboost::iterator_range<const char32_t*>(chars, chars + count)
        );
    }
#endif

} // namespace ndnboost

#else

namespace ndnboost {
    namespace detail
    {

        // selectors for choosing stream character type
        template<typename Type>
        struct stream_char
        {
            typedef char type;
        };

#ifndef NDNBOOST_LCAST_NO_WCHAR_T
#ifndef NDNBOOST_NO_INTRINSIC_WCHAR_T
        template<>
        struct stream_char<wchar_t>
        {
            typedef wchar_t type;
        };
#endif

        template<>
        struct stream_char<wchar_t *>
        {
            typedef wchar_t type;
        };

        template<>
        struct stream_char<const wchar_t *>
        {
            typedef wchar_t type;
        };

        template<>
        struct stream_char<std::wstring>
        {
            typedef wchar_t type;
        };
#endif

        // stream wrapper for handling lexical conversions
        template<typename Target, typename Source, typename Traits>
        class lexical_stream
        {
        private:
            typedef typename widest_char<
                typename stream_char<Target>::type,
                typename stream_char<Source>::type>::type char_type;

            typedef Traits traits_type;

        public:
            lexical_stream(char_type* = 0, char_type* = 0)
            {
                stream.unsetf(std::ios::skipws);
                lcast_set_precision(stream, static_cast<Source*>(0), static_cast<Target*>(0) );
            }
            ~lexical_stream()
            {
                #if defined(NDNBOOST_NO_STRINGSTREAM)
                stream.freeze(false);
                #endif
            }
            bool operator<<(const Source &input)
            {
                return !(stream << input).fail();
            }
            template<typename InputStreamable>
            bool operator>>(InputStreamable &output)
            {
                return !is_pointer<InputStreamable>::value &&
                       stream >> output &&
                       stream.get() == traits_type::eof();
            }

            bool operator>>(std::string &output)
            {
                #if defined(NDNBOOST_NO_STRINGSTREAM)
                stream << '\0';
                #endif
                stream.str().swap(output);
                return true;
            }
            #ifndef NDNBOOST_LCAST_NO_WCHAR_T
            bool operator>>(std::wstring &output)
            {
                stream.str().swap(output);
                return true;
            }
            #endif

        private:
            #if defined(NDNBOOST_NO_STRINGSTREAM)
            std::strstream stream;
            #elif defined(NDNBOOST_NO_STD_LOCALE)
            std::stringstream stream;
            #else
            std::basic_stringstream<char_type,traits_type> stream;
            #endif
        };
    }

    // call-by-value fallback version (deprecated)

    template<typename Target, typename Source>
    Target lexical_cast(Source arg)
    {
        typedef typename detail::widest_char< 
            NDNBOOST_DEDUCED_TYPENAME detail::stream_char<Target>::type 
          , NDNBOOST_DEDUCED_TYPENAME detail::stream_char<Source>::type 
        >::type char_type; 

        typedef std::char_traits<char_type> traits;
        detail::lexical_stream<Target, Source, traits> interpreter;
        Target result;

        if(!(interpreter << arg && interpreter >> result))
          NDNBOOST_LCAST_THROW_BAD_CAST(Source, Target);
        return result;
    }

} // namespace ndnboost

#endif

// Copyright Kevlin Henney, 2000-2005.
// Copyright Alexander Nasonov, 2006-2010.
// Copyright Antony Polukhin, 2011-2014.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#undef NDNBOOST_LCAST_THROW_BAD_CAST
#undef NDNBOOST_LCAST_NO_WCHAR_T

#endif // NDNBOOST_LEXICAL_CAST_INCLUDED

