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
#include <ndn-cpp/security/certificate/certificate-subject-description.hpp>

using namespace std;

namespace ndn {

ptr_lib::shared_ptr<DerNode>
CertificateSubjectDescription::toDer() const
{
  ptr_lib::shared_ptr<DerNode::DerSequence> root(new DerNode::DerSequence());

  ptr_lib::shared_ptr<DerNode::DerOid> oid(new DerNode::DerOid(oid_));
  ptr_lib::shared_ptr<DerNode::DerPrintableString> value
    (new DerNode::DerPrintableString((const uint8_t*)&value_[0], value_.size()));

  root->addChild(oid);
  root->addChild(value);

  return root;
}

}
