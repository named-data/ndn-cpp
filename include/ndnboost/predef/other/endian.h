/*
Copyright Rene Rivera 2013-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_ENDIAN_H
#define NDNBOOST_PREDEF_ENDIAN_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>
#include <ndnboost/predef/library/c/gnu.h>
#include <ndnboost/predef/os/macos.h>
#include <ndnboost/predef/os/bsd.h>

/*`
[heading `NDNBOOST_ENDIAN_*`]

Detection of endian memory ordering. There are four defined macros
in this header that define the various generally possible endian
memory orderings:

* `NDNBOOST_ENDIAN_BIG_BYTE`, byte-swapped big-endian.
* `NDNBOOST_ENDIAN_BIG_WORD`, word-swapped big-endian.
* `NDNBOOST_ENDIAN_LITTLE_BYTE`, byte-swapped little-endian.
* `NDNBOOST_ENDIAN_LITTLE_WORD`, word-swapped little-endian.

The detection is conservative in that it only identifies endianness
that it knows for certain. In particular bi-endianness is not
indicated as is it not practically possible to determine the
endianness from anything but an operating system provided
header. And the currently known headers do not define that
programatic bi-endianness is available.

This implementation is a compilation of various publicly available
information and acquired knowledge:

# The indispensable documentation of "Pre-defined Compiler Macros"
  [@http://sourceforge.net/p/predef/wiki/Endianness Endianness].
# The various endian specifications available in the
  [@http://wikipedia.org/ Wikipedia] computer architecture pages.
# Generally available searches for headers that define endianness.
 */

#define NDNBOOST_ENDIAN_BIG_BYTE NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE
#define NDNBOOST_ENDIAN_BIG_WORD NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE
#define NDNBOOST_ENDIAN_LITTLE_BYTE NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE
#define NDNBOOST_ENDIAN_LITTLE_WORD NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

/* GNU libc provides a header defining __BYTE_ORDER, or _BYTE_ORDER.
 * And some OSs provide some for of endian header also.
 */
#if !NDNBOOST_ENDIAN_BIG_BYTE && !NDNBOOST_ENDIAN_BIG_WORD && \
    !NDNBOOST_ENDIAN_LITTLE_BYTE && !NDNBOOST_ENDIAN_LITTLE_WORD
#   if NDNBOOST_LIB_C_GNU
#       include <endian.h>
#   else
#       if NDNBOOST_OS_MACOS
#           include <machine/endian.h>
#       else
#           if NDNBOOST_OS_BSD
#               if NDNBOOST_OS_BSD_OPEN
#                   include <machine/endian.h>
#               else
#                   include <sys/endian.h>
#               endif
#           endif
#       endif
#   endif
#   if defined(__BYTE_ORDER)
#       if (__BYTE_ORDER == __BIG_ENDIAN)
#           undef NDNBOOST_ENDIAN_BIG_BYTE
#           define NDNBOOST_ENDIAN_BIG_BYTE NDNBOOST_VERSION_NUMBER_AVAILABLE
#       endif
#       if (__BYTE_ORDER == __LITTLE_ENDIAN)
#           undef NDNBOOST_ENDIAN_LITTLE_BYTE
#           define NDNBOOST_ENDIAN_LITTLE_BYTE NDNBOOST_VERSION_NUMBER_AVAILABLE
#       endif
#       if (__BYTE_ORDER == __PDP_ENDIAN)
#           undef NDNBOOST_ENDIAN_LITTLE_WORD
#           define NDNBOOST_ENDIAN_LITTLE_WORD NDNBOOST_VERSION_NUMBER_AVAILABLE
#       endif
#   endif
#   if !defined(__BYTE_ORDER) && defined(_BYTE_ORDER)
#       if (_BYTE_ORDER == _BIG_ENDIAN)
#           undef NDNBOOST_ENDIAN_BIG_BYTE
#           define NDNBOOST_ENDIAN_BIG_BYTE NDNBOOST_VERSION_NUMBER_AVAILABLE
#       endif
#       if (_BYTE_ORDER == _LITTLE_ENDIAN)
#           undef NDNBOOST_ENDIAN_LITTLE_BYTE
#           define NDNBOOST_ENDIAN_LITTLE_BYTE NDNBOOST_VERSION_NUMBER_AVAILABLE
#       endif
#       if (_BYTE_ORDER == _PDP_ENDIAN)
#           undef NDNBOOST_ENDIAN_LITTLE_WORD
#           define NDNBOOST_ENDIAN_LITTLE_WORD NDNBOOST_VERSION_NUMBER_AVAILABLE
#       endif
#   endif
#endif

/* Built-in byte-swpped big-endian macros.
 */
#if !NDNBOOST_ENDIAN_BIG_BYTE && !NDNBOOST_ENDIAN_BIG_WORD && \
    !NDNBOOST_ENDIAN_LITTLE_BYTE && !NDNBOOST_ENDIAN_LITTLE_WORD
#   if (defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)) || \
       (defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)) || \
        defined(__ARMEB__) || \
        defined(__THUMBEB__) || \
        defined(__AARCH64EB__) || \
        defined(_MIPSEB) || \
        defined(__MIPSEB) || \
        defined(__MIPSEB__)
#       undef NDNBOOST_ENDIAN_BIG_BYTE
#       define NDNBOOST_ENDIAN_BIG_BYTE NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

/* Built-in byte-swpped little-endian macros.
 */
#if !NDNBOOST_ENDIAN_BIG_BYTE && !NDNBOOST_ENDIAN_BIG_WORD && \
    !NDNBOOST_ENDIAN_LITTLE_BYTE && !NDNBOOST_ENDIAN_LITTLE_WORD
#   if (defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)) || \
       (defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)) || \
        defined(__ARMEL__) || \
        defined(__THUMBEL__) || \
        defined(__AARCH64EL__) || \
        defined(_MIPSEL) || \
        defined(__MIPSEL) || \
        defined(__MIPSEL__)
#       undef NDNBOOST_ENDIAN_LITTLE_BYTE
#       define NDNBOOST_ENDIAN_LITTLE_BYTE NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

/* Some architectures are strictly one endianess (as opposed
 * the current common bi-endianess).
 */
#if !NDNBOOST_ENDIAN_BIG_BYTE && !NDNBOOST_ENDIAN_BIG_WORD && \
    !NDNBOOST_ENDIAN_LITTLE_BYTE && !NDNBOOST_ENDIAN_LITTLE_WORD
#   include <ndnboost/predef/architecture.h>
#   if NDNBOOST_ARCH_M68K || \
        NDNBOOST_ARCH_PARISK || \
        NDNBOOST_ARCH_SPARC || \
        NDNBOOST_ARCH_SYS370 || \
        NDNBOOST_ARCH_SYS390 || \
        NDNBOOST_ARCH_Z
#       undef NDNBOOST_ENDIAN_BIG_BYTE
#       define NDNBOOST_ENDIAN_BIG_BYTE NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#   if NDNBOOST_ARCH_AMD64 || \
        NDNBOOST_ARCH_IA64 || \
        NDNBOOST_ARCH_X86 || \
        NDNBOOST_ARCH_BLACKFIN
#       undef NDNBOOST_ENDIAN_LITTLE_BYTE
#       define NDNBOOST_ENDIAN_LITTLE_BYTE NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

/* Windows on ARM, if not otherwise detected/specified, is always
 * byte-swaped little-endian.
 */
#if !NDNBOOST_ENDIAN_BIG_BYTE && !NDNBOOST_ENDIAN_BIG_WORD && \
    !NDNBOOST_ENDIAN_LITTLE_BYTE && !NDNBOOST_ENDIAN_LITTLE_WORD
#   if NDNBOOST_ARCH_ARM
#       include <ndnboost/predef/os/windows.h>
#       if NDNBOOST_OS_WINDOWS
#           undef NDNBOOST_ENDIAN_LITTLE_BYTE
#           define NDNBOOST_ENDIAN_LITTLE_BYTE NDNBOOST_VERSION_NUMBER_AVAILABLE
#       endif
#   endif
#endif

#if NDNBOOST_ENDIAN_BIG_BYTE
#   define NDNBOOST_ENDIAN_BIG_BYTE_AVAILABLE
#endif
#if NDNBOOST_ENDIAN_BIG_WORD
#   define NDNBOOST_ENDIAN_BIG_WORD_BYTE_AVAILABLE
#endif
#if NDNBOOST_ENDIAN_LITTLE_BYTE
#   define NDNBOOST_ENDIAN_LITTLE_BYTE_AVAILABLE
#endif
#if NDNBOOST_ENDIAN_LITTLE_WORD
#   define NDNBOOST_ENDIAN_LITTLE_WORD_BYTE_AVAILABLE
#endif

#define NDNBOOST_ENDIAN_BIG_BYTE_NAME "Byte-Swapped Big-Endian"
#define NDNBOOST_ENDIAN_BIG_WORD_NAME "Word-Swapped Big-Endian"
#define NDNBOOST_ENDIAN_LITTLE_BYTE_NAME "Byte-Swapped Little-Endian"
#define NDNBOOST_ENDIAN_LITTLE_WORD_NAME "Word-Swapped Little-Endian"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_ENDIAN_BIG_BYTE,NDNBOOST_ENDIAN_BIG_BYTE_NAME)

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_ENDIAN_BIG_WORD,NDNBOOST_ENDIAN_BIG_WORD_NAME)

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_ENDIAN_LITTLE_BYTE,NDNBOOST_ENDIAN_LITTLE_BYTE_NAME)

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_ENDIAN_LITTLE_WORD,NDNBOOST_ENDIAN_LITTLE_WORD_NAME)


#endif
