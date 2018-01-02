/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/trust-anchor-group.cpp
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

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <ndn-cpp/util/logging.hpp>
#include "../../encoding/base64.hpp"
#include <ndn-cpp/security/v2/trust-anchor-group.hpp>

INIT_LOGGER("ndn.TrustAnchorGroup");

using namespace std;

namespace ndn {

CertificateContainerInterface::~CertificateContainerInterface()
{
}

TrustAnchorGroup::~TrustAnchorGroup()
{
}

void
TrustAnchorGroup::refresh()
{
  // The base method does nothing.
}

ptr_lib::shared_ptr<CertificateV2>
TrustAnchorGroup::readCertificate(const string& filePath)
{
  ifstream certificateFile(filePath.c_str());
  if (!certificateFile.good())
    return ptr_lib::shared_ptr<CertificateV2>();

  stringstream encodedData;
  encodedData << certificateFile.rdbuf();

  // Use a vector in a shared_ptr so we can make it a Blob without copying.
  ptr_lib::shared_ptr<vector<uint8_t> > decodedData(new vector<uint8_t>());
  fromBase64(encodedData.str(), *decodedData);

  ptr_lib::shared_ptr<CertificateV2> result(new CertificateV2());
  try {
    result->wireDecode(Blob(decodedData, false));
    return result;
  } catch (...) {
    return ptr_lib::shared_ptr<CertificateV2>();
  }
}

void
StaticTrustAnchorGroup::add(const CertificateV2& certificate)
{
  if (anchorNames_.count(certificate.getName()) != 0)
    return;

  // This copies the certificate name.
  anchorNames_.insert(certificate.getName());
  // This copies the certificate.
  certificates_.add(certificate);
}

void
StaticTrustAnchorGroup::remove(const Name& certificateName)
{
  anchorNames_.erase(certificateName);
  certificates_.remove(certificateName);
}

DynamicTrustAnchorGroup::DynamicTrustAnchorGroup
  (CertificateContainerInterface& certificateContainer, const string& id,
   const string& path, Milliseconds refreshPeriod, bool isDirectory)
: TrustAnchorGroup(certificateContainer, id),
  isDirectory_(isDirectory),
  path_(path),
  refreshPeriod_(refreshPeriod),
  expireTime_(0)
{
  if (refreshPeriod <= 0)
    throw runtime_error("Refresh period for the dynamic group must be positive");

  _LOG_TRACE("Create a dynamic trust anchor group " << id << " for file/dir " <<
    path << " with refresh time " << refreshPeriod);
  refresh();
}

void
DynamicTrustAnchorGroup::refresh()
{
  Milliseconds now = ndn_getNowMilliseconds();
  if (expireTime_ > now)
    return;

  expireTime_ = now + refreshPeriod_;
  _LOG_TRACE("Reloading the dynamic trust anchor group");

  // Save a copy of anchorNames_ .
  set<Name> oldAnchorNames = anchorNames_;

  if (!isDirectory_)
    loadCertificate(path_, oldAnchorNames);
  else {
    DIR *directory = ::opendir(path_.c_str());
    if (directory != NULL) {
      struct dirent *entry;
      while ((entry = ::readdir(directory)) != NULL) {
        // TODO: Handle non-unix file systems which don't have stat.
        string filePath = path_ + '/' + entry->d_name;
        struct stat fileStat;
        if (::stat(filePath.c_str(), &fileStat) != -1 &&
            S_ISREG(fileStat.st_mode))
          loadCertificate(filePath, oldAnchorNames);
      }

      ::closedir(directory);
    }
  }

  // Remove old certificates.
  for (set<Name>::iterator name = oldAnchorNames.begin();
       name != oldAnchorNames.end(); ++name) {
    anchorNames_.erase(*name);
    certificates_.remove(*name);
  }
}

void
DynamicTrustAnchorGroup::loadCertificate
  (const string& file, set<Name>& oldAnchorNames)
{
  ptr_lib::shared_ptr<CertificateV2> certificate = readCertificate(file);
  if (certificate) {
    if (anchorNames_.count(certificate->getName()) == 0) {
      anchorNames_.insert(certificate->getName());
      certificates_.add(*certificate);
    }
    else
      oldAnchorNames.erase(certificate->getName());
  }
}

}
