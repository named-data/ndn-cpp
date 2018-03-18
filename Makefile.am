include aminclude.am

## Place generated object files (.o) into the same directory as their source
## files, in order to avoid collisions when non-recursive make is used.
AUTOMAKE_OPTIONS = subdir-objects

## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4

AM_CPPFLAGS=${BOOST_CPPFLAGS}
AM_LDFLAGS=${BOOST_SYSTEM_LIB} ${BOOST_REGEX_LIB} ${BOOST_ASIO_LIB}

# "make install" will also install the include headers.
SUBDIRS = include

lib_LTLIBRARIES = libndn-c.la libndn-cpp.la libndn-cpp-tools.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libndn-cpp.pc

# Make ndn-cpp-config.h so we recompile the .pb.cc after Protobuf is installed or removed.
%.pb.cc: %.proto include/ndn-cpp/ndn-cpp-config.h
	if test "no${PROTOC}" == "no"; then \
	  cat protobuf-stub.txt > $@ ; \
	else \
	  $(PROTOC) --proto_path=$(dir $<) --cpp_out=$(dir $<) $< ; \
	fi

# On clean, remake the Protobuf code.
CLEANFILES = \
  examples/channel-status.pb.cc \
  examples/chatbuf.pb.cc \
  examples/control-parameters.pb.cc \
  examples/face-query-filter.pb.cc \
  examples/face-status.pb.cc \
  examples/fib-entry.pb.cc \
  examples/repo-ng/repo-command-parameter.pb.cc \
  examples/repo-ng/repo-command-response.pb.cc \
  examples/rib-entry.pb.cc \
  src/sync/sync-state.pb.cc \
  tools/usersync/content-meta-info.pb.cc


check_PROGRAMS = bin/unit-tests/test-aes-algorithm bin/unit-tests/test-certificate \
  bin/unit-tests/test-consumer bin/unit-tests/test-consumer-db \
  bin/unit-tests/test-control-parameters-encode-decode \
  bin/unit-tests/test-control-response \
  bin/unit-tests/test-data-methods bin/unit-tests/test-der-encode-decode \
  bin/unit-tests/test-encrypted-content bin/unit-tests/test-encryptor \
  bin/unit-tests/test-face-methods bin/unit-tests/test-group-manager-db \
  bin/unit-tests/test-group-manager bin/unit-tests/test-identity-methods \
  bin/unit-tests/test-interest-methods \
  bin/unit-tests/test-interval bin/unit-tests/test-key-chain \
  bin/unit-tests/test-link bin/unit-tests/test-name-conventions \
  bin/unit-tests/test-name-methods bin/unit-tests/test-pib-certificate-container \
  bin/unit-tests/test-pib-identity-container \
  bin/unit-tests/test-pib-identity-impl bin/unit-tests/test-pib-impl \
  bin/unit-tests/test-pib-key-container bin/unit-tests/test-pib-key-impl \
  bin/unit-tests/test-policy-manager bin/unit-tests/test-policy-manager-v2 \
  bin/unit-tests/test-producer \
  bin/unit-tests/test-producer-db bin/unit-tests/test-regex \
  bin/unit-tests/test-registration-callbacks \
  bin/unit-tests/test-repetitive-interval \
  bin/unit-tests/test-rsa-algorithm bin/unit-tests/test-schedule \
  bin/unit-tests/test-signing-info bin/unit-tests/test-tpm-back-ends \
  bin/unit-tests/test-tpm-private-key bin/unit-tests/test-validation-policy-command-interest \
  bin/unit-tests/test-validation-policy-config bin/unit-tests/test-validator-null \
  bin/unit-tests/test-validator bin/unit-tests/test-verification-rules

noinst_PROGRAMS = bin/test-channel-discovery bin/test-chrono-chat \
  bin/test-echo-consumer bin/test-echo-consumer-lite \
  bin/test-encode-decode-benchmark bin/test-encode-decode-data \
  bin/test-encode-decode-fib-entry bin/test-encode-decode-interest \
  bin/test-generalized-content bin/test-get-async bin/test-get-async-threadsafe \
  bin/test-list-channels bin/test-list-faces bin/test-list-rib \
  bin/test-prefix-discovery \
  bin/test-publish-async-nfd bin/test-publish-async-nfd-lite \
  bin/test-register-route bin/test-sign-verify-data-hmac \
  bin/analog-reading-consumer bin/basic-insertion bin/watched-insertion

# Public C headers.
# NOTE: If a new directory is added, then add it to ndn_cpp_c_headers in include/Makefile.am.
ndn_cpp_c_headers = \
  include/ndn-cpp/ndn-cpp-config.h \
  include/ndn-cpp/c/common.h \
  include/ndn-cpp/c/control-parameters-types.h \
  include/ndn-cpp/c/control-response-types.h \
  include/ndn-cpp/c/data-types.h \
  include/ndn-cpp/c/delegation-set-types.h \
  include/ndn-cpp/c/errors.h \
  include/ndn-cpp/c/forwarding-flags.h \
  include/ndn-cpp/c/interest-types.h \
  include/ndn-cpp/c/key-types.h \
  include/ndn-cpp/c/name-types.h \
  include/ndn-cpp/c/network-nack-types.h \
  include/ndn-cpp/c/encoding/element-reader-types.h \
  include/ndn-cpp/c/encrypt/encrypted-content-types.h \
  include/ndn-cpp/c/encrypt/algo/encrypt-params-types.h \
  include/ndn-cpp/c/lp/congestion-mark-types.h \
  include/ndn-cpp/c/lp/incoming-face-id-types.h \
  include/ndn-cpp/c/lp/lp-packet-types.h \
  include/ndn-cpp/c/security/ec-private-key-types.h \
  include/ndn-cpp/c/security/ec-public-key-types.h \
  include/ndn-cpp/c/security/rsa-private-key-types.h \
  include/ndn-cpp/c/security/rsa-public-key-types.h \
  include/ndn-cpp/c/security/validity-period-types.h \
  include/ndn-cpp/c/transport/transport-types.h \
  include/ndn-cpp/c/util/blob-types.h \
  include/ndn-cpp/c/util/dynamic-uint8-array-types.h

# Public C++ headers.
# NOTE: If a new directory is added, then add it to ndn_cpp_cpp_headers in include/Makefile.am.
ndn_cpp_cpp_headers = \
  include/ndn-cpp/common.hpp \
  include/ndn-cpp/control-parameters.hpp \
  include/ndn-cpp/control-response.hpp \
  include/ndn-cpp/data.hpp \
  include/ndn-cpp/delegation-set.hpp \
  include/ndn-cpp/digest-sha256-signature.hpp \
  include/ndn-cpp/exclude.hpp \
  include/ndn-cpp/face.hpp \
  include/ndn-cpp/forwarding-flags.hpp \
  include/ndn-cpp/generic-signature.hpp \
  include/ndn-cpp/hmac-with-sha256-signature.hpp \
  include/ndn-cpp/interest-filter.hpp \
  include/ndn-cpp/interest.hpp \
  include/ndn-cpp/key-locator.hpp \
  include/ndn-cpp/link.hpp \
  include/ndn-cpp/meta-info.hpp \
  include/ndn-cpp/name.hpp \
  include/ndn-cpp/network-nack.hpp \
  include/ndn-cpp/sha256-with-ecdsa-signature.hpp \
  include/ndn-cpp/sha256-with-rsa-signature.hpp \
  include/ndn-cpp/signature.hpp \
  include/ndn-cpp/threadsafe-face.hpp \
  include/ndn-cpp/encoding/oid.hpp \
  include/ndn-cpp/encoding/protobuf-tlv.hpp \
  include/ndn-cpp/encoding/tlv-0_1-wire-format.hpp \
  include/ndn-cpp/encoding/tlv-0_1_1-wire-format.hpp \
  include/ndn-cpp/encoding/tlv-0_2-wire-format.hpp \
  include/ndn-cpp/encoding/tlv-wire-format.hpp \
  include/ndn-cpp/encoding/wire-format.hpp \
  include/ndn-cpp/encrypt/consumer.hpp \
  include/ndn-cpp/encrypt/consumer-db.hpp \
  include/ndn-cpp/encrypt/decrypt-key.hpp \
  include/ndn-cpp/encrypt/encrypt-key.hpp \
  include/ndn-cpp/encrypt/encrypted-content.hpp \
  include/ndn-cpp/encrypt/interval.hpp \
  include/ndn-cpp/encrypt/group-manager.hpp \
  include/ndn-cpp/encrypt/group-manager-db.hpp \
  include/ndn-cpp/encrypt/producer.hpp \
  include/ndn-cpp/encrypt/producer-db.hpp \
  include/ndn-cpp/encrypt/repetitive-interval.hpp \
  include/ndn-cpp/encrypt/schedule.hpp \
  include/ndn-cpp/encrypt/sqlite3-consumer-db.hpp \
  include/ndn-cpp/encrypt/sqlite3-group-manager-db.hpp \
  include/ndn-cpp/encrypt/sqlite3-producer-db.hpp \
  include/ndn-cpp/encrypt/algo/aes-algorithm.hpp \
  include/ndn-cpp/encrypt/algo/encrypt-params.hpp \
  include/ndn-cpp/encrypt/algo/encryptor.hpp \
  include/ndn-cpp/encrypt/algo/rsa-algorithm.hpp \
  include/ndn-cpp/lite/common-lite.hpp \
  include/ndn-cpp/lite/control-parameters-lite.hpp \
  include/ndn-cpp/lite/control-response-lite.hpp \
  include/ndn-cpp/lite/data-lite.hpp \
  include/ndn-cpp/lite/delegation-set-lite.hpp \
  include/ndn-cpp/lite/exclude-lite.hpp \
  include/ndn-cpp/lite/forwarding-flags-lite.hpp \
  include/ndn-cpp/lite/interest-lite.hpp \
  include/ndn-cpp/lite/key-locator-lite.hpp \
  include/ndn-cpp/lite/meta-info-lite.hpp \
  include/ndn-cpp/lite/name-lite.hpp \
  include/ndn-cpp/lite/network-nack-lite.hpp \
  include/ndn-cpp/lite/signature-lite.hpp \
  include/ndn-cpp/lite/encoding/element-listener-lite.hpp \
  include/ndn-cpp/lite/encoding/tlv-0_1_1-wire-format-lite.hpp \
  include/ndn-cpp/lite/encoding/tlv-0_2-wire-format-lite.hpp \
  include/ndn-cpp/lite/encrypt/encrypted-content-lite.hpp \
  include/ndn-cpp/lite/encrypt/algo/aes-algorithm-lite.hpp \
  include/ndn-cpp/lite/encrypt/algo/encrypt-params-lite.hpp \
  include/ndn-cpp/lite/lp/congestion-mark-lite.hpp \
  include/ndn-cpp/lite/lp/incoming-face-id-lite.hpp \
  include/ndn-cpp/lite/lp/lp-packet-lite.hpp \
  include/ndn-cpp/lite/security/ec-private-key-lite.hpp \
  include/ndn-cpp/lite/security/ec-public-key-lite.hpp \
  include/ndn-cpp/lite/security/rsa-private-key-lite.hpp \
  include/ndn-cpp/lite/security/rsa-public-key-lite.hpp \
  include/ndn-cpp/lite/security/validity-period-lite.hpp \
  include/ndn-cpp/lite/transport/tcp-transport-lite.hpp \
  include/ndn-cpp/lite/transport/udp-transport-lite.hpp \
  include/ndn-cpp/lite/transport/unix-transport-lite.hpp \
  include/ndn-cpp/lite/util/blob-lite.hpp \
  include/ndn-cpp/lite/util/crypto-lite.hpp \
  include/ndn-cpp/lite/util/dynamic-malloc-uint8-array-lite.hpp \
  include/ndn-cpp/lite/util/dynamic-uint8-array-lite.hpp \
  include/ndn-cpp/security/security-common.hpp \
  include/ndn-cpp/security/command-interest-signer.hpp \
  include/ndn-cpp/security/key-chain.hpp \
  include/ndn-cpp/security/key-params.hpp \
  include/ndn-cpp/security/safe-bag.hpp \
  include/ndn-cpp/security/security-exception.hpp \
  include/ndn-cpp/security/signing-info.hpp \
  include/ndn-cpp/security/validity-period.hpp \
  include/ndn-cpp/security/validator-config-error.hpp \
  include/ndn-cpp/security/validator-config.hpp \
  include/ndn-cpp/security/validator-null.hpp \
  include/ndn-cpp/security/validity-period.hpp \
  include/ndn-cpp/security/verification-helpers.hpp \
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
  include/ndn-cpp/security/pib/pib-certificate-container.hpp \
  include/ndn-cpp/security/pib/pib-identity.hpp \
  include/ndn-cpp/security/pib/pib-identity-container.hpp \
  include/ndn-cpp/security/pib/pib-impl.hpp \
  include/ndn-cpp/security/pib/pib-key.hpp \
  include/ndn-cpp/security/pib/pib-key-container.hpp \
  include/ndn-cpp/security/pib/pib-memory.hpp \
  include/ndn-cpp/security/pib/pib-sqlite3.hpp \
  include/ndn-cpp/security/pib/pib.hpp \
  include/ndn-cpp/security/policy/config-policy-manager.hpp \
  include/ndn-cpp/security/policy/no-verify-policy-manager.hpp \
  include/ndn-cpp/security/policy/policy-manager.hpp \
  include/ndn-cpp/security/policy/self-verify-policy-manager.hpp \
  include/ndn-cpp/security/policy/validation-request.hpp \
  include/ndn-cpp/security/tpm/helper-osx.hpp \
  include/ndn-cpp/security/tpm/tpm.hpp \
  include/ndn-cpp/security/tpm/tpm-back-end.hpp \
  include/ndn-cpp/security/tpm/tpm-back-end-file.hpp \
  include/ndn-cpp/security/tpm/tpm-back-end-memory.hpp \
  include/ndn-cpp/security/tpm/tpm-back-end-osx.hpp \
  include/ndn-cpp/security/tpm/tpm-key-handle.hpp \
  include/ndn-cpp/security/tpm/tpm-key-handle-memory.hpp \
  include/ndn-cpp/security/tpm/tpm-key-handle-osx.hpp \
  include/ndn-cpp/security/tpm/tpm-private-key.hpp \
  include/ndn-cpp/security/v2/certificate-fetcher-from-network.hpp \
  include/ndn-cpp/security/v2/certificate-fetcher-offline.hpp \
  include/ndn-cpp/security/v2/certificate-fetcher.hpp \
  include/ndn-cpp/security/v2/certificate-request.hpp \
  include/ndn-cpp/security/v2/certificate-storage.hpp \
  include/ndn-cpp/security/v2/certificate-v2.hpp \
  include/ndn-cpp/security/v2/trust-anchor-container.hpp \
  include/ndn-cpp/security/v2/trust-anchor-group.hpp \
  include/ndn-cpp/security/v2/validation-error.hpp \
  include/ndn-cpp/security/v2/validation-policy-accept-all.hpp \
  include/ndn-cpp/security/v2/validation-policy-command-interest.hpp \
  include/ndn-cpp/security/v2/validation-policy-config.hpp \
  include/ndn-cpp/security/v2/validation-policy-from-pib.hpp \
  include/ndn-cpp/security/v2/validation-policy-simple-hierarchy.hpp \
  include/ndn-cpp/security/v2/validation-policy.hpp \
  include/ndn-cpp/security/v2/validation-state.hpp \
  include/ndn-cpp/security/v2/validator.hpp \
  include/ndn-cpp/security/v2/validator-config/config-checker.hpp \
  include/ndn-cpp/security/v2/validator-config/config-filter.hpp \
  include/ndn-cpp/security/v2/validator-config/config-name-relation.hpp \
  include/ndn-cpp/security/v2/validator-config/config-rule.hpp \
  include/ndn-cpp/sync/chrono-sync2013.hpp \
  include/ndn-cpp/transport/async-tcp-transport.hpp \
  include/ndn-cpp/transport/async-unix-transport.hpp \
  include/ndn-cpp/transport/tcp-transport.hpp \
  include/ndn-cpp/transport/transport.hpp \
  include/ndn-cpp/transport/udp-transport.hpp \
  include/ndn-cpp/transport/unix-transport.hpp \
  include/ndn-cpp/util/blob.hpp \
  include/ndn-cpp/util/change-counter.hpp \
  include/ndn-cpp/util/exponential-re-express.hpp \
  include/ndn-cpp/util/logging.hpp \
  include/ndn-cpp/util/memory-content-cache.hpp \
  include/ndn-cpp/util/segment-fetcher.hpp \
  include/ndn-cpp/util/signed-blob.hpp

# Public ndn-cpp-tools C++ headers.
# NOTE: If a new directory is added, then add it to ndn_cpp_tools_cpp_headers in include/Makefile.am.
ndn_cpp_tools_cpp_headers = \
  include/ndn-cpp-tools/usersync/channel-discovery.hpp \
  include/ndn-cpp-tools/usersync/content-meta-info.hpp \
  include/ndn-cpp-tools/usersync/generalized-content.hpp \
  include/ndn-cpp-tools/usersync/prefix-discovery.hpp

# Just the C code.
libndn_c_la_SOURCES = ${ndn_cpp_c_headers} \
  contrib/apache/apr_base64.c contrib/apache/apr_base64.h \
  src/c/control-parameters_c.c src/c/control-parameters.h \
  src/c/control-response.h \
  src/c/data.h \
  src/c/delegation-set.h \
  src/c/errors.c src/c/errors.h \
  src/c/forwarding-flags.c src/c/forwarding-flags-impl.h \
  src/c/interest_c.c src/c/interest.h \
  src/c/key-locator.h \
  src/c/name_c.c src/c/name.h \
  src/c/network-nack_c.c src/c/network-nack.h \
  src/c/encoding/element-listener.h \
  src/c/encoding/element-reader.c src/c/encoding/element-reader.h \
  src/c/encoding/tlv-0_2-wire-format_c.c src/c/encoding/tlv-0_2-wire-format.h \
  src/c/encoding/tlv/tlv-control-parameters.c src/c/encoding/tlv/tlv-control-parameters.h \
  src/c/encoding/tlv/tlv-control-response.c src/c/encoding/tlv/tlv-control-response.h \
  src/c/encoding/tlv/tlv-data.c src/c/encoding/tlv/tlv-data.h \
  src/c/encoding/tlv/tlv-delegation-set.c src/c/encoding/tlv/tlv-delegation-set.h \
  src/c/encoding/tlv/tlv-decoder.c src/c/encoding/tlv/tlv-decoder.h \
  src/c/encoding/tlv/tlv-encoder.c src/c/encoding/tlv/tlv-encoder.h \
  src/c/encoding/tlv/tlv-encrypted-content.c src/c/encoding/tlv/tlv-encrypted-content.h \
  src/c/encoding/tlv/tlv-interest.c src/c/encoding/tlv/tlv-interest.h \
  src/c/encoding/tlv/tlv-key-locator.c src/c/encoding/tlv/tlv-key-locator.h \
  src/c/encoding/tlv/tlv-lp-packet.c src/c/encoding/tlv/tlv-lp-packet.h \
  src/c/encoding/tlv/tlv-name.c src/c/encoding/tlv/tlv-name.h \
  src/c/encoding/tlv/tlv-signature-info.c src/c/encoding/tlv/tlv-signature-info.h \
  src/c/encoding/tlv/tlv-structure-decoder.c src/c/encoding/tlv/tlv-structure-decoder.h \
  src/c/encoding/tlv/tlv.h \
  src/c/encrypt/encrypted-content.h \
  src/c/encrypt/algo/aes-algorithm_c.c src/c/encrypt/algo/aes-algorithm.h \
  src/c/encrypt/algo/encrypt-params.h \
  src/c/lp/congestion-mark_c.c src/c/lp/congestion-mark.h \
  src/c/lp/incoming-face-id_c.c src/c/lp/incoming-face-id.h \
  src/c/lp/lp-packet.h \
  src/c/security/ec-private-key.c src/c/util/ec-private-key.h \
  src/c/security/ec-public-key.c src/c/util/ec-public-key.h \
  src/c/security/rsa-private-key.c src/c/util/rsa-private-key.h \
  src/c/security/rsa-public-key.c src/c/util/rsa-public-key.h \
  src/c/security/validity-period.h \
  src/c/transport/socket-transport.c src/c/transport/socket-transport.h \
  src/c/transport/tcp-transport_c.c src/c/transport/tcp-transport.h \
  src/c/transport/udp-transport.h \
  src/c/transport/unix-transport.h \
  src/c/util/blob_c.c src/c/util/blob.h \
  src/c/util/crypto.c src/c/util/crypto.h \
  src/c/util/dynamic-uint8-array.c src/c/util/dynamic-uint8-array.h \
  src/c/util/ndn_memory.c src/c/util/ndn_memory.h \
  src/c/util/ndn_realloc.c src/c/util/ndn_realloc.h \
  src/c/util/time.c src/c/util/time.h

# C++ code and also the C code.
libndn_cpp_la_SOURCES = ${libndn_c_la_SOURCES} ${ndn_cpp_cpp_headers} \
  src/common.cpp \
  src/control-parameters.cpp \
  src/control-response.cpp \
  src/data.cpp \
  src/delegation-set.cpp \
  src/digest-sha256-signature.cpp \
  src/exclude.cpp \
  src/face.cpp \
  src/generic-signature.cpp \
  src/hmac-with-sha256-signature.cpp \
  src/interest-filter.cpp \
  src/interest.cpp \
  src/key-locator.cpp \
  src/link.cpp \
  src/meta-info.cpp \
  src/name.cpp \
  src/network-nack.cpp \
  src/node.cpp src/node.hpp \
  src/signature.cpp \
  src/sha256-with-ecdsa-signature.cpp \
  src/sha256-with-rsa-signature.cpp \
  src/threadsafe-face.cpp \
  src/encoding/base64.cpp src/encoding/base64.hpp \
  src/encoding/element-listener.cpp src/encoding/element-listener.hpp \
  src/encoding/oid.cpp \
  src/encoding/protobuf-tlv.cpp \
  src/encoding/tlv-0_1-wire-format.cpp \
  src/encoding/tlv-0_1_1-wire-format.cpp \
  src/encoding/tlv-0_2-wire-format.cpp \
  src/encoding/tlv-decoder.hpp \
  src/encoding/tlv-encoder.hpp \
  src/encoding/tlv-wire-format.cpp \
  src/encoding/wire-format.cpp \
  src/encoding/der/der-exception.cpp src/encoding/der/der-exception.hpp \
  src/encoding/der/der-node.cpp src/encoding/der/der-node.hpp \
  src/encrypt/consumer.cpp \
  src/encrypt/consumer-db.cpp \
  src/encrypt/encrypted-content.cpp \
  src/encrypt/group-manager.cpp \
  src/encrypt/group-manager-db.cpp \
  src/encrypt/interval.cpp \
  src/encrypt/producer.cpp \
  src/encrypt/producer-db.cpp \
  src/encrypt/repetitive-interval.cpp \
  src/encrypt/schedule.cpp \
  src/encrypt/sqlite3-consumer-db.cpp \
  src/encrypt/sqlite3-group-manager-db.cpp \
  src/encrypt/sqlite3-producer-db.cpp \
  src/encrypt/algo/aes-algorithm.cpp \
  src/encrypt/algo/encrypt-params.cpp \
  src/encrypt/algo/encryptor.cpp \
  src/encrypt/algo/rsa-algorithm.cpp \
  src/impl/delayed-call-table.cpp src/impl/delayed-call-table.hpp \
  src/impl/interest-filter-table.cpp src/impl/interest-filter-table.hpp \
  src/impl/pending-interest-table.cpp src/impl/pending-interest-table.hpp \
  src/impl/registered-prefix-table.cpp src/impl/registered-prefix-table.hpp \
  src/lite/control-parameters-lite.cpp \
  src/lite/control-response-lite.cpp \
  src/lite/data-lite.cpp \
  src/lite/delegation-set-lite.cpp \
  src/lite/exclude-lite.cpp \
  src/lite/forwarding-flags-lite.cpp \
  src/lite/interest-lite.cpp \
  src/lite/key-locator-lite.cpp \
  src/lite/meta-info-lite.cpp \
  src/lite/name-lite.cpp \
  src/lite/network-nack-lite.cpp \
  src/lite/signature-lite.cpp \
  src/lite/encoding/element-listener-lite.cpp \
  src/lite/encoding/tlv-0_2-wire-format-lite.cpp \
  src/lite/encrypt/encrypted-content-lite.cpp \
  src/lite/encrypt/algo/aes-algorithm-lite.cpp \
  src/lite/encrypt/algo/encrypt-params-lite.cpp \
  src/lite/lp/congestion-mark-lite.cpp \
  src/lite/lp/incoming-face-id-lite.cpp \
  src/lite/lp/lp-packet-lite.cpp \
  src/lite/security/ec-private-key-lite.cpp \
  src/lite/security/ec-public-key-lite.cpp \
  src/lite/security/rsa-private-key-lite.cpp \
  src/lite/security/rsa-public-key-lite.cpp \
  src/lite/security/validity-period-lite.cpp \
  src/lite/transport/tcp-transport-lite.cpp \
  src/lite/transport/udp-transport-lite.cpp \
  src/lite/transport/unix-transport-lite.cpp \
  src/lite/util/blob-lite.cpp \
  src/lite/util/crypto-lite.cpp \
  src/lite/util/dynamic-malloc-uint8-array-lite.cpp \
  src/lite/util/dynamic-uint8-array-lite.cpp \
  src/lp/congestion-mark.cpp src/lp/congestion-mark.hpp \
  src/lp/incoming-face-id.cpp src/lp/incoming-face-id.hpp \
  src/lp/lp-packet.cpp src/lp/lp-packet.hpp \
  src/security/command-interest-signer.cpp \
  src/security/key-chain.cpp \
  src/security/key-params.cpp \
  src/security/safe-bag.cpp \
  src/security/security-exception.cpp \
  src/security/signing-info.cpp \
  src/security/validator-null.cpp \
  src/security/validity-period.cpp \
  src/security/verification-helpers.cpp \
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
  src/security/identity/private-key-storage.cpp \
  src/security/pib/pib-certificate-container.cpp \
  src/security/pib/pib-identity.cpp \
  src/security/pib/pib-identity-container.cpp \
  src/security/pib/pib-key.cpp \
  src/security/pib/pib-key-container.cpp \
  src/security/pib/pib-memory.cpp \
  src/security/pib/pib-sqlite3.cpp \
  src/security/pib/pib.cpp \
  src/security/pib/detail/pib-identity-impl.cpp \src/security/pib/detail/pib-identity-impl.hpp \
  src/security/pib/detail/pib-key-impl.cpp \src/security/pib/detail/pib-key-impl.hpp \
  src/security/policy/config-policy-manager.cpp \
  src/security/policy/no-verify-policy-manager.cpp \
  src/security/policy/policy-manager.cpp \
  src/security/policy/self-verify-policy-manager.cpp \
  src/security/tpm/tpm.cpp \
  src/security/tpm/tpm-back-end.cpp \
  src/security/tpm/tpm-back-end-file.cpp \
  src/security/tpm/tpm-back-end-memory.cpp \
  src/security/tpm/tpm-back-end-osx.cpp \
  src/security/tpm/tpm-key-handle.cpp \
  src/security/tpm/tpm-key-handle-memory.cpp \
  src/security/tpm/tpm-key-handle-osx.cpp \
  src/security/tpm/tpm-private-key.cpp \
  src/security/v2/certificate-cache-v2.cpp \
  src/security/v2/certificate-fetcher-from-network.cpp \
  src/security/v2/certificate-fetcher-offline.cpp \
  src/security/v2/certificate-fetcher.cpp \
  src/security/v2/certificate-storage.cpp \
  src/security/v2/certificate-v2.cpp \
  src/security/v2/trust-anchor-container.cpp \
  src/security/v2/trust-anchor-group.cpp \
  src/security/v2/validation-error.cpp \
  src/security/v2/validation-policy-accept-all.cpp \
  src/security/v2/validation-policy-command-interest.cpp \
  src/security/v2/validation-policy-config.cpp \
  src/security/v2/validation-policy-from-pib.cpp \
  src/security/v2/validation-policy-simple-hierarchy.cpp \
  src/security/v2/validation-policy.cpp \
  src/security/v2/validation-state.cpp \
  src/security/v2/validator.cpp \
  src/security/v2/validator-config/config-checker.cpp \
  src/security/v2/validator-config/config-filter.cpp \
  src/security/v2/validator-config/config-name-relation.cpp \
  src/security/v2/validator-config/config-rule.cpp \
  src/sync/sync-state.pb.cc \
  src/sync/chrono-sync2013.cpp \
  src/sync/digest-tree.cpp src/sync/digest-tree.hpp \
  src/transport/async-tcp-transport.cpp \
  src/transport/async-unix-transport.cpp \
  src/transport/tcp-transport.cpp \
  src/transport/transport.cpp \
  src/transport/udp-transport.cpp \
  src/transport/unix-transport.cpp \
  src/util/blob-stream.hpp \
  src/util/boost-info-parser.cpp src/util/boost-info-parser.hpp \
  src/util/command-interest-generator.cpp src/util/command-interest-generator.hpp \
  src/util/config-file.cpp src/util/config-file.hpp \
  src/util/dynamic-uint8-vector.cpp src/util/dynamic-uint8-vector.hpp \
  src/util/exponential-re-express.cpp \
  src/util/logging.cpp \
  src/util/memory-content-cache.cpp \
  src/util/segment-fetcher.cpp \
  src/util/sqlite3-statement.cpp src/util/sqlite3-statement.hpp \
  src/util/regex/ndn-regex-backref-manager.cpp src/util/regex/ndn-regex-backref-manager.hpp \
  src/util/regex/ndn-regex-backref-matcher.cpp src/util/regex/ndn-regex-backref-matcher.hpp \
  src/util/regex/ndn-regex-component-matcher.cpp src/util/regex/ndn-regex-component-matcher.hpp \
  src/util/regex/ndn-regex-component-set-matcher.cpp src/util/regex/ndn-regex-component-set-matcher.hpp \
  src/util/regex/ndn-regex-matcher-base.cpp src/util/regex/ndn-regex-matcher-base.hpp \
  src/util/regex/ndn-regex-pattern-list-matcher.cpp src/util/regex/ndn-regex-pattern-list-matcher.hpp \
  src/util/regex/ndn-regex-pseudo-matcher.cpp src/util/regex/ndn-regex-pseudo-matcher.hpp \
  src/util/regex/ndn-regex-repeat-matcher.cpp src/util/regex/ndn-regex-repeat-matcher.hpp \
  src/util/regex/ndn-regex-top-matcher.cpp src/util/regex/ndn-regex-top-matcher.hpp

# The ndn-cpp-tools library.
libndn_cpp_tools_la_SOURCES = ${ndn_cpp_tools_cpp_headers} ${ndn_cpp_cpp_headers} \
  tools/usersync/content-meta-info.pb.cc \
  tools/usersync/channel-discovery.cpp \
  tools/usersync/content-meta-info.cpp \
  tools/usersync/generalized-content.cpp tools/usersync/prefix-discovery.cpp

bin_test_channel_discovery_SOURCES = examples/test-channel-discovery.cpp
bin_test_channel_discovery_LDADD = libndn-cpp.la libndn-cpp-tools.la

bin_test_chrono_chat_SOURCES = examples/chatbuf.pb.cc examples/test-chrono-chat.cpp
bin_test_chrono_chat_LDADD = libndn-cpp.la

bin_test_echo_consumer_lite_SOURCES = examples/test-echo-consumer-lite.cpp
bin_test_echo_consumer_lite_LDADD = libndn-cpp.la

bin_test_echo_consumer_SOURCES = examples/test-echo-consumer.cpp
bin_test_echo_consumer_LDADD = libndn-cpp.la

bin_test_encode_decode_benchmark_SOURCES = examples/test-encode-decode-benchmark.cpp
bin_test_encode_decode_benchmark_LDADD = libndn-cpp.la

bin_test_encode_decode_data_SOURCES = examples/test-encode-decode-data.cpp
bin_test_encode_decode_data_LDADD = libndn-cpp.la

bin_test_encode_decode_fib_entry_SOURCES = examples/fib-entry.pb.cc examples/test-encode-decode-fib-entry.cpp
bin_test_encode_decode_fib_entry_LDADD = libndn-cpp.la

bin_test_encode_decode_interest_SOURCES = examples/test-encode-decode-interest.cpp
bin_test_encode_decode_interest_LDADD = libndn-cpp.la

bin_test_generalized_content_SOURCES = examples/test-generalized-content.cpp
bin_test_generalized_content_LDADD = libndn-cpp.la libndn-cpp-tools.la

bin_test_get_async_SOURCES = examples/test-get-async.cpp
bin_test_get_async_LDADD = libndn-cpp.la

bin_test_get_async_threadsafe_SOURCES = examples/test-get-async-threadsafe.cpp
bin_test_get_async_threadsafe_LDADD = libndn-cpp.la

bin_test_list_channels_SOURCES = examples/channel-status.pb.cc examples/test-list-channels.cpp
bin_test_list_channels_LDADD = libndn-cpp.la

bin_test_list_faces_SOURCES = examples/face-status.pb.cc examples/test-list-faces.cpp
bin_test_list_faces_LDADD = libndn-cpp.la

bin_test_list_rib_SOURCES = examples/rib-entry.pb.cc examples/test-list-rib.cpp
bin_test_list_rib_LDADD = libndn-cpp.la

bin_test_prefix_discovery_SOURCES = examples/test-prefix-discovery.cpp
bin_test_prefix_discovery_LDADD = libndn-cpp.la libndn-cpp-tools.la

bin_test_publish_async_nfd_SOURCES = examples/test-publish-async-nfd.cpp
bin_test_publish_async_nfd_LDADD = libndn-cpp.la

bin_test_publish_async_nfd_lite_SOURCES = examples/test-publish-async-nfd-lite.cpp
bin_test_publish_async_nfd_lite_LDADD = libndn-cpp.la

bin_analog_reading_consumer_SOURCES = examples/arduino/analog-reading-consumer.cpp
bin_analog_reading_consumer_LDADD = libndn-cpp.la

bin_test_sign_verify_data_hmac_SOURCES = examples/test-sign-verify-data-hmac.cpp
bin_test_sign_verify_data_hmac_LDADD = libndn-cpp.la

bin_basic_insertion_SOURCES = \
  examples/repo-ng/repo-command-parameter.pb.cc examples/repo-ng/repo-command-response.pb.cc \
  examples/repo-ng/basic-insertion.cpp
bin_basic_insertion_LDADD = libndn-cpp.la

bin_watched_insertion_SOURCES = \
  examples/repo-ng/repo-command-parameter.pb.cc examples/repo-ng/repo-command-response.pb.cc \
  examples/repo-ng/watched-insertion.cpp
bin_watched_insertion_LDADD = libndn-cpp.la

bin_test_register_route_SOURCES = \
  examples/control-parameters.pb.cc examples/face-query-filter.pb.cc \
  examples/face-status.pb.cc examples/test-register-route.cpp
bin_test_register_route_LDADD = libndn-cpp.la

# Unit tests

bin_unit_tests_test_aes_algorithm_SOURCES = tests/unit-tests/test-aes-algorithm.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_aes_algorithm_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_aes_algorithm_LDADD = libndn-cpp.la

bin_unit_tests_test_certificate_SOURCES = tests/unit-tests/test-certificate.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_certificate_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_certificate_LDADD = libndn-cpp.la

bin_unit_tests_test_consumer_SOURCES = tests/unit-tests/test-consumer.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_consumer_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_consumer_LDADD = libndn-cpp.la

bin_unit_tests_test_consumer_db_SOURCES = tests/unit-tests/test-consumer-db.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_consumer_db_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_consumer_db_LDADD = libndn-cpp.la

bin_unit_tests_test_control_parameters_encode_decode_SOURCES = tests/unit-tests/test-control-parameters-encode-decode.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_control_parameters_encode_decode_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_control_parameters_encode_decode_LDADD = libndn-cpp.la

bin_unit_tests_test_control_response_SOURCES = tests/unit-tests/test-control-response.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_control_response_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_control_response_LDADD = libndn-cpp.la

bin_unit_tests_test_data_methods_SOURCES = tests/unit-tests/test-data-methods.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_data_methods_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_data_methods_LDADD = libndn-cpp.la

bin_unit_tests_test_der_encode_decode_SOURCES = tests/unit-tests/test-der-encode-decode.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_der_encode_decode_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_der_encode_decode_LDADD = libndn-cpp.la

bin_unit_tests_test_encrypted_content_SOURCES = tests/unit-tests/test-encrypted-content.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_encrypted_content_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_encrypted_content_LDADD = libndn-cpp.la

bin_unit_tests_test_encryptor_SOURCES = tests/unit-tests/test-encryptor.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_encryptor_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_encryptor_LDADD = libndn-cpp.la

bin_unit_tests_test_face_methods_SOURCES = tests/unit-tests/test-face-methods.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_face_methods_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_face_methods_LDADD = libndn-cpp.la

bin_unit_tests_test_group_manager_SOURCES = tests/unit-tests/test-group-manager.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_group_manager_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_group_manager_LDADD = libndn-cpp.la

bin_unit_tests_test_group_manager_db_SOURCES = tests/unit-tests/test-group-manager-db.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_group_manager_db_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_group_manager_db_LDADD = libndn-cpp.la

bin_unit_tests_test_identity_methods_SOURCES = tests/unit-tests/test-identity-methods.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_identity_methods_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_identity_methods_LDADD = libndn-cpp.la

bin_unit_tests_test_interest_methods_SOURCES = tests/unit-tests/test-interest-methods.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_interest_methods_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_interest_methods_LDADD = libndn-cpp.la

bin_unit_tests_test_interval_SOURCES = tests/unit-tests/test-interval.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_interval_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_interval_LDADD = libndn-cpp.la

bin_unit_tests_test_key_chain_SOURCES = tests/unit-tests/test-key-chain.cpp \
  tests/unit-tests/identity-management-fixture.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_key_chain_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_key_chain_LDADD = libndn-cpp.la

bin_unit_tests_test_link_SOURCES = tests/unit-tests/test-link.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_link_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_link_LDADD = libndn-cpp.la

bin_unit_tests_test_name_conventions_SOURCES = tests/unit-tests/test-name-conventions.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_name_conventions_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_name_conventions_LDADD = libndn-cpp.la

bin_unit_tests_test_name_methods_SOURCES = tests/unit-tests/test-name-methods.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_name_methods_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_name_methods_LDADD = libndn-cpp.la

bin_unit_tests_test_pib_certificate_container_SOURCES = tests/unit-tests/test-pib-certificate-container.cpp \
  tests/unit-tests/pib-data-fixture.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_pib_certificate_container_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_pib_certificate_container_LDADD = libndn-cpp.la

bin_unit_tests_test_pib_identity_container_SOURCES = tests/unit-tests/test-pib-identity-container.cpp \
  tests/unit-tests/pib-data-fixture.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_pib_identity_container_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_pib_identity_container_LDADD = libndn-cpp.la

bin_unit_tests_test_pib_identity_impl_SOURCES = tests/unit-tests/test-pib-identity-impl.cpp \
  tests/unit-tests/pib-data-fixture.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_pib_identity_impl_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_pib_identity_impl_LDADD = libndn-cpp.la

bin_unit_tests_test_pib_impl_SOURCES = tests/unit-tests/test-pib-impl.cpp \
  tests/unit-tests/pib-data-fixture.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_pib_impl_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_pib_impl_LDADD = libndn-cpp.la

bin_unit_tests_test_pib_key_container_SOURCES = tests/unit-tests/test-pib-key-container.cpp \
  tests/unit-tests/pib-data-fixture.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_pib_key_container_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_pib_key_container_LDADD = libndn-cpp.la

bin_unit_tests_test_pib_key_impl_SOURCES = tests/unit-tests/test-pib-key-impl.cpp \
  tests/unit-tests/pib-data-fixture.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_pib_key_impl_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_pib_key_impl_LDADD = libndn-cpp.la

bin_unit_tests_test_policy_manager_SOURCES = tests/unit-tests/test-policy-manager.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_policy_manager_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_policy_manager_LDADD = libndn-cpp.la

bin_unit_tests_test_policy_manager_v2_SOURCES = tests/unit-tests/test-policy-manager-v2.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_policy_manager_v2_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_policy_manager_v2_LDADD = libndn-cpp.la

bin_unit_tests_test_producer_SOURCES = tests/unit-tests/test-producer.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_producer_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_producer_LDADD = libndn-cpp.la

bin_unit_tests_test_producer_db_SOURCES = tests/unit-tests/test-producer-db.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_producer_db_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_producer_db_LDADD = libndn-cpp.la

bin_unit_tests_test_regex_SOURCES = tests/unit-tests/test-regex.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_regex_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_regex_LDADD = libndn-cpp.la

bin_unit_tests_test_registration_callbacks_SOURCES = tests/unit-tests/test-registration-callbacks.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_registration_callbacks_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_registration_callbacks_LDADD = libndn-cpp.la

bin_unit_tests_test_repetitive_interval_SOURCES = tests/unit-tests/test-repetitive-interval.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_repetitive_interval_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_repetitive_interval_LDADD = libndn-cpp.la

bin_unit_tests_test_rsa_algorithm_SOURCES = tests/unit-tests/test-rsa-algorithm.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_rsa_algorithm_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_rsa_algorithm_LDADD = libndn-cpp.la

bin_unit_tests_test_schedule_SOURCES = tests/unit-tests/test-schedule.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_schedule_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_schedule_LDADD = libndn-cpp.la

bin_unit_tests_test_signing_info_SOURCES = tests/unit-tests/test-signing-info.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_signing_info_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_signing_info_LDADD = libndn-cpp.la

bin_unit_tests_test_tpm_back_ends_SOURCES = tests/unit-tests/test-tpm-back-ends.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_tpm_back_ends_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_tpm_back_ends_LDADD = libndn-cpp.la

bin_unit_tests_test_tpm_private_key_SOURCES = tests/unit-tests/test-tpm-private-key.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_tpm_private_key_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_tpm_private_key_LDADD = libndn-cpp.la

bin_unit_tests_test_validation_policy_command_interest_SOURCES = \
  tests/unit-tests/test-validation-policy-command-interest.cpp \
  tests/unit-tests/identity-management-fixture.cpp \
  tests/unit-tests/validator-fixture.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_validation_policy_command_interest_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_validation_policy_command_interest_LDADD = libndn-cpp.la

bin_unit_tests_test_validation_policy_config_SOURCES = tests/unit-tests/test-validation-policy-config.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_validation_policy_config_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_validation_policy_config_LDADD = libndn-cpp.la

bin_unit_tests_test_validator_null_SOURCES = tests/unit-tests/test-validator-null.cpp \
  tests/unit-tests/identity-management-fixture.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_validator_null_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_validator_null_LDADD = libndn-cpp.la

bin_unit_tests_test_validator_SOURCES = tests/unit-tests/test-validator.cpp \
  tests/unit-tests/identity-management-fixture.cpp \
  tests/unit-tests/validator-fixture.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_validator_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_validator_LDADD = libndn-cpp.la

bin_unit_tests_test_verification_rules_SOURCES = tests/unit-tests/test-verification-rules.cpp contrib/gtest-1.7.0/fused-src/gtest/gtest-all.cc
bin_unit_tests_test_verification_rules_CPPFLAGS = -I./contrib/gtest-1.7.0/fused-src
bin_unit_tests_test_verification_rules_LDADD = libndn-cpp.la

TESTS = ${check_PROGRAMS}

dist_noinst_SCRIPTS = autogen.sh
