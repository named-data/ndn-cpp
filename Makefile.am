include aminclude.am

## Place generated object files (.o) into the same directory as their source
## files, in order to avoid collisions when non-recursive make is used.
AUTOMAKE_OPTIONS = subdir-objects

## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4

AM_CPPFLAGS=${BOOST_CPPFLAGS}
AM_LDFLAGS=${BOOST_SYSTEM_LIB} ${BOOST_REGEX_LIB}

# "make install" will also install the include headers.
SUBDIRS = include

lib_LTLIBRARIES = libndn-c.la libndn-cpp.la

check_PROGRAMS = bin/unit-tests/test-control-parameters-encode-decode \
   bin/unit-tests/test-data-methods \
  bin/unit-tests/test-der-encode-decode bin/unit-tests/test-face-methods \
  bin/unit-tests/test-identity-methods bin/unit-tests/test-interest-methods \
  bin/unit-tests/test-name-conventions bin/unit-tests/test-name-methods \
  bin/unit-tests/test-policy-manager bin/unit-tests/test-verification-rules

noinst_PROGRAMS = bin/test-chrono-chat bin/test-echo-consumer \
  bin/test-encode-decode-benchmark bin/test-encode-decode-data \
  bin/test-encode-decode-fib-entry bin/test-encode-decode-interest \
  bin/test-get-async bin/test-list-channels bin/test-list-faces bin/test-list-rib \
  bin/test-publish-async-nfd bin/test-register-route bin/analog-reading-consumer \
  bin/basic-insertion bin/watched-insertion

# Public C headers.
# NOTE: If a new directory is added, then add it to ndn_cpp_c_headers in include/Makefile.am.
ndn_cpp_c_headers = \
  include/ndn-cpp/ndn-cpp-config.h \
  include/ndn-cpp/c/common.h \
  include/ndn-cpp/c/data-types.h \
  include/ndn-cpp/c/errors.h \
  include/ndn-cpp/c/forwarding-flags.h \
  include/ndn-cpp/c/interest-types.h \
  include/ndn-cpp/c/key-types.h \
  include/ndn-cpp/c/name-types.h \
  include/ndn-cpp/c/publisher-public-key-digest-types.h \
  include/ndn-cpp/c/encoding/element-reader-types.h \
  include/ndn-cpp/c/transport/transport-types.h \
  include/ndn-cpp/c/util/blob-types.h \
  include/ndn-cpp/c/util/dynamic-uint8-array-types.h

# Public C++ headers.
# NOTE: If a new directory is added, then add it to ndn_cpp_cpp_headers in include/Makefile.am.
ndn_cpp_cpp_headers = \
  include/ndn-cpp/common.hpp \
  include/ndn-cpp/data.hpp \
  include/ndn-cpp/digest-sha256-signature.hpp \
  include/ndn-cpp/exclude.hpp \
  include/ndn-cpp/face.hpp \
  include/ndn-cpp/forwarding-entry.hpp \
  include/ndn-cpp/forwarding-flags.hpp \
  include/ndn-cpp/interest-filter.hpp \
  include/ndn-cpp/interest.hpp \
  include/ndn-cpp/key-locator.hpp \
  include/ndn-cpp/meta-info.hpp \
  include/ndn-cpp/name.hpp \
  include/ndn-cpp/control-parameters.hpp \
  include/ndn-cpp/publisher-public-key-digest.hpp \
  include/ndn-cpp/signature.hpp \
  include/ndn-cpp/sha256-with-ecdsa-signature.hpp \
  include/ndn-cpp/sha256-with-rsa-signature.hpp \
  include/ndn-cpp/encoding/binary-xml-wire-format.hpp \
  include/ndn-cpp/encoding/oid.hpp \
  include/ndn-cpp/encoding/protobuf-tlv.hpp \
  include/ndn-cpp/encoding/tlv-0_1-wire-format.hpp \
  include/ndn-cpp/encoding/tlv-0_1_1-wire-format.hpp \
  include/ndn-cpp/encoding/tlv-wire-format.hpp \
  include/ndn-cpp/encoding/wire-format.hpp \
  include/ndn-cpp/lite/common-lite.hpp \
  include/ndn-cpp/lite/data-lite.hpp \
  include/ndn-cpp/lite/exclude-lite.hpp \
  include/ndn-cpp/lite/interest-lite.hpp \
  include/ndn-cpp/lite/key-locator-lite.hpp \
  include/ndn-cpp/lite/meta-info-lite.hpp \
  include/ndn-cpp/lite/name-lite.hpp \
  include/ndn-cpp/lite/signature-lite.hpp \
  include/ndn-cpp/lite/encoding/element-listener-lite.hpp \
  include/ndn-cpp/lite/encoding/tlv-0_1_1-wire-format-lite.hpp \
  include/ndn-cpp/lite/transport/tcp-transport-lite.hpp \
  include/ndn-cpp/lite/transport/udp-transport-lite.hpp \
  include/ndn-cpp/lite/transport/unix-transport-lite.hpp \
  include/ndn-cpp/lite/util/blob-lite.hpp \
  include/ndn-cpp/lite/util/dynamic-uint8-array-lite.hpp \
  include/ndn-cpp/security/security-common.hpp \
  include/ndn-cpp/security/key-chain.hpp \
  include/ndn-cpp/security/security-exception.hpp \
  include/ndn-cpp/security/certificate/certificate-extension.hpp \
  include/ndn-cpp/security/certificate/certificate-subject-description.hpp \
  include/ndn-cpp/security/certificate/certificate.hpp \
  include/ndn-cpp/security/certificate/identity-certificate.hpp \
  include/ndn-cpp/security/certificate/public-key.hpp \
  include/ndn-cpp/security/identity/basic-identity-storage.hpp \
  include/ndn-cpp/security/identity/file-private-key-storage.hpp \
  include/ndn-cpp/security/identity/identity-manager.hpp \
  include/ndn-cpp/security/identity/identity-storage.hpp \
  include/ndn-cpp/security/identity/memory-identity-storage.hpp \
  include/ndn-cpp/security/identity/memory-private-key-storage.hpp \
  include/ndn-cpp/security/identity/osx-private-key-storage.hpp \
  include/ndn-cpp/security/identity/private-key-storage.hpp \
  include/ndn-cpp/security/policy/config-policy-manager.hpp \
  include/ndn-cpp/security/policy/no-verify-policy-manager.hpp \
  include/ndn-cpp/security/policy/policy-manager.hpp \
  include/ndn-cpp/security/policy/self-verify-policy-manager.hpp \
  include/ndn-cpp/security/policy/validation-request.hpp \
  include/ndn-cpp/sync/chrono-sync2013.hpp \
  include/ndn-cpp/transport/tcp-transport.hpp \
  include/ndn-cpp/transport/transport.hpp \
  include/ndn-cpp/transport/udp-transport.hpp \
  include/ndn-cpp/transport/unix-transport.hpp \
  include/ndn-cpp/util/blob.hpp \
  include/ndn-cpp/util/change-counter.hpp \
  include/ndn-cpp/util/memory-content-cache.hpp \
  include/ndn-cpp/util/segment-fetcher.hpp \
  include/ndn-cpp/util/signed-blob.hpp

# Just the C code.
libndn_c_la_SOURCES = ${ndn_cpp_c_headers} \
  src/c/data.h \
  src/c/errors.c src/c/errors.h \
  src/c/forwarding-entry.h \
  src/c/forwarding-flags.c src/c/forwarding-flags-impl.h \
  src/c/interest.c src/c/interest.h \
  src/c/key-locator.h \
  src/c/name.c src/c/name.h \
  src/c/control-parameters.h \
  src/c/publisher-public-key-digest.h \
  src/c/encoding/binary-xml-data.c src/c/encoding/binary-xml-data.h \
  src/c/encoding/binary-xml-decoder.c src/c/encoding/binary-xml-decoder.h \
  src/c/encoding/binary-xml-encoder.c src/c/encoding/binary-xml-encoder.h \
  src/c/encoding/binary-xml-forwarding-entry.c src/c/encoding/binary-xml-forwarding-entry.h \
  src/c/encoding/binary-xml-interest.c src/c/encoding/binary-xml-interest.h \
  src/c/encoding/binary-xml-key-locator.c src/c/encoding/binary-xml-key-locator.h \
  src/c/encoding/binary-xml-name.c src/c/encoding/binary-xml-name.h \
  src/c/encoding/binary-xml-publisher-public-key-digest.c src/c/encoding/binary-xml-publisher-public-key-digest.h \
  src/c/encoding/binary-xml-structure-decoder.c src/c/encoding/binary-xml-structure-decoder.h \
  src/c/encoding/binary-xml.h \
  src/c/encoding/element-listener.h \
  src/c/encoding/element-reader.c src/c/encoding/element-reader.h \
  src/c/encoding/tlv-0_1_1-wire-format.c src/c/encoding/tlv-0_1_1-wire-format.h \
  src/c/encoding/tlv/tlv-data.c src/c/encoding/tlv/tlv-data.h \
  src/c/encoding/tlv/tlv-decoder.c src/c/encoding/tlv/tlv-decoder.h \
  src/c/encoding/tlv/tlv-encoder.c src/c/encoding/tlv/tlv-encoder.h \
  src/c/encoding/tlv/tlv-interest.c src/c/encoding/tlv/tlv-interest.h \
  src/c/encoding/tlv/tlv-key-locator.c src/c/encoding/tlv/tlv-key-locator.h \
  src/c/encoding/tlv/tlv-name.c src/c/encoding/tlv/tlv-name.h \
  src/c/encoding/tlv/tlv-control-parameters.c src/c/encoding/tlv/tlv-control-parameters.h \
  src/c/encoding/tlv/tlv-signature-info.c src/c/encoding/tlv/tlv-signature-info.h \
  src/c/encoding/tlv/tlv-structure-decoder.c src/c/encoding/tlv/tlv-structure-decoder.h \
  src/c/encoding/tlv/tlv.h \
  src/c/transport/socket-transport.c src/c/transport/socket-transport.h \
  src/c/transport/tcp-transport.c src/c/transport/tcp-transport.h \
  src/c/transport/udp-transport.h \
  src/c/transport/unix-transport.h \
  src/c/util/blob.h \
  src/c/util/crypto.c src/c/util/crypto.h \
  src/c/util/dynamic-uint8-array.c src/c/util/dynamic-uint8-array.h \
  src/c/util/ndn_memory.c src/c/util/ndn_memory.h \
  src/c/util/ndn_realloc.c src/c/util/ndn_realloc.h \
  src/c/util/time.c src/c/util/time.h

# C++ code and also the C code.
libndn_cpp_la_SOURCES = ${libndn_c_la_SOURCES} ${ndn_cpp_cpp_headers} \
  src/common.cpp \
  src/data.cpp \
  src/digest-sha256-signature.cpp \
  src/exclude.cpp \
  src/face.cpp \
  src/forwarding-entry.cpp \
  src/forwarding-flags.cpp \
  src/interest-filter.cpp \
  src/interest.cpp \
  src/key-locator.cpp \
  src/meta-info.cpp \
  src/name.cpp \
  src/node.cpp src/node.hpp \
  src/control-parameters.cpp \
  src/publisher-public-key-digest.cpp \
  src/signature.cpp \
  src/sha256-with-ecdsa-signature.cpp \
  src/sha256-with-rsa-signature.cpp \
  src/encoding/base64.cpp src/encoding/base64.hpp \
  src/encoding/binary-xml-decoder.hpp \
  src/encoding/binary-xml-encoder.hpp \
  src/encoding/binary-xml-wire-format.cpp \
  src/encoding/element-listener.cpp src/encoding/element-listener.hpp \
  src/encoding/oid.cpp \
  src/encoding/protobuf-tlv.cpp \
  src/encoding/tlv-0_1-wire-format.cpp \
  src/encoding/tlv-0_1_1-wire-format.cpp \
  src/encoding/tlv-decoder.hpp \
  src/encoding/tlv-encoder.hpp \
  src/encoding/tlv-wire-format.cpp \
  src/encoding/wire-format.cpp \
  src/encoding/der/der-exception.cpp src/encoding/der/der-exception.hpp \
  src/encoding/der/der-node.cpp src/encoding/der/der-node.hpp \
  src/lite/data-lite.cpp \
  src/lite/exclude-lite.cpp \
  src/lite/interest-lite.cpp \
  src/lite/key-locator-lite.cpp \
  src/lite/meta-info-lite.cpp \
  src/lite/name-lite.cpp \
  src/lite/signature-lite.cpp \
  src/lite/encoding/element-listener-lite.cpp \
  src/lite/encoding/tlv-0_1_1-wire-format-lite.cpp \
  src/lite/transport/tcp-transport-lite.cpp \
  src/lite/transport/udp-transport-lite.cpp \
  src/lite/transport/unix-transport-lite.cpp \
  src/lite/util/blob-lite.cpp \
  src/lite/util/dynamic-uint8-array-lite.cpp \
  src/security/key-chain.cpp \
  src/security/security-exception.cpp \
  src/security/certificate/certificate-extension.cpp \
  src/security/certificate/certificate-subject-description.cpp \
  src/security/certificate/certificate.cpp \
  src/security/certificate/identity-certificate.cpp \
  src/security/certificate/public-key.cpp \
  src/security/identity/basic-identity-storage.cpp \
  src/security/identity/file-private-key-storage.cpp \
  src/security/identity/identity-manager.cpp \
  src/security/identity/identity-storage.cpp \
  src/security/identity/memory-identity-storage.cpp \
  src/security/identity/memory-private-key-storage.cpp \
  src/security/identity/osx-private-key-storage.cpp \
  src/security/policy/config-policy-manager.cpp \
  src/security/policy/no-verify-policy-manager.cpp \
  src/security/policy/policy-manager.cpp \
  src/security/policy/self-verify-policy-manager.cpp \
  src/sync/chrono-sync2013.cpp \
  src/sync/digest-tree.cpp src/sync/digest-tree.hpp \
  src/sync/sync-state.pb.cc src/sync/sync-state.pb.h \
  src/transport/tcp-transport.cpp \
  src/transport/transport.cpp \
  src/transport/udp-transport.cpp \
  src/transport/unix-transport.cpp \
  src/util/blob-stream.hpp \
  src/util/blob.cpp \
  src/util/boost-info-parser.cpp src/util/boost-info-parser.hpp \
  src/util/command-interest-generator.cpp src/util/command-interest-generator.hpp \
  src/util/dynamic-uint8-vector.cpp src/util/dynamic-uint8-vector.hpp \
  src/util/logging.cpp src/util/logging.hpp \
  src/util/memory-content-cache.cpp \
  src/util/ndn-regex-matcher.cpp src/util/ndn-regex-matcher.hpp \
  src/util/segment-fetcher.cpp

bin_test_chrono_chat_SOURCES = examples/test-chrono-chat.cpp examples/chatbuf.pb.cc
bin_test_chrono_chat_LDADD = libndn-cpp.la

bin_test_echo_consumer_SOURCES = examples/test-echo-consumer.cpp
bin_test_echo_consumer_LDADD = libndn-cpp.la

bin_test_encode_decode_benchmark_SOURCES = examples/test-encode-decode-benchmark.cpp
bin_test_encode_decode_benchmark_LDADD = libndn-cpp.la

bin_test_encode_decode_data_SOURCES = examples/test-encode-decode-data.cpp
bin_test_encode_decode_data_LDADD = libndn-cpp.la

bin_test_encode_decode_fib_entry_SOURCES = examples/test-encode-decode-fib-entry.cpp examples/fib-entry.pb.cc
bin_test_encode_decode_fib_entry_LDADD = libndn-cpp.la

bin_test_encode_decode_interest_SOURCES = examples/test-encode-decode-interest.cpp
bin_test_encode_decode_interest_LDADD = libndn-cpp.la

bin_test_get_async_SOURCES = examples/test-get-async.cpp
bin_test_get_async_LDADD = libndn-cpp.la

bin_test_list_channels_SOURCES = examples/test-list-channels.cpp examples/channel-status.pb.cc
bin_test_list_channels_LDADD = libndn-cpp.la

bin_test_list_faces_SOURCES = examples/test-list-faces.cpp examples/face-status.pb.cc
bin_test_list_faces_LDADD = libndn-cpp.la

bin_test_list_rib_SOURCES = examples/test-list-rib.cpp examples/rib-entry.pb.cc
bin_test_list_rib_LDADD = libndn-cpp.la

bin_test_publish_async_nfd_SOURCES = examples/test-publish-async-nfd.cpp
bin_test_publish_async_nfd_LDADD = libndn-cpp.la

bin_analog_reading_consumer_SOURCES = examples/arduino/analog-reading-consumer.cpp
bin_analog_reading_consumer_LDADD = libndn-cpp.la

bin_basic_insertion_SOURCES = examples/repo-ng/basic-insertion.cpp \
  examples/repo-ng/repo-command-parameter.pb.cc examples/repo-ng/repo-command-response.pb.cc
bin_basic_insertion_LDADD = libndn-cpp.la

bin_watched_insertion_SOURCES = examples/repo-ng/watched-insertion.cpp \
  examples/repo-ng/repo-command-parameter.pb.cc examples/repo-ng/repo-command-response.pb.cc
bin_watched_insertion_LDADD = libndn-cpp.la

bin_test_register_route_SOURCES = examples/test-register-route.cpp \
  examples/control-parameters.pb.cc examples/face-query-filter.pb.cc \
  examples/face-status.pb.cc
bin_test_register_route_LDADD = libndn-cpp.la

# Unit tests

bin_unit_tests_test_control_parameters_encode_decode_SOURCES = tests/unit-tests/test-control-parameters-encode-decode.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_control_parameters_encode_decode_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_control_parameters_encode_decode_LDADD = libndn-cpp.la

bin_unit_tests_test_data_methods_SOURCES = tests/unit-tests/test-data-methods.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_data_methods_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_data_methods_LDADD = libndn-cpp.la

bin_unit_tests_test_der_encode_decode_SOURCES = tests/unit-tests/test-der-encode-decode.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_der_encode_decode_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_der_encode_decode_LDADD = libndn-cpp.la

bin_unit_tests_test_face_methods_SOURCES = tests/unit-tests/test-face-methods.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_face_methods_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_face_methods_LDADD = libndn-cpp.la

bin_unit_tests_test_identity_methods_SOURCES = tests/unit-tests/test-identity-methods.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_identity_methods_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_identity_methods_LDADD = libndn-cpp.la

bin_unit_tests_test_interest_methods_SOURCES = tests/unit-tests/test-interest-methods.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_interest_methods_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_interest_methods_LDADD = libndn-cpp.la

bin_unit_tests_test_name_conventions_SOURCES = tests/unit-tests/test-name-conventions.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_name_conventions_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_name_conventions_LDADD = libndn-cpp.la

bin_unit_tests_test_name_methods_SOURCES = tests/unit-tests/test-name-methods.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_name_methods_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_name_methods_LDADD = libndn-cpp.la

bin_unit_tests_test_policy_manager_SOURCES = tests/unit-tests/test-policy-manager.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_policy_manager_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_policy_manager_LDADD = libndn-cpp.la

bin_unit_tests_test_verification_rules_SOURCES = tests/unit-tests/test-verification-rules.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_verification_rules_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_verification_rules_LDADD = libndn-cpp.la

TESTS = ${check_PROGRAMS}

dist_noinst_SCRIPTS = autogen.sh
