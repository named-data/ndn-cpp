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

#include <float.h>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include "../../encoding/der/der-node.hpp"
#include "../../encoding/base64.hpp"
#include "../../util/blob-stream.hpp"
#include "../../c/util/time.h"
#include <ndn-cpp/security/certificate/certificate.hpp>

using namespace std;

namespace ndn {

typedef DerNode::DerSequence DerSequence;
typedef DerNode::DerGeneralizedTime DerGeneralizedTime;

Certificate::Certificate()
  : notBefore_(DBL_MAX)
  , notAfter_(-DBL_MAX)
{}

Certificate::Certificate(const Data& data)
// Use the copy constructor.  It clones the signature object.
: Data(data)
{
  decode();
}

Certificate::~Certificate()
{
  //TODO:
}

bool
Certificate::isTooEarly() const
{
  MillisecondsSince1970 now = ndn_getNowMilliseconds();
  if(now < notBefore_)
    return true;
  else
    return false;
}

bool
Certificate::isTooLate() const
{
  MillisecondsSince1970 now = ndn_getNowMilliseconds();
  if(now > notAfter_)
    return true;
  else
    return false;
}

void
Certificate::encode()
{
  ptr_lib::shared_ptr<DerNode> root = toDer();
  setContent(root->encode());
  getMetaInfo().setType(ndn_ContentType_KEY);
}

void
Certificate::wireDecode(const Blob& input, WireFormat& wireFormat)
{
  Data::wireDecode(input, wireFormat);
  decode();
}

ptr_lib::shared_ptr<DerNode>
Certificate::toDer()
{
  ptr_lib::shared_ptr<DerSequence> root(new DerSequence());
  ptr_lib::shared_ptr<DerSequence> validity(new DerSequence());
  ptr_lib::shared_ptr<DerGeneralizedTime> notBefore(new DerGeneralizedTime(notBefore_));
  ptr_lib::shared_ptr<DerGeneralizedTime> notAfter(new DerGeneralizedTime(notAfter_));

  validity->addChild(notBefore);
  validity->addChild(notAfter);

  root->addChild(validity);

  ptr_lib::shared_ptr<DerSequence> subjectList(new DerSequence());
  for (int i = 0; i < subjectDescriptionList_.size(); ++i)
    subjectList->addChild(subjectDescriptionList_[i].toDer());

  root->addChild(subjectList);
  root->addChild(key_.toDer());

  if (extensionList_.size() > 0) {
    ptr_lib::shared_ptr<DerSequence> extensionList(new DerSequence());
    for (int i = 0; i < extensionList_.size(); ++i)
      extensionList->addChild(extensionList_[i].toDer());
    root->addChild(extensionList);
  }

  return root;
}

void
Certificate::decode()
{
  ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse(getContent().buf());

  // We need to ensure that there are:
  //   validity (notBefore, notAfter)
  //   subject list
  //   public key
  //   (optional) extension list

  const std::vector<ptr_lib::shared_ptr<DerNode> >& rootChildren =
    parsedNode->getChildren();
  // 1st: validity info
  const std::vector<ptr_lib::shared_ptr<DerNode> >& validityChildren =
    DerNode::getSequence(rootChildren, 0).getChildren();
  notBefore_ = dynamic_cast<DerGeneralizedTime&>
    (*validityChildren[0]).toMillisecondsSince1970();
  notAfter_ = dynamic_cast<DerGeneralizedTime&>
    (*validityChildren[1]).toMillisecondsSince1970();

  // 2nd: subjectList
  const std::vector<ptr_lib::shared_ptr<DerNode> >& subjectChildren =
    DerNode::getSequence(rootChildren, 1).getChildren();
  for (int i = 0; i < subjectChildren.size(); ++i) {
    DerSequence& sd = DerNode::getSequence(subjectChildren, i);
    const std::vector<ptr_lib::shared_ptr<DerNode> >& descriptionChildren =
      sd.getChildren();
    string oidStr = descriptionChildren[0]->toVal().toRawStr();
    string value = descriptionChildren[1]->toVal().toRawStr();

    addSubjectDescription(CertificateSubjectDescription(oidStr, value));
  }

  // 3rd: public key
  Blob publicKeyInfo = rootChildren[2]->encode();
  ptr_lib::shared_ptr<PublicKey> tempPublicKey(new PublicKey(publicKeyInfo));
  key_ = *tempPublicKey;

  if (rootChildren.size() > 3) {
    const std::vector<ptr_lib::shared_ptr<DerNode> >& extensionChildren =
      DerNode::getSequence(rootChildren, 3).getChildren();
    for (int i = 0; i < extensionChildren.size(); ++i) {
      DerSequence& extInfo = DerNode::getSequence(extensionChildren, i);

      const std::vector<ptr_lib::shared_ptr<DerNode> >& children =
        extInfo.getChildren();
      string oidStr = children[0]->toVal().toRawStr();
      bool isCritical = children[1]->toVal().buf()[0] != 0;
      Blob value = children[2]->toVal();
      addExtension(CertificateExtension(oidStr, isCritical, value));
    }
  }
}

void
Certificate::printCertificate(ostream& os) const
{
  os << "Certificate name:" << endl;
  os << "  " << getName() << endl;
  os << "Validity:" << endl;
  os << "  NotBefore: " << DerGeneralizedTime::toIsoString(notBefore_) << endl;
  os << "  NotAfter: "  << DerGeneralizedTime::toIsoString(notAfter_)  << endl;

  os << "Subject Description:" << endl;
  vector<CertificateSubjectDescription>::const_iterator it = subjectDescriptionList_.begin();
  for(; it < subjectDescriptionList_.end(); it++)
    os << "  " << it->getOidString() << ": " << it->getValue() << endl;

  os << "Public key bits:" << endl;
  os << toBase64(key_.getKeyDer().buf(), key_.getKeyDer().size(), true);
}

void
Certificate::printCertificate() const
{
  printCertificate(cout);
}

}
