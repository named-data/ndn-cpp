//  duration.hpp  --------------------------------------------------------------//

//  Copyright 2008 Howard Hinnant
//  Copyright 2008 Beman Dawes
//  Copyright 2009-2011 Vicente J. Botet Escriba

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

/*

This code was derived by Beman Dawes from Howard Hinnant's time2_demo prototype.
Many thanks to Howard for making his code available under the Boost license.
The original code was modified to conform to Boost conventions and to section
20.9 Time utilities [time] of the C++ committee's working paper N2798.
See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2798.pdf.

time2_demo contained this comment:

    Much thanks to Andrei Alexandrescu,
                   Walter Brown,
                   Peter Dimov,
                   Jeff Garland,
                   Terry Golubiewski,
                   Daniel Krugler,
                   Anthony Williams.
*/


#ifndef NDNBOOST_CHRONO_DURATION_HPP
#define NDNBOOST_CHRONO_DURATION_HPP

#include <ndnboost/chrono/config.hpp>
#include <ndnboost/chrono/detail/static_assert.hpp>

#include <climits>
#include <limits>


#include <ndnboost/mpl/logical.hpp>
#include <ndnboost/ratio/ratio.hpp>
#include <ndnboost/type_traits/common_type.hpp>
#include <ndnboost/type_traits/is_arithmetic.hpp>
#include <ndnboost/type_traits/is_convertible.hpp>
#include <ndnboost/type_traits/is_floating_point.hpp>
#include <ndnboost/type_traits/is_unsigned.hpp>
#include <ndnboost/chrono/detail/is_evenly_divisible_by.hpp>

#include <ndnboost/cstdint.hpp>
#include <ndnboost/utility/enable_if.hpp>
#include <ndnboost/detail/workaround.hpp>
#include <ndnboost/integer_traits.hpp>

#if !defined(NDNBOOST_NO_CXX11_STATIC_ASSERT) || !defined(NDNBOOST_CHRONO_USES_MPL_ASSERT)
#define NDNBOOST_CHRONO_A_DURATION_REPRESENTATION_CAN_NOT_BE_A_DURATION        "A duration representation can not be a duration"
#define NDNBOOST_CHRONO_SECOND_TEMPLATE_PARAMETER_OF_DURATION_MUST_BE_A_STD_RATIO "Second template parameter of duration must be a ndnboost::ratio"
#define NDNBOOST_CHRONO_DURATION_PERIOD_MUST_BE_POSITIVE "duration period must be positive"
#define NDNBOOST_CHRONO_SECOND_TEMPLATE_PARAMETER_OF_TIME_POINT_MUST_BE_A_NDNBOOST_CHRONO_DURATION "Second template parameter of time_point must be a ndnboost::chrono::duration"
#endif

#ifndef NDNBOOST_CHRONO_HEADER_ONLY
// this must occur after all of the includes and before any code appears:
#include <ndnboost/config/abi_prefix.hpp> // must be the last #include
#endif

//----------------------------------------------------------------------------//
//                                                                            //
//                        20.9 Time utilities [time]                          //
//                                 synopsis                                   //
//                                                                            //
//----------------------------------------------------------------------------//

namespace ndnboost {
namespace chrono {

    template <class Rep, class Period = ratio<1> >
    class duration;

    namespace detail
    {
    template <class T>
      struct is_duration
        : ndnboost::false_type {};

    template <class Rep, class Period>
      struct is_duration<duration<Rep, Period> >
        : ndnboost::true_type  {};

    template <class Duration, class Rep, bool = is_duration<Rep>::value>
    struct duration_divide_result
    {
    };

    template <class Duration, class Rep2,
        bool = (
                    ((ndnboost::is_convertible<typename Duration::rep,
                        typename common_type<typename Duration::rep, Rep2>::type>::value))
                &&  ((ndnboost::is_convertible<Rep2,
                        typename common_type<typename Duration::rep, Rep2>::type>::value))
                )
        >
    struct duration_divide_imp
    {
    };

    template <class Rep1, class Period, class Rep2>
    struct duration_divide_imp<duration<Rep1, Period>, Rep2, true>
    {
        typedef duration<typename common_type<Rep1, Rep2>::type, Period> type;
    };

    template <class Rep1, class Period, class Rep2>
    struct duration_divide_result<duration<Rep1, Period>, Rep2, false>
        : duration_divide_imp<duration<Rep1, Period>, Rep2>
    {
    };

///
    template <class Rep, class Duration, bool = is_duration<Rep>::value>
    struct duration_divide_result2
    {
    };

    template <class Rep, class Duration,
        bool = (
                    ((ndnboost::is_convertible<typename Duration::rep,
                        typename common_type<typename Duration::rep, Rep>::type>::value))
                &&  ((ndnboost::is_convertible<Rep,
                        typename common_type<typename Duration::rep, Rep>::type>::value))
                )
        >
    struct duration_divide_imp2
    {
    };

    template <class Rep1, class Rep2, class Period >
    struct duration_divide_imp2<Rep1, duration<Rep2, Period>, true>
    {
        //typedef typename common_type<Rep1, Rep2>::type type;
        typedef double type;
    };

    template <class Rep1, class Rep2, class Period >
    struct duration_divide_result2<Rep1, duration<Rep2, Period>, false>
        : duration_divide_imp2<Rep1, duration<Rep2, Period> >
    {
    };

///
    template <class Duration, class Rep, bool = is_duration<Rep>::value>
    struct duration_modulo_result
    {
    };

    template <class Duration, class Rep2,
        bool = (
                    //ndnboost::is_convertible<typename Duration::rep,
                        //typename common_type<typename Duration::rep, Rep2>::type>::value
                //&&
    ndnboost::is_convertible<Rep2,
                        typename common_type<typename Duration::rep, Rep2>::type>::value
                )
        >
    struct duration_modulo_imp
    {
    };

    template <class Rep1, class Period, class Rep2>
    struct duration_modulo_imp<duration<Rep1, Period>, Rep2, true>
    {
        typedef duration<typename common_type<Rep1, Rep2>::type, Period> type;
    };

    template <class Rep1, class Period, class Rep2>
    struct duration_modulo_result<duration<Rep1, Period>, Rep2, false>
        : duration_modulo_imp<duration<Rep1, Period>, Rep2>
    {
    };

} // namespace detail
} // namespace chrono


// common_type trait specializations

template <class Rep1, class Period1, class Rep2, class Period2>
struct common_type<chrono::duration<Rep1, Period1>,
                     chrono::duration<Rep2, Period2> >;


namespace chrono {

    // customization traits
    template <class Rep> struct treat_as_floating_point;
    template <class Rep> struct duration_values;

    // convenience typedefs
    typedef duration<ndnboost::int_least64_t, nano> nanoseconds;    // at least 64 bits needed
    typedef duration<ndnboost::int_least64_t, micro> microseconds;  // at least 55 bits needed
    typedef duration<ndnboost::int_least64_t, milli> milliseconds;  // at least 45 bits needed
    typedef duration<ndnboost::int_least64_t> seconds;              // at least 35 bits needed
    typedef duration<ndnboost::int_least32_t, ratio< 60> > minutes; // at least 29 bits needed
    typedef duration<ndnboost::int_least32_t, ratio<3600> > hours;  // at least 23 bits needed

//----------------------------------------------------------------------------//
//                          duration helpers                                  //
//----------------------------------------------------------------------------//

namespace detail
{

    // duration_cast

    // duration_cast is the heart of this whole prototype.  It can convert any
    //   duration to any other.  It is also (implicitly) used in converting
    //   time_points.  The conversion is always exact if possible.  And it is
    //   always as efficient as hand written code.  If different representations
    //   are involved, care is taken to never require implicit conversions.
    //   Instead static_cast is used explicitly for every required conversion.
    //   If there are a mixture of integral and floating point representations,
    //   the use of common_type ensures that the most logical "intermediate"
    //   representation is used.
    template <class FromDuration, class ToDuration,
              class Period,
              bool PeriodNumEq1,
              bool PeriodDenEq1>
    struct duration_cast_aux;

    // When the two periods are the same, all that is left to do is static_cast from
    //   the source representation to the target representation (which may be a no-op).
    //   This conversion is always exact as long as the static_cast from the source
    //   representation to the destination representation is exact.
    template <class FromDuration, class ToDuration, class Period>
    struct duration_cast_aux<FromDuration, ToDuration, Period, true, true>
    {
        NDNBOOST_CONSTEXPR ToDuration operator()(const FromDuration& fd) const
        {
            return ToDuration(static_cast<typename ToDuration::rep>(fd.count()));
        }
    };

    // When the numerator of FromPeriod / ToPeriod is 1, then all we need to do is
    //   divide by the denominator of FromPeriod / ToPeriod.  The common_type of
    //   the two representations is used for the intermediate computation before
    //   static_cast'ing to the destination.
    //   This conversion is generally not exact because of the division (but could be
    //   if you get lucky on the run time value of fd.count()).
    template <class FromDuration, class ToDuration, class Period>
    struct duration_cast_aux<FromDuration, ToDuration, Period, true, false>
    {
        NDNBOOST_CONSTEXPR ToDuration operator()(const FromDuration& fd) const
        {
            typedef typename common_type<
                typename ToDuration::rep,
                typename FromDuration::rep,
                ndnboost::intmax_t>::type C;
            return ToDuration(static_cast<typename ToDuration::rep>(
                              static_cast<C>(fd.count()) / static_cast<C>(Period::den)));
        }
    };

    // When the denominator of FromPeriod / ToPeriod is 1, then all we need to do is
    //   multiply by the numerator of FromPeriod / ToPeriod.  The common_type of
    //   the two representations is used for the intermediate computation before
    //   static_cast'ing to the destination.
    //   This conversion is always exact as long as the static_cast's involved are exact.
    template <class FromDuration, class ToDuration, class Period>
    struct duration_cast_aux<FromDuration, ToDuration, Period, false, true>
    {
        NDNBOOST_CONSTEXPR ToDuration operator()(const FromDuration& fd) const
        {
            typedef typename common_type<
              typename ToDuration::rep,
              typename FromDuration::rep,
              ndnboost::intmax_t>::type C;
            return ToDuration(static_cast<typename ToDuration::rep>(
                              static_cast<C>(fd.count()) * static_cast<C>(Period::num)));
        }
    };

    // When neither the numerator or denominator of FromPeriod / ToPeriod is 1, then we need to
    //   multiply by the numerator and divide by the denominator of FromPeriod / ToPeriod.  The
    //   common_type of the two representations is used for the intermediate computation before
    //   static_cast'ing to the destination.
    //   This conversion is generally not exact because of the division (but could be
    //   if you get lucky on the run time value of fd.count()).
    template <class FromDuration, class ToDuration, class Period>
    struct duration_cast_aux<FromDuration, ToDuration, Period, false, false>
    {
        NDNBOOST_CONSTEXPR ToDuration operator()(const FromDuration& fd) const
        {
            typedef typename common_type<
              typename ToDuration::rep,
              typename FromDuration::rep,
              ndnboost::intmax_t>::type C;
            return ToDuration(static_cast<typename ToDuration::rep>(
               static_cast<C>(fd.count()) * static_cast<C>(Period::num)
                 / static_cast<C>(Period::den)));
        }
    };

    template <class FromDuration, class ToDuration>
    struct duration_cast {
        typedef typename ratio_divide<typename FromDuration::period,
              typename ToDuration::period>::type Period;
        typedef duration_cast_aux<
            FromDuration,
            ToDuration,
            Period,
            Period::num == 1,
            Period::den == 1
        > Aux;
        NDNBOOST_CONSTEXPR ToDuration operator()(const FromDuration& fd) const
        {
            return Aux()(fd);
        }
    };

} // namespace detail

//----------------------------------------------------------------------------//
//                                                                            //
//      20.9.2 Time-related traits [time.traits]                              //
//                                                                            //
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//      20.9.2.1 treat_as_floating_point [time.traits.is_fp]                        //
//      Probably should have been treat_as_floating_point. Editor notifed.    //
//----------------------------------------------------------------------------//

    // Support bidirectional (non-exact) conversions for floating point rep types
    //   (or user defined rep types which specialize treat_as_floating_point).
    template <class Rep>
    struct treat_as_floating_point : ndnboost::is_floating_point<Rep> {};

//----------------------------------------------------------------------------//
//      20.9.2.2 duration_values [time.traits.duration_values]                //
//----------------------------------------------------------------------------//

namespace detail {
    template <class T, bool = is_arithmetic<T>::value>
    struct chrono_numeric_limits {
        static NDNBOOST_CHRONO_LIB_CONSTEXPR T lowest() NDNBOOST_CHRONO_LIB_NOEXCEPT_OR_THROW {return (std::numeric_limits<T>::min)  ();}
    };

    template <class T>
    struct chrono_numeric_limits<T,true> {
        static NDNBOOST_CHRONO_LIB_CONSTEXPR T lowest() NDNBOOST_CHRONO_LIB_NOEXCEPT_OR_THROW {return (std::numeric_limits<T>::min)  ();}
    };

    template <>
    struct chrono_numeric_limits<float,true> {
        static NDNBOOST_CHRONO_LIB_CONSTEXPR float lowest() NDNBOOST_CHRONO_LIB_NOEXCEPT_OR_THROW
        {
            return -(std::numeric_limits<float>::max) ();
        }
    };

    template <>
    struct chrono_numeric_limits<double,true> {
        static NDNBOOST_CHRONO_LIB_CONSTEXPR double lowest() NDNBOOST_CHRONO_LIB_NOEXCEPT_OR_THROW
        {
            return -(std::numeric_limits<double>::max) ();
        }
    };

    template <>
    struct chrono_numeric_limits<long double,true> {
        static NDNBOOST_CHRONO_LIB_CONSTEXPR long double lowest() NDNBOOST_CHRONO_LIB_NOEXCEPT_OR_THROW
        {
            return -(std::numeric_limits<long double>::max)();
        }
    };

    template <class T>
    struct numeric_limits : chrono_numeric_limits<typename remove_cv<T>::type>
    {};

}
template <class Rep>
struct duration_values
{
    static NDNBOOST_CONSTEXPR Rep zero() {return Rep(0);}
    static NDNBOOST_CHRONO_LIB_CONSTEXPR Rep max NDNBOOST_PREVENT_MACRO_SUBSTITUTION ()
    {
        return (std::numeric_limits<Rep>::max)();
    }

    static NDNBOOST_CHRONO_LIB_CONSTEXPR Rep min NDNBOOST_PREVENT_MACRO_SUBSTITUTION ()
    {
        return detail::numeric_limits<Rep>::lowest();
    }
};

}  // namespace chrono

//----------------------------------------------------------------------------//
//      20.9.2.3 Specializations of common_type [time.traits.specializations] //
//----------------------------------------------------------------------------//

template <class Rep1, class Period1, class Rep2, class Period2>
struct common_type<chrono::duration<Rep1, Period1>,
                   chrono::duration<Rep2, Period2> >
{
  typedef chrono::duration<typename common_type<Rep1, Rep2>::type,
                      typename ndnboost::ratio_gcd<Period1, Period2>::type> type;
};


//----------------------------------------------------------------------------//
//                                                                            //
//         20.9.3 Class template duration [time.duration]                     //
//                                                                            //
//----------------------------------------------------------------------------//


namespace chrono {

    template <class Rep, class Period>
    class NDNBOOST_SYMBOL_VISIBLE duration
    {
    //NDNBOOST_CHRONO_STATIC_ASSERT(ndnboost::is_integral<Rep>::value, NDNBOOST_CHRONO_A_DURATION_REPRESENTATION_MUST_BE_INTEGRAL, ());
    NDNBOOST_CHRONO_STATIC_ASSERT(!ndnboost::chrono::detail::is_duration<Rep>::value,
            NDNBOOST_CHRONO_A_DURATION_REPRESENTATION_CAN_NOT_BE_A_DURATION, ());
    NDNBOOST_CHRONO_STATIC_ASSERT(ndnboost::ratio_detail::is_ratio<typename Period::type>::value,
            NDNBOOST_CHRONO_SECOND_TEMPLATE_PARAMETER_OF_DURATION_MUST_BE_A_STD_RATIO, ());
    NDNBOOST_CHRONO_STATIC_ASSERT(Period::num>0,
            NDNBOOST_CHRONO_DURATION_PERIOD_MUST_BE_POSITIVE, ());
    public:
        typedef Rep rep;
        typedef Period period;
    private:
        rep rep_;
    public:

        NDNBOOST_FORCEINLINE NDNBOOST_CONSTEXPR
        duration() : rep_(duration_values<rep>::zero()) { }
        template <class Rep2>
        NDNBOOST_SYMBOL_VISIBLE NDNBOOST_FORCEINLINE NDNBOOST_CONSTEXPR
        explicit duration(const Rep2& r
        , typename ndnboost::enable_if <
                    mpl::and_ <
                        ndnboost::is_convertible<Rep2, rep>,
                        mpl::or_ <
                            treat_as_floating_point<rep>,
                            mpl::and_ <
                                mpl::not_ < treat_as_floating_point<rep> >,
                                mpl::not_ < treat_as_floating_point<Rep2> >
                            >
                        >
                    >
                >::type* = 0
            ) : rep_(r) { }
        //~duration() {} //= default;
//        NDNBOOST_CONSTEXPR        duration(const duration& rhs) : rep_(rhs.rep_) {} // = default;
        duration& operator=(const duration& rhs) // = default;
        {
            if (&rhs != this) rep_= rhs.rep_;
            return *this;
        }

        // conversions
        template <class Rep2, class Period2>
        NDNBOOST_FORCEINLINE NDNBOOST_CONSTEXPR
        duration(const duration<Rep2, Period2>& d
        , typename ndnboost::enable_if <
                    mpl::or_ <
                        treat_as_floating_point<rep>,
                        mpl::and_ <
                            chrono_detail::is_evenly_divisible_by<Period2, period>,
                            mpl::not_ < treat_as_floating_point<Rep2> >
                        >
                    >
                >::type* = 0
        )
            : rep_(chrono::detail::duration_cast<duration<Rep2, Period2>, duration>()(d).count()) {}

        // observer

        NDNBOOST_CONSTEXPR
        rep count() const {return rep_;}

        // arithmetic

        NDNBOOST_CONSTEXPR
        duration  operator+() const {return duration(rep_);;}
        NDNBOOST_CONSTEXPR
        duration  operator-() const {return duration(-rep_);}
        duration& operator++()      {++rep_; return *this;}
        duration  operator++(int)   {return duration(rep_++);}
        duration& operator--()      {--rep_; return *this;}
        duration  operator--(int)   {return duration(rep_--);}

        duration& operator+=(const duration& d)
        {
            rep_ += d.count(); return *this;
        }
        duration& operator-=(const duration& d)
        {
            rep_ -= d.count(); return *this;
        }

        duration& operator*=(const rep& rhs) {rep_ *= rhs; return *this;}
        duration& operator/=(const rep& rhs) {rep_ /= rhs; return *this;}
        duration& operator%=(const rep& rhs) {rep_ %= rhs; return *this;}
        duration& operator%=(const duration& rhs)
        {
            rep_ %= rhs.count(); return *this;
        }
        // 20.9.3.4 duration special values [time.duration.special]

        static NDNBOOST_CONSTEXPR duration zero()
        {
            return duration(duration_values<rep>::zero());
        }
        static NDNBOOST_CHRONO_LIB_CONSTEXPR duration min NDNBOOST_PREVENT_MACRO_SUBSTITUTION ()
        {
            return duration((duration_values<rep>::min)());
        }
        static NDNBOOST_CHRONO_LIB_CONSTEXPR duration max NDNBOOST_PREVENT_MACRO_SUBSTITUTION ()
        {
            return duration((duration_values<rep>::max)());
        }
    };

//----------------------------------------------------------------------------//
//      20.9.3.5 duration non-member arithmetic [time.duration.nonmember]     //
//----------------------------------------------------------------------------//

    // Duration +

    template <class Rep1, class Period1, class Rep2, class Period2>
    inline NDNBOOST_CONSTEXPR
    typename common_type<duration<Rep1, Period1>, duration<Rep2, Period2> >::type
    operator+(const duration<Rep1, Period1>& lhs,
          const duration<Rep2, Period2>& rhs)
    {
      typedef typename common_type<duration<Rep1, Period1>,
        duration<Rep2, Period2> >::type CD;
      return CD(CD(lhs).count()+CD(rhs).count());
    }

    // Duration -

    template <class Rep1, class Period1, class Rep2, class Period2>
    inline NDNBOOST_CONSTEXPR
    typename common_type<duration<Rep1, Period1>, duration<Rep2, Period2> >::type
    operator-(const duration<Rep1, Period1>& lhs,
          const duration<Rep2, Period2>& rhs)
    {
      typedef typename common_type<duration<Rep1, Period1>,
            duration<Rep2, Period2> >::type CD;
      return CD(CD(lhs).count()-CD(rhs).count());
    }

    // Duration *

    template <class Rep1, class Period, class Rep2>
    inline NDNBOOST_CONSTEXPR
    typename ndnboost::enable_if <
        mpl::and_ <
        ndnboost::is_convertible<Rep1, typename common_type<Rep1, Rep2>::type>,
        ndnboost::is_convertible<Rep2, typename common_type<Rep1, Rep2>::type>
        >,
        duration<typename common_type<Rep1, Rep2>::type, Period>
    >::type
    operator*(const duration<Rep1, Period>& d, const Rep2& s)
    {
      typedef typename common_type<Rep1, Rep2>::type CR;
      typedef duration<CR, Period> CD;
      return CD(CD(d).count()*static_cast<CR>(s));
    }

    template <class Rep1, class Period, class Rep2>
    inline NDNBOOST_CONSTEXPR
    typename ndnboost::enable_if <
        mpl::and_ <
        ndnboost::is_convertible<Rep1, typename common_type<Rep1, Rep2>::type>,
        ndnboost::is_convertible<Rep2, typename common_type<Rep1, Rep2>::type>
        >,
        duration<typename common_type<Rep1, Rep2>::type, Period>
    >::type
    operator*(const Rep1& s, const duration<Rep2, Period>& d)
    {
        return d * s;
    }

    // Duration /

    template <class Rep1, class Period, class Rep2>
    inline NDNBOOST_CONSTEXPR
    typename ndnboost::disable_if <ndnboost::chrono::detail::is_duration<Rep2>,
      typename ndnboost::chrono::detail::duration_divide_result<
        duration<Rep1, Period>, Rep2>::type
    >::type
    operator/(const duration<Rep1, Period>& d, const Rep2& s)
    {
        typedef typename common_type<Rep1, Rep2>::type CR;
        typedef duration<CR, Period> CD;

      return CD(CD(d).count()/static_cast<CR>(s));
    }

    template <class Rep1, class Period1, class Rep2, class Period2>
    inline NDNBOOST_CONSTEXPR
    typename common_type<Rep1, Rep2>::type
    operator/(const duration<Rep1, Period1>& lhs, const duration<Rep2, Period2>& rhs)
    {
        typedef typename common_type<duration<Rep1, Period1>,
                                   duration<Rep2, Period2> >::type CD;
        return CD(lhs).count() / CD(rhs).count();
    }

    #ifdef NDNBOOST_CHRONO_EXTENSIONS
    template <class Rep1, class Rep2, class Period>
    inline NDNBOOST_CONSTEXPR
    typename ndnboost::disable_if <ndnboost::chrono::detail::is_duration<Rep1>,
      typename ndnboost::chrono::detail::duration_divide_result2<
        Rep1, duration<Rep2, Period> >::type
      >::type
    operator/(const Rep1& s, const duration<Rep2, Period>& d)
    {
        typedef typename common_type<Rep1, Rep2>::type CR;
        typedef duration<CR, Period> CD;

      return static_cast<CR>(s)/CD(d).count();
    }
    #endif
    // Duration %

    template <class Rep1, class Period, class Rep2>
    inline NDNBOOST_CONSTEXPR
    typename ndnboost::disable_if <ndnboost::chrono::detail::is_duration<Rep2>,
      typename ndnboost::chrono::detail::duration_modulo_result<
        duration<Rep1, Period>, Rep2>::type
    >::type
    operator%(const duration<Rep1, Period>& d, const Rep2& s)
    {
        typedef typename common_type<Rep1, Rep2>::type CR;
        typedef duration<CR, Period> CD;

      return CD(CD(d).count()%static_cast<CR>(s));
    }

    template <class Rep1, class Period1, class Rep2, class Period2>
    inline NDNBOOST_CONSTEXPR
    typename common_type<duration<Rep1, Period1>, duration<Rep2, Period2> >::type
    operator%(const duration<Rep1, Period1>& lhs,
          const duration<Rep2, Period2>& rhs) {
        typedef typename common_type<duration<Rep1, Period1>,
                                 duration<Rep2, Period2> >::type CD;

      return CD(CD(lhs).count()%CD(rhs).count());
    }


//----------------------------------------------------------------------------//
//      20.9.3.6 duration comparisons [time.duration.comparisons]             //
//----------------------------------------------------------------------------//

namespace detail
{
    template <class LhsDuration, class RhsDuration>
    struct duration_eq
    {
      NDNBOOST_CONSTEXPR bool operator()(const LhsDuration& lhs, const RhsDuration& rhs) const
        {
            typedef typename common_type<LhsDuration, RhsDuration>::type CD;
            return CD(lhs).count() == CD(rhs).count();
        }
    };

    template <class LhsDuration>
    struct duration_eq<LhsDuration, LhsDuration>
    {
      NDNBOOST_CONSTEXPR bool operator()(const LhsDuration& lhs, const LhsDuration& rhs) const
        {
            return lhs.count() == rhs.count();
        }
    };

    template <class LhsDuration, class RhsDuration>
    struct duration_lt
    {
      NDNBOOST_CONSTEXPR bool operator()(const LhsDuration& lhs, const RhsDuration& rhs) const
        {
            typedef typename common_type<LhsDuration, RhsDuration>::type CD;
            return CD(lhs).count() < CD(rhs).count();
        }
    };

    template <class LhsDuration>
    struct duration_lt<LhsDuration, LhsDuration>
    {
      NDNBOOST_CONSTEXPR bool operator()(const LhsDuration& lhs, const LhsDuration& rhs) const
        {
            return lhs.count() < rhs.count();
        }
    };

} // namespace detail

    // Duration ==

    template <class Rep1, class Period1, class Rep2, class Period2>
    inline NDNBOOST_CONSTEXPR
    bool
    operator==(const duration<Rep1, Period1>& lhs,
          const duration<Rep2, Period2>& rhs)
    {
        return ndnboost::chrono::detail::duration_eq<
            duration<Rep1, Period1>, duration<Rep2, Period2> >()(lhs, rhs);
    }

    // Duration !=

    template <class Rep1, class Period1, class Rep2, class Period2>
    inline NDNBOOST_CONSTEXPR
    bool
    operator!=(const duration<Rep1, Period1>& lhs,
          const duration<Rep2, Period2>& rhs)
    {
        return !(lhs == rhs);
    }

    // Duration <

    template <class Rep1, class Period1, class Rep2, class Period2>
    inline NDNBOOST_CONSTEXPR
    bool
    operator< (const duration<Rep1, Period1>& lhs,
          const duration<Rep2, Period2>& rhs)
    {
        return ndnboost::chrono::detail::duration_lt<
          duration<Rep1, Period1>, duration<Rep2, Period2> >()(lhs, rhs);
    }

    // Duration >

    template <class Rep1, class Period1, class Rep2, class Period2>
    inline NDNBOOST_CONSTEXPR
    bool
    operator> (const duration<Rep1, Period1>& lhs,
          const duration<Rep2, Period2>& rhs)
    {
        return rhs < lhs;
    }

    // Duration <=

    template <class Rep1, class Period1, class Rep2, class Period2>
    inline NDNBOOST_CONSTEXPR
    bool
    operator<=(const duration<Rep1, Period1>& lhs,
          const duration<Rep2, Period2>& rhs)
    {
        return !(rhs < lhs);
    }

    // Duration >=

    template <class Rep1, class Period1, class Rep2, class Period2>
    inline NDNBOOST_CONSTEXPR
    bool
    operator>=(const duration<Rep1, Period1>& lhs,
          const duration<Rep2, Period2>& rhs)
    {
        return !(lhs < rhs);
    }

//----------------------------------------------------------------------------//
//      20.9.3.7 duration_cast [time.duration.cast]                           //
//----------------------------------------------------------------------------//

    // Compile-time select the most efficient algorithm for the conversion...
    template <class ToDuration, class Rep, class Period>
    inline NDNBOOST_CONSTEXPR
    typename ndnboost::enable_if <
      ndnboost::chrono::detail::is_duration<ToDuration>, ToDuration>::type
    duration_cast(const duration<Rep, Period>& fd)
    {
        return ndnboost::chrono::detail::duration_cast<
          duration<Rep, Period>, ToDuration>()(fd);
    }

} // namespace chrono
} // namespace ndnboost

#ifndef NDNBOOST_CHRONO_HEADER_ONLY
// the suffix header occurs after all of our code:
#include <ndnboost/config/abi_suffix.hpp> // pops abi_prefix.hpp pragmas
#endif

#endif // NDNBOOST_CHRONO_DURATION_HPP
