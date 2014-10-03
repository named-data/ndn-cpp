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
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU Lesser General Public License is in the file COPYING.
 */

#include <float.h>
// We can use ndnboost::iostreams because this is internal and will not conflict with the application if it uses boost::iostreams.
#include <ndnboost/iostreams/stream.hpp>
#include <ndnboost/iostreams/device/array.hpp>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include "../../encoding/der/der.hpp"
#include "../../encoding/der/visitor/certificate-data-visitor.hpp"
#include "../../encoding/der/visitor/print-visitor.hpp"
#include "../../util/blob-stream.hpp"
#include "../../c/util/time.h"
#include <ndn-cpp/security/certificate/certificate.hpp>

using namespace std;

namespace ndn {

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
Certificate::isTooEarly()
{
  MillisecondsSince1970 now = ndn_getNowMilliseconds();
  if(now < notBefore_)
    return true;
  else
    return false;
}

bool
Certificate::isTooLate()
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
  ptr_lib::shared_ptr<der::DerSequence> root(new der::DerSequence());

  ptr_lib::shared_ptr<der::DerSequence> validity(new der::DerSequence());
  ptr_lib::shared_ptr<der::DerGtime> notBefore(new der::DerGtime(notBefore_));
  ptr_lib::shared_ptr<der::DerGtime> notAfter(new der::DerGtime(notAfter_));
  validity->addChild(notBefore);
  validity->addChild(notAfter);
  root->addChild(validity);

  ptr_lib::shared_ptr<der::DerSequence> subjectList(new der::DerSequence());
  SubjectDescriptionList::iterator it = subjectDescriptionList_.begin();
  for(; it != subjectDescriptionList_.end(); it++)
    {
      ptr_lib::shared_ptr<der::DerNode> child = it->toDer();
      subjectList->addChild(child);
    }
  root->addChild(subjectList);

  root->addChild(key_.toDer());

  if(!extensionList_.empty())
    {
      ptr_lib::shared_ptr<der::DerSequence> extnList(new der::DerSequence());
      ExtensionList::iterator it = extensionList_.begin();
      for(; it != extensionList_.end(); it++)
        extnList->addChild(it->toDer());
      root->addChild(extnList);
    }

  blob_stream blobStream;
  der::OutputIterator& start = reinterpret_cast<der::OutputIterator&>(blobStream);

  root->encode(start);

  ptr_lib::shared_ptr<vector<uint8_t> > blob = blobStream.buf();
  setContent(Blob(blob, false));
  getMetaInfo().setType(ndn_ContentType_KEY);
}

void
Certificate::decode()
{
  Blob blob = getContent();

  ndnboost::iostreams::stream<ndnboost::iostreams::array_source> is((const char*)blob.buf(), blob.size());

  ptr_lib::shared_ptr<der::DerNode> node = der::DerNode::parse(reinterpret_cast<der::InputIterator&>(is));

  // der::PrintVisitor printVisitor;
  // node->accept(printVisitor, string(""));

  der::CertificateDataVisitor certDataVisitor;
  node->accept(certDataVisitor, this);
}

void
Certificate::printCertificate()
{
  cout << "Validity:" << endl;
  cout << der::DerGtime::toIsoString(notBefore_) << endl;
  cout << der::DerGtime::toIsoString(notAfter_) << endl;

  cout << "Subject Info:" << endl;
  vector<CertificateSubjectDescription>::iterator it = subjectDescriptionList_.begin();
  for(; it < subjectDescriptionList_.end(); it++){
    cout << it->getOidString() << "\t" << it->getValue() << endl;
  }

  ndnboost::iostreams::stream<ndnboost::iostreams::array_source> is((const char*)key_.getKeyDer().buf(), key_.getKeyDer().size());

  ptr_lib::shared_ptr<der::DerNode> keyRoot = der::DerNode::parse(reinterpret_cast<der::InputIterator&> (is));

  der::PrintVisitor printVisitor;
  keyRoot->accept(printVisitor, string(""));
}

}
