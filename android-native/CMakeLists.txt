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
             src/ndn-cpp/src/c/errors.c
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
             src/ndn-cpp/src/common.cpp
             src/ndn-cpp/src/control-parameters.cpp
             src/ndn-cpp/src/control-response.cpp
             src/ndn-cpp/src/data.cpp
             src/ndn-cpp/src/delegation-set.cpp
             src/ndn-cpp/src/digest-sha256-signature.cpp
             src/ndn-cpp/src/exclude.cpp
             src/ndn-cpp/src/face.cpp
             src/ndn-cpp/src/generic-signature.cpp
             src/ndn-cpp/src/hmac-with-sha256-signature.cpp
             src/ndn-cpp/src/interest-filter.cpp
             src/ndn-cpp/src/interest.cpp
             src/ndn-cpp/src/key-locator.cpp
             src/ndn-cpp/src/link.cpp
             src/ndn-cpp/src/meta-info.cpp
             src/ndn-cpp/src/name.cpp
             src/ndn-cpp/src/network-nack.cpp
             src/ndn-cpp/src/node.cpp
             src/ndn-cpp/src/signature.cpp
             src/ndn-cpp/src/sha256-with-ecdsa-signature.cpp
             src/ndn-cpp/src/sha256-with-rsa-signature.cpp
             src/ndn-cpp/src/encoding/base64.cpp
             src/ndn-cpp/src/encoding/element-listener.cpp
             src/ndn-cpp/src/encoding/oid.cpp
             src/ndn-cpp/src/encoding/protobuf-tlv.cpp
             src/ndn-cpp/src/encoding/tlv-0_1-wire-format.cpp
             src/ndn-cpp/src/encoding/tlv-0_1_1-wire-format.cpp
             src/ndn-cpp/src/encoding/tlv-0_2-wire-format.cpp
             src/ndn-cpp/src/encoding/tlv-wire-format.cpp
             src/ndn-cpp/src/encoding/wire-format.cpp
             src/ndn-cpp/src/encoding/der/der-exception.cpp
             src/ndn-cpp/src/encoding/der/der-node.cpp
             src/ndn-cpp/src/encrypt/encrypted-content.cpp
             src/ndn-cpp/src/impl/delayed-call-table.cpp
             src/ndn-cpp/src/impl/interest-filter-table.cpp
             src/ndn-cpp/src/impl/pending-interest-table.cpp
             src/ndn-cpp/src/impl/registered-prefix-table.cpp
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
             src/ndn-cpp/src/lp/incoming-face-id.cpp
             src/ndn-cpp/src/lp/lp-packet.cpp
             src/ndn-cpp/src/security/key-chain.cpp
             src/ndn-cpp/src/security/security-exception.cpp
             src/ndn-cpp/src/security/certificate/certificate-extension.cpp
             src/ndn-cpp/src/security/certificate/certificate-subject-description.cpp
             src/ndn-cpp/src/security/certificate/certificate.cpp
             src/ndn-cpp/src/security/certificate/identity-certificate.cpp
             src/ndn-cpp/src/security/certificate/public-key.cpp
             src/ndn-cpp/src/security/identity/basic-identity-storage.cpp
             src/ndn-cpp/src/security/identity/file-private-key-storage.cpp
             src/ndn-cpp/src/security/identity/identity-manager.cpp
             src/ndn-cpp/src/security/identity/identity-storage.cpp
             src/ndn-cpp/src/security/identity/memory-identity-storage.cpp
             src/ndn-cpp/src/security/identity/private-key-storage.cpp
             src/ndn-cpp/src/security/policy/config-policy-manager.cpp
             src/ndn-cpp/src/security/policy/no-verify-policy-manager.cpp
             src/ndn-cpp/src/security/policy/policy-manager.cpp
             src/ndn-cpp/src/security/policy/self-verify-policy-manager.cpp
             src/ndn-cpp/src/transport/async-tcp-transport.cpp
             src/ndn-cpp/src/transport/async-unix-transport.cpp
             src/ndn-cpp/src/transport/tcp-transport.cpp
             src/ndn-cpp/src/transport/transport.cpp
             src/ndn-cpp/src/transport/udp-transport.cpp
             src/ndn-cpp/src/transport/unix-transport.cpp
             src/ndn-cpp/src/util/boost-info-parser.cpp
             src/ndn-cpp/src/util/command-interest-generator.cpp
             src/ndn-cpp/src/util/config-file.cpp
             src/ndn-cpp/src/util/dynamic-uint8-vector.cpp
             src/ndn-cpp/src/util/exponential-re-express.cpp
             src/ndn-cpp/src/util/logging.cpp
             src/ndn-cpp/src/util/memory-content-cache.cpp
             src/ndn-cpp/src/util/regex/ndn-regex-backref-manager.cpp
             src/ndn-cpp/src/util/regex/ndn-regex-backref-matcher.cpp
             src/ndn-cpp/src/util/regex/ndn-regex-component-matcher.cpp
             src/ndn-cpp/src/util/regex/ndn-regex-component-set-matcher.cpp
             src/ndn-cpp/src/util/regex/ndn-regex-matcher-base.cpp
             src/ndn-cpp/src/util/regex/ndn-regex-pattern-list-matcher.cpp
             src/ndn-cpp/src/util/regex/ndn-regex-pseudo-matcher.cpp
             src/ndn-cpp/src/util/regex/ndn-regex-repeat-matcher.cpp
             src/ndn-cpp/src/util/regex/ndn-regex-top-matcher.cpp
             src/ndn-cpp/src/util/segment-fetcher.cpp
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
