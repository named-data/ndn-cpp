/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
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
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU Lesser General Public License is in the file COPYING.
 */

#include "../../encoding/der/der-node.hpp"
#include <ndn-cpp/security/certificate/certificate-extension.hpp>

using namespace std;

namespace ndn {

ptr_lib::shared_ptr<DerNode>
CertificateExtension::toDer() const
{
  ptr_lib::shared_ptr<DerNode::DerSequence> root(new DerNode::DerSequence);

  ptr_lib::shared_ptr<DerNode::DerOid> extensionId
    (new DerNode::DerOid(extensionId_));
  ptr_lib::shared_ptr<DerNode::DerBoolean> isCritical
    (new DerNode::DerBoolean(isCritical_));
  ptr_lib::shared_ptr<DerNode::DerOctetString> extensionValue
    (new DerNode::DerOctetString(extensionValue_.buf(), extensionValue_.size()));

  root->addChild(extensionId);
  root->addChild(isCritical);
  root->addChild(extensionValue);

  root->getSize();

  return root;
}

Blob
CertificateExtension::toDerBlob() const
{
  return toDer()->encode();
}
}
