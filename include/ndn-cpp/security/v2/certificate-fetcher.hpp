/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/certificate-fetcher.hpp
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

#ifndef NDN_CERTIFICATE_FETCHER_HPP
#define NDN_CERTIFICATE_FETCHER_HPP

#include "certificate-storage.hpp"
#include "certificate-request.hpp"
#include "validation-state.hpp"

namespace ndn {

/**
 * CertificateFetcher is an abstract base class which provides an interface used
 * by the validator to fetch missing certificates.
 */
class CertificateFetcher {
public:
  typedef func_lib::function<void
    (const ptr_lib::shared_ptr<CertificateV2>& certificate,
     const ptr_lib::shared_ptr<ValidationState>& state)> ValidationContinuation;

  CertificateFetcher()
  : certificateStorage_(0)
  {
  }

  virtual
  ~CertificateFetcher();

  /**
   * Assign the certificate storage used to check for known certificates and to
   * cache unverified ones.
   * @param certificateStorage The certificate storage object which must be
   * valid for the lifetime of this CertificateFetcher.
   */
  virtual void
  setCertificateStorage(CertificateStorage& certificateStorage);

  /**
   * Asynchronously fetch a certificate. setCertificateStorage must have been
   * called first.
   * If the requested certificate exists in the storage, then this method will
   * immediately call continueValidation with the certificate. If certificate is
   * not available, then the implementation-specific doFetch will be called to
   * asynchronously fetch the certificate. The successfully-retrieved
   * certificate will be automatically added to the unverified cache of the
   * certificate storage.
   * When the requested certificate is retrieved, continueValidation is called.
   * Otherwise, the fetcher implementation calls state.failed() with the
   * appropriate error code and diagnostic message.
   * @param certificateRequest The the request with the Interest for fetching
   * the certificate.
   * @param state The validation state.
   * @param continueValidation After fetching, this calls
   * continueValidation(certificate, state) where certificate is the fetched
   * certificate and state is the ValidationState.
   */
  void
  fetch
    (const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
     const ptr_lib::shared_ptr<ValidationState>& state,
     const ValidationContinuation& continueValidation);

protected:
  /**
   * An implementation to fetch a certificate asynchronously. The subclass must
   * implement this method.
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
     const ValidationContinuation& continueValidation) = 0;

private:
  /**
   * This is the continuation for the fetch method.
   */
  void
  continueFetch
    (const ptr_lib::shared_ptr<CertificateV2>& certificate,
     const ptr_lib::shared_ptr<ValidationState>& state,
     const ValidationContinuation& continueValidation)
  {
    certificateStorage_->cacheUnverifiedCertificate(*certificate);
    continueValidation(certificate, state);
  }

  // Disable the copy constructor and assignment operator.
  CertificateFetcher(const CertificateFetcher& other);
  CertificateFetcher& operator=(const CertificateFetcher& other);

protected:
  CertificateStorage* certificateStorage_;
};

}

#endif
