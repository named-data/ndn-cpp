/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/certificate-fetcher-from-network.hpp
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

#ifndef NDN_CERTIFICATE_FETCHER_FROM_NETWORK_HPP
#define NDN_CERTIFICATE_FETCHER_FROM_NETWORK_HPP

#include "../../face.hpp"
#include "certificate-fetcher.hpp"

namespace ndn {

/**
 * CertificateFetcherFromNetwork extends CertificateFetcher to fetch missing
 * certificates from the network.
 */
class CertificateFetcherFromNetwork : public CertificateFetcher {
public:
  /**
   * Create a CertificateFetcherFromNetwork to fetch certificates using the Face.
   * @param face The face for calling expressInterest.
   */
  CertificateFetcherFromNetwork(Face& face)
  : impl_(new Impl(*this, face))
  {}

protected:
  /**
   * Implement doFetch to use face_.expressInterest to fetch a certificate.
   * @param certificateRequest The the request with the Interest for fetching
   * the certificate.
   * @param state The validation state.
   * @param continueValidation After fetching, this calls
   * continueValidation(certificate, state) where certificate is the fetched
   * certificate and state is the ValidationState.
   */
  virtual void
  doFetch
    (const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
     const ptr_lib::shared_ptr<ValidationState>& state,
     const ValidationContinuation& continueValidation)
  {
    impl_->doFetch(certificateRequest, state, continueValidation);
  }

private:
  /**
   * CertificateFetcherFromNetwork::Impl does the work of
   * CertificateFetcherFromNetwork. It is a separate class so that
   * CertificateFetcherFromNetwork can create an instance in a shared_ptr to
   * use in callbacks.
   */
  class Impl : public ptr_lib::enable_shared_from_this<Impl> {
  public:
    Impl(CertificateFetcherFromNetwork& parent, Face& face)
    : parent_(parent), face_(face)
    {}

    /**
     * Implement doFetch to use face_.expressInterest to fetch a certificate.
     * @param certificateRequest The the request with the Interest for fetching
     * the certificate.
     * @param state The validation state.
     * @param continueValidation After fetching, this calls
     * continueValidation(certificate, state) where certificate is the fetched
     * certificate and state is the ValidationState.
     */
    virtual void
    doFetch
      (const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
       const ptr_lib::shared_ptr<ValidationState>& state,
       const ValidationContinuation& continueValidation);

  private:
    /** This is called when a certificate is retrieved.
     * @param interest This is ignored.
     * @param data The data packet of the fetched certificate.
     * @param state The validation state, from doFetch.
     * @param continueValidation After fetching, this calls
     * continueValidation(certificate, state) where certificate is the fetched
     * certificate and state is the ValidationState.
     */
    void
    onData
      (const ptr_lib::shared_ptr<const Interest>& interest,
       const ptr_lib::shared_ptr<Data>& data,
       const ptr_lib::shared_ptr<ValidationState>& state,
       const CertificateFetcher::ValidationContinuation& continueValidation);

    /** This is called when expressing the Interest for a certificate receives a
     * network Nack. It will re-express the interest if
     * certificateRequest.nRetriesLeft_ is greater than zero, otherwise this calls
     * state.fail() .
     * @param interest This is ignored in favor of the Interest in
     * certificateRequest.
     * @param networkNack The received network Nack.
     * @param certificateRequest The the request with the Interest for fetching
     * the certificate, from doFetch.
     * @param state The validation state, from doFetch.
     * @param continueValidation After fetching, this calls
     * continueValidation(certificate, state) where certificate is the fetched
     * certificate and state is the ValidationState.
     */
    void
    onNetworkNack
      (const ptr_lib::shared_ptr<const Interest>& interest,
       const ptr_lib::shared_ptr<NetworkNack>& networkNack,
       const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
       const ptr_lib::shared_ptr<ValidationState>& state,
       const CertificateFetcher::ValidationContinuation& continueValidation);

    /** This is called when expressing the Interest for a certificate receives a
     * timeout. It will re-express the interest if
     * certificateRequest.nRetriesLeft_ is greater than zero, otherwise this calls
     * state.fail() .
     * @param interest This is ignored in favor of the Interest in
     * certificateRequest.
     * @param certificateRequest The the request with the Interest for fetching
     * the certificate, from doFetch.
     * @param state The validation state, from doFetch.
     * @param continueValidation After fetching, this calls
     * continueValidation(certificate, state) where certificate is the fetched
     * certificate and state is the ValidationState.
     */
    void
    onTimeout
      (const ptr_lib::shared_ptr<const Interest>& interest,
       const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
       const ptr_lib::shared_ptr<ValidationState>& state,
       const CertificateFetcher::ValidationContinuation& continueValidation);

    CertificateFetcherFromNetwork& parent_;
    Face& face_;
  };

  ptr_lib::shared_ptr<Impl> impl_;
};

}

#endif
