/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/encrypt-params https://github.com/named-data/ndn-group-encrypt
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

#ifndef NDN_ENCRYPT_PARAMS_HPP
#define NDN_ENCRYPT_PARAMS_HPP

#include "../../util/blob.hpp"
#include "../../c/encrypt/algo/encrypt-params-types.h"

namespace ndn {

/**
 * An EncryptParams holds an algorithm type and other parameters used to encrypt
 * and decrypt.
 * @note This class is an experimental feature. The API may change.
 */
class EncryptParams {
public:
  /**
   * Create an EncryptParams with the given parameters.
   * @param algorithmType The algorithm type, or -1 if not specified.
   * @param initialVectorLength The initial vector length, or 0 if the initial
   * vector is not specified.
   */
  EncryptParams
    (ndn_EncryptAlgorithmType algorithmType, size_t initialVectorLength);

  /**
   * Create an EncryptParams with the given algorithmType and an unspecified
   * initial vector.
   * @param algorithmType The algorithm type, or -1 if not specified.
   */
  EncryptParams(ndn_EncryptAlgorithmType algorithmType)
  {
    algorithmType_ = algorithmType;
  }

  /**
   * Get the algorithm type.
   * @return The algorithm type, or -1 if not specified.
   */
  ndn_EncryptAlgorithmType
  getAlgorithmType() const { return algorithmType_; }

  /**
   * Get the initial vector.
   * @return The initial vector. If not specified, isNull() is true.
   */
  const Blob&
  getInitialVector() const { return initialVector_; }

  /**
   * Set the algorithm type.
   * @param algorithmType The algorithm type. If not specified, set to -1.
   * @return This EncryptParams so that you can chain calls to update values.
   */
  EncryptParams&
  setAlgorithmType(ndn_EncryptAlgorithmType algorithmType)
  {
    algorithmType_ = algorithmType;
    return *this;
  }

  /**
   * Set the initial vector.
   * @param initialVector The initial vector. If not specified, set to the
   * default Blob() where isNull() is true.
   * @return This EncryptParams so that you can chain calls to update values.
   */
  EncryptParams&
  setInitialVector(const Blob& initialVector)
  {
    initialVector_ = initialVector;
    return *this;
  }

private:
  ndn_EncryptAlgorithmType algorithmType_;
  Blob initialVector_;
};

}

#endif
