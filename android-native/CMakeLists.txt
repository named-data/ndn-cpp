# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             ndn-cpp

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/ndn-cpp/contrib/apache/apr_base64.c
             src/ndn-cpp/contrib/openssl/sha256.c
             src/ndn-cpp/src/c/control-parameters_c.c
             src/ndn-cpp/src/c/forwarding-flags.c
             src/ndn-cpp/src/c/interest_c.c
             src/ndn-cpp/src/c/name_c.c
             src/ndn-cpp/src/c/network-nack_c.c
             src/ndn-cpp/src/c/encoding/element-reader.c
             src/ndn-cpp/src/c/encoding/tlv-0_2-wire-format_c.c
             src/ndn-cpp/src/c/encoding/tlv/tlv-control-parameters.c
             src/ndn-cpp/src/c/encoding/tlv/tlv-control-response.c
             src/ndn-cpp/src/c/encoding/tlv/tlv-data.c
             src/ndn-cpp/src/c/encoding/tlv/tlv-delegation-set.c
             src/ndn-cpp/src/c/encoding/tlv/tlv-decoder.c
             src/ndn-cpp/src/c/encoding/tlv/tlv-encoder.c
             src/ndn-cpp/src/c/encoding/tlv/tlv-encrypted-content.c
             src/ndn-cpp/src/c/encoding/tlv/tlv-interest.c
             src/ndn-cpp/src/c/encoding/tlv/tlv-key-locator.c
             src/ndn-cpp/src/c/encoding/tlv/tlv-lp-packet.c
             src/ndn-cpp/src/c/encoding/tlv/tlv-name.c
             src/ndn-cpp/src/c/encoding/tlv/tlv-signature-info.c
             src/ndn-cpp/src/c/encoding/tlv/tlv-structure-decoder.c
             src/ndn-cpp/src/c/encrypt/algo/aes-algorithm_c.c
             src/ndn-cpp/src/c/lp/incoming-face-id_c.c
             src/ndn-cpp/src/c/security/ec-private-key.c
             src/ndn-cpp/src/c/security/ec-public-key.c
             src/ndn-cpp/src/c/security/rsa-private-key.c
             src/ndn-cpp/src/c/security/rsa-public-key.c
             src/ndn-cpp/src/c/transport/socket-transport.c
             src/ndn-cpp/src/c/transport/tcp-transport_c.c
             src/ndn-cpp/src/c/util/blob_c.c
             src/ndn-cpp/src/c/util/crypto.c
             src/ndn-cpp/src/c/util/dynamic-uint8-array.c
             src/ndn-cpp/src/c/util/ndn_memory.c
             src/ndn-cpp/src/c/util/ndn_realloc.c
             src/ndn-cpp/src/c/util/time.c
             src/ndn-cpp/src/lite/control-parameters-lite.cpp
             src/ndn-cpp/src/lite/control-response-lite.cpp
             src/ndn-cpp/src/lite/data-lite.cpp
             src/ndn-cpp/src/lite/delegation-set-lite.cpp
             src/ndn-cpp/src/lite/exclude-lite.cpp
             src/ndn-cpp/src/lite/forwarding-flags-lite.cpp
             src/ndn-cpp/src/lite/interest-lite.cpp
             src/ndn-cpp/src/lite/key-locator-lite.cpp
             src/ndn-cpp/src/lite/meta-info-lite.cpp
             src/ndn-cpp/src/lite/name-lite.cpp
             src/ndn-cpp/src/lite/network-nack-lite.cpp
             src/ndn-cpp/src/lite/signature-lite.cpp
             src/ndn-cpp/src/lite/encoding/element-listener-lite.cpp
             src/ndn-cpp/src/lite/encoding/tlv-0_2-wire-format-lite.cpp
             src/ndn-cpp/src/lite/encrypt/encrypted-content-lite.cpp
             src/ndn-cpp/src/lite/encrypt/algo/aes-algorithm-lite.cpp
             src/ndn-cpp/src/lite/encrypt/algo/encrypt-params-lite.cpp
             src/ndn-cpp/src/lite/lp/incoming-face-id-lite.cpp
             src/ndn-cpp/src/lite/lp/lp-packet-lite.cpp
             src/ndn-cpp/src/lite/security/ec-private-key-lite.cpp
             src/ndn-cpp/src/lite/security/ec-public-key-lite.cpp
             src/ndn-cpp/src/lite/security/rsa-private-key-lite.cpp
             src/ndn-cpp/src/lite/security/rsa-public-key-lite.cpp
             src/ndn-cpp/src/lite/security/validity-period-lite.cpp
             src/ndn-cpp/src/lite/transport/tcp-transport-lite.cpp
             src/ndn-cpp/src/lite/transport/udp-transport-lite.cpp
             src/ndn-cpp/src/lite/util/blob-lite.cpp
             src/ndn-cpp/src/lite/util/crypto-lite.cpp
             src/ndn-cpp/src/lite/util/dynamic-malloc-uint8-array-lite.cpp
             src/ndn-cpp/src/lite/util/dynamic-uint8-array-lite.cpp
             )

include_directories(src/ndn-cpp/include)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       ndn-cpp

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )
