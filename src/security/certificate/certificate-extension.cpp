/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Yingdi Yu <yingdi@cs.ucla.edu>
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#include "../../encoding/der/der.hpp"
#include "../../util/blob-stream.hpp"
#include <ndn-cpp/security/certificate/certificate-extension.hpp>

using namespace std;

namespace ndn {

ptr_lib::shared_ptr<der::DerNode>
CertificateExtension::toDer() const
{
  ptr_lib::shared_ptr<der::DerSequence> root(new der::DerSequence);

  ptr_lib::shared_ptr<der::DerOid> extensionId(new der::DerOid(extensionId_));
  ptr_lib::shared_ptr<der::DerBool> isCritical(new der::DerBool(isCritical_));
  ptr_lib::shared_ptr<der::DerOctetString> extensionValue(new der::DerOctetString(*extensionValue_));

  root->addChild(extensionId);
  root->addChild(isCritical);
  root->addChild(extensionValue);

  root->getSize();

  return root;
}

Blob
CertificateExtension::toDerBlob() const
{
  blob_stream blobStream;
  der::OutputIterator& start = reinterpret_cast<der::OutputIterator&>(blobStream);

  toDer()->encode(start);

  return Blob(blobStream.buf(), false);
}


}
