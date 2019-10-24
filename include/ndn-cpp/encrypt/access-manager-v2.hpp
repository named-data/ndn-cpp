/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2018-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the NAC library https://github.com/named-data/name-based-access-control/blob/new/src/access-manager.hpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version, with the additional exemption that
 * compiling, linking, and/or using OpenSSL is allowed.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU Lesser General Public License is in the file COPYING.
 */

#ifndef NDN_ACCESS_MANAGER_V2_HPP
#define NDN_ACCESS_MANAGER_V2_HPP

#include "../security/key-chain.hpp"
#include "../in-memory-storage/in-memory-storage-retaining.hpp"

// Give friend access to the tests.
class TestAccessManagerV2_EnumerateDataFromInMemoryStorage_Test;

namespace ndn {

/**
 * AccessManagerV2 controls the decryption policy by publishing granular
 * per-namespace access policies in the form of key encryption
 * (KEK, plaintext public) and key decryption (KDK, encrypted private key)
 * key pairs. This works with EncryptorV2 and DecryptorV2 using security v2.
 * For the meaning of "KDK", etc. see:
 * https://github.com/named-data/name-based-access-control/blob/new/docs/spec.rst
 */
class AccessManagerV2 {
public:
  /**
   * Create an AccessManagerV2 to serve the NAC public key for other data
   * producers to fetch, and to serve encrypted versions of the private keys
   * (as safe bags) for authorized consumers to fetch.
   *
   * KEK and KDK naming:
   *
   * [identity]/NAC/[dataset]/KEK            /[key-id]                           (== KEK, public key)
   *
   * [identity]/NAC/[dataset]/KDK/[key-id]   /ENCRYPTED-BY/[user]/KEY/[key-id]   (== KDK, encrypted private key)
   *
   * \_____________  ______________/
   *               \/
   *      registered with NFD
   *
   * @param identity The data owner's namespace identity. (This will be used to
   * sign the KEK and KDK.)
   * @param dataset The name of dataset that this manager is controlling.
   * @param keyChain The KeyChain used to sign Data packets.
   * @param face The Face for calling registerPrefix that will be used to
   * publish the KEK and KDK Data packets.
   */
  AccessManagerV2
    (const ptr_lib::shared_ptr<PibIdentity>& identity, const Name& dataset,
     KeyChain* keyChain, Face* face)
  : impl_(new Impl(identity, keyChain, face))
  {
    impl_->initialize(dataset);
  }

  void
  shutdown() { impl_->shutdown(); }

  /**
   * Authorize a member identified by memberCertificate to decrypt data under
   * the policy.
   * @param memberCertificate The certificate that identifies the member to
   * authorize.
   * @return The published KDK Data packet.
   */
  ptr_lib::shared_ptr<Data>
  addMember(const CertificateV2& memberCertificate)
  {
    return impl_->addMember(memberCertificate);
  }

  /**
   * Get the number of packets stored in in-memory storage.
   * @return The number of packets.
   */
  size_t
  size() { return impl_->size(); }

private:
  // Give friend access to the tests.
  friend class ::TestAccessManagerV2_EnumerateDataFromInMemoryStorage_Test;

  /**
   * AccessManagerV2::Impl does the work of AccessManagerV2. It is a separate
   * class so that AccessManagerV2 can create an instance in a shared_ptr to use
   * in callbacks.
   */
  class Impl : public ptr_lib::enable_shared_from_this<Impl> {
  public:
    /**
     * Create a new Impl, which should belong to a shared_ptr. Then you must
     * call initialize().  See the AccessManagerV2 constructor for parameter
     * documentation.
     */
    Impl
      (const ptr_lib::shared_ptr<PibIdentity>& identity, KeyChain* keyChain,
       Face* face)
      : identity_(identity), keyChain_(keyChain), face_(face)
      {}

    /**
     * Complete the work of the constructor. This is needed because we can't
     * call shared_from_this() in the constructor.
     */
    void
    initialize(const Name& dataset);

    void
    shutdown();

    ptr_lib::shared_ptr<Data>
    addMember(const CertificateV2& memberCertificate);

    size_t
    size() { return storage_.size(); }

  private:
    // Give friend access to the tests.
    friend class ::TestAccessManagerV2_EnumerateDataFromInMemoryStorage_Test;

    ptr_lib::shared_ptr<PibIdentity> identity_;
    ptr_lib::shared_ptr<PibKey> nacKey_;
    KeyChain* keyChain_;
    Face* face_;

    // storage_ is for the KEK and KDKs.
    InMemoryStorageRetaining storage_;
    uint64_t kekRegisteredPrefixId_;
    uint64_t kdkRegisteredPrefixId_;

    static const uint64_t DEFAULT_KEK_FRESHNESS_PERIOD_MS = 3600 * 1000;
    static const uint64_t DEFAULT_KDK_FRESHNESS_PERIOD_MS = 3600 * 1000;
  };

  ptr_lib::shared_ptr<Impl> impl_;
};

}

#endif
