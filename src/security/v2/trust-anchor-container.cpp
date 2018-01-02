/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/trust-anchor-container.cpp
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

#include <ndn-cpp/interest.hpp>
#include <ndn-cpp/security/v2/trust-anchor-container.hpp>

using namespace std;

namespace ndn {

void
TrustAnchorContainer::insert
  (const string& groupId, const CertificateV2& certificate)
{
  ptr_lib::shared_ptr<TrustAnchorGroup> group;
  std::map<string, ptr_lib::shared_ptr<TrustAnchorGroup> >::iterator groupIt =
    groups_.find(groupId);
  if (groupIt == groups_.end()) {
    group.reset(new StaticTrustAnchorGroup(anchors_, groupId));
    groups_[groupId] = group;
  }
  else
    group = groupIt->second;

  StaticTrustAnchorGroup* staticGroup = dynamic_cast<StaticTrustAnchorGroup*>
    (group.get());
  if (!staticGroup)
    throw Error
      ("Cannot add a static anchor to the non-static anchor group " + groupId);

  staticGroup->add(certificate);
}

void
TrustAnchorContainer::insert
  (const string& groupId, const string& path, Milliseconds refreshPeriod,
   bool isDirectory)
{
  if (groups_.count(groupId) != 0)
    throw Error("Cannot create the dynamic group, because group " + groupId +
      " already exists");

  groups_[groupId] = ptr_lib::make_shared<DynamicTrustAnchorGroup>
    (anchors_, groupId, path, refreshPeriod, isDirectory);
}

ptr_lib::shared_ptr<CertificateV2>
TrustAnchorContainer::find(const Name& keyName) const
{
  const_cast<TrustAnchorContainer*>(this)->refresh();

  map<Name, ptr_lib::shared_ptr<CertificateV2> >::const_iterator it =
    anchors_.anchorsByName_.lower_bound(keyName);
  if (it ==  anchors_.anchorsByName_.end() ||
      !keyName.isPrefixOf(it->second->getName()))
    return ptr_lib::shared_ptr<CertificateV2>();
  return it->second;
}

ptr_lib::shared_ptr<CertificateV2>
TrustAnchorContainer::find(const Interest& interest) const
{
  const_cast<TrustAnchorContainer*>(this)->refresh();

  for (map<Name, ptr_lib::shared_ptr<CertificateV2> >::const_iterator it =
         anchors_.anchorsByName_.lower_bound(interest.getName());
       it != anchors_.anchorsByName_.end() && 
         interest.getName().isPrefixOf(it->second->getName());
       ++it) {
    if (interest.matchesData(*it->second))
      return it->second;
  }

  return ptr_lib::shared_ptr<CertificateV2>();
}

TrustAnchorGroup&
TrustAnchorContainer::getGroup(const string& groupId)
{
  map<string, ptr_lib::shared_ptr<TrustAnchorGroup> >::iterator group =
    groups_.find(groupId);
  if (group == groups_.end())
    throw Error("Trust anchor group " + groupId + " does not exist");

  return *group->second;
}

void
TrustAnchorContainer::refresh()
{
  for (map<string, ptr_lib::shared_ptr<TrustAnchorGroup> >::iterator it =
       groups_.begin(); it != groups_.end(); ++it)
    it->second->refresh();
}

TrustAnchorContainer::AnchorContainer::~AnchorContainer()
{
}

void
TrustAnchorContainer::AnchorContainer::add(const CertificateV2& certificate)
{
  ptr_lib::shared_ptr<CertificateV2> certificateCopy
    (new CertificateV2(certificate));
  anchorsByName_[certificateCopy->getName()] = certificateCopy;
}

void
TrustAnchorContainer::AnchorContainer::remove(const Name& certificateName)
{
  anchorsByName_.erase(certificateName);
}

}
