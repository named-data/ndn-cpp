/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include <stdexcept>
#include <ndn-cpp/common.hpp>
#include <ndn-cpp/interest.hpp>
#include "c/interest.h"

using namespace std;

namespace ndn {
  
void 
Exclude::Entry::get(struct ndn_ExcludeEntry& excludeEntryStruct) const 
{
  excludeEntryStruct.type = type_;
  if (type_ == ndn_Exclude_COMPONENT)
    component_.get(excludeEntryStruct.component);
}

void 
Exclude::get(struct ndn_Exclude& excludeStruct) const
{
  if (excludeStruct.maxEntries < entries_.size())
    throw runtime_error("excludeStruct.maxEntries must be >= this exclude getEntryCount()");
  
  excludeStruct.nEntries = entries_.size();
  for (size_t i = 0; i < excludeStruct.nEntries; ++i)
    entries_[i].get(excludeStruct.entries[i]);  
}

void 
Exclude::set(const struct ndn_Exclude& excludeStruct)
{
  clear();
  for (size_t i = 0; i < excludeStruct.nEntries; ++i) {
    ndn_ExcludeEntry *entry = &excludeStruct.entries[i];
    
    if (entry->type == ndn_Exclude_COMPONENT)
      appendComponent(entry->component.value.value, entry->component.value.length);
    else if (entry->type == ndn_Exclude_ANY)
      appendAny();
    else
      throw runtime_error("unrecognized ndn_ExcludeType");
  }
}

string 
Exclude::toUri() const
{
  if (entries_.size() == 0)
    return "";

  ostringstream result;
  for (unsigned i = 0; i < entries_.size(); ++i) {
    if (i > 0)
      result << ",";
        
    if (entries_[i].getType() == ndn_Exclude_ANY)
      result << "*";
    else
      Name::toEscapedString(*entries_[i].getComponent().getValue(), result);
  }
  
  return result.str();  
}

void 
Interest::set(const struct ndn_Interest& interestStruct) 
{
  name_.get().set(interestStruct.name);
  setMinSuffixComponents(interestStruct.minSuffixComponents);
  setMaxSuffixComponents(interestStruct.maxSuffixComponents);
  
  publisherPublicKeyDigest_.get().set(interestStruct.publisherPublicKeyDigest);
  keyLocator_.get().set(interestStruct.keyLocator);
  
  exclude_.get().set(interestStruct.exclude);
  setChildSelector(interestStruct.childSelector);
  setAnswerOriginKind(interestStruct.answerOriginKind);
  setScope(interestStruct.scope);
  setInterestLifetimeMilliseconds(interestStruct.interestLifetimeMilliseconds);
  // Set the nonce last so that getNonceChangeCount_ is set correctly.
  setNonce(Blob(interestStruct.nonce));
}

void 
Interest::get(struct ndn_Interest& interestStruct) const 
{
  name_.get().get(interestStruct.name);
  interestStruct.minSuffixComponents = minSuffixComponents_;
  interestStruct.maxSuffixComponents = maxSuffixComponents_;
  publisherPublicKeyDigest_.get().get(interestStruct.publisherPublicKeyDigest);
  keyLocator_.get().get(interestStruct.keyLocator);
  exclude_.get().get(interestStruct.exclude);
  interestStruct.childSelector = childSelector_;
  interestStruct.answerOriginKind = answerOriginKind_;
  interestStruct.scope = scope_;
  interestStruct.interestLifetimeMilliseconds = interestLifetimeMilliseconds_;
  getNonce().get(interestStruct.nonce);
}

string 
Interest::toUri() const
{
  ostringstream selectors;

  if (minSuffixComponents_ >= 0)
    selectors << "&ndn.MinSuffixComponents=" << minSuffixComponents_;
  if (maxSuffixComponents_ >= 0)
    selectors << "&ndn.MaxSuffixComponents=" << maxSuffixComponents_;
  if (childSelector_ >= 0)
    selectors << "&ndn.ChildSelector=" << childSelector_;
  if (answerOriginKind_ >= 0)
    selectors << "&ndn.AnswerOriginKind=" << answerOriginKind_;
  if (scope_ >= 0)
    selectors << "&ndn.Scope=" << scope_;
  if (interestLifetimeMilliseconds_ >= 0)
    selectors << "&ndn.InterestLifetime=" << interestLifetimeMilliseconds_;
  if (publisherPublicKeyDigest_.get().getPublisherPublicKeyDigest().size() > 0) {
    selectors << "&ndn.PublisherPublicKeyDigest=";
    Name::toEscapedString(*publisherPublicKeyDigest_.get().getPublisherPublicKeyDigest(), selectors);
  }
  if (getNonce().size() > 0) {
    selectors << "&ndn.Nonce=";
    Name::toEscapedString(*getNonce(), selectors);
  }
  if (exclude_.get().size() > 0)
    selectors << "&ndn.Exclude=" << exclude_.get().toUri();

  ostringstream result;

  result << name_.get().toUri();
  string selectorsString(selectors.str());
  if (selectorsString.size() > 0) {
    // Replace the first & with ?.
    result << "?";
    result.write(&selectorsString[1], selectorsString.size() - 1);
  }
  
  return result.str();  
}

}

