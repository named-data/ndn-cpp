#ifndef NDNBOOST_THREAD_DETAIL_THREAD_INTERRUPTION_HPP
#define NDNBOOST_THREAD_DETAIL_THREAD_INTERRUPTION_HPP
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2007-9 Anthony Williams
// (C) Copyright 2012 Vicente J. Botet Escriba

#include <ndnboost/thread/detail/config.hpp>
#include <ndnboost/thread/detail/delete.hpp>

#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS

namespace ndnboost
{
    namespace this_thread
    {
        class NDNBOOST_THREAD_DECL disable_interruption
        {
          bool interruption_was_enabled;
          friend class restore_interruption;
        public:
            NDNBOOST_THREAD_NO_COPYABLE(disable_interruption)
            disable_interruption() NDNBOOST_NOEXCEPT;
            ~disable_interruption() NDNBOOST_NOEXCEPT;
        };

        class NDNBOOST_THREAD_DECL restore_interruption
        {
        public:
            NDNBOOST_THREAD_NO_COPYABLE(restore_interruption)
            explicit restore_interruption(disable_interruption& d) NDNBOOST_NOEXCEPT;
            ~restore_interruption() NDNBOOST_NOEXCEPT;
        };
    }
}

#endif // NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
#endif // header
