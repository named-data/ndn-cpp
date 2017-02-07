/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
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

#include <math.h>
#include <ndn-cpp/util/logging.hpp>
#include <ndn-cpp/encrypt/algo/encryptor.hpp>
#include <ndn-cpp/encrypt/algo/aes-algorithm.hpp>
#include <ndn-cpp/encrypt/schedule.hpp>
#include <ndn-cpp/encrypt/producer.hpp>

using namespace std;
using namespace ndn::func_lib;

INIT_LOGGER("ndn.Producer");

namespace ndn {

void
Producer::defaultOnError(EncryptError::ErrorCode errorCode, const string& message)
{
  // Do nothing.
}

Producer::Impl::Impl
  (const Name& prefix, const Name& dataType, Face* face, KeyChain* keyChain,
   const ptr_lib::shared_ptr<ProducerDb>& database, int repeatAttempts,
   const Link& keyRetrievalLink)
  : face_(face),
    keyChain_(keyChain),
    database_(database),
    maxRepeatAttempts_(repeatAttempts),
    keyRetrievalLink_(keyRetrievalLink)
{
  Name fixedPrefix(prefix);
  Name fixedDataType(dataType);

  // Fill ekeyInfo_ with all permutations of dataType, including the 'E-KEY'
  // component of the name. This will be used in createContentKey to send
  // interests without reconstructing names every time.
  fixedPrefix.append(Encryptor::getNAME_COMPONENT_READ());
  while (fixedDataType.size() > 0) {
    Name nodeName(fixedPrefix);
    nodeName.append(fixedDataType);
    nodeName.append(Encryptor::getNAME_COMPONENT_E_KEY());

    eKeyInfo_[nodeName] = ptr_lib::make_shared<KeyInfo>();
    fixedDataType = fixedDataType.getPrefix(-1);
  }
  
  fixedPrefix.append(dataType);
  namespace_ = Name(prefix);
  namespace_.append(Encryptor::getNAME_COMPONENT_SAMPLE());
  namespace_.append(dataType);
}

Name
Producer::Impl::createContentKey
  (MillisecondsSince1970 timeSlot, const OnEncryptedKeys& onEncryptedKeys,
   const EncryptError::OnError& onError)
{
  MillisecondsSince1970 hourSlot = getRoundedTimeSlot(timeSlot);

  // Create the content key name.
  Name contentKeyName(namespace_);
  contentKeyName.append(Encryptor::getNAME_COMPONENT_C_KEY());
  contentKeyName.append(Schedule::toIsoString(hourSlot));

  Blob contentKeyBits;

  // Check if we have created the content key before.
  if (database_->hasContentKey(timeSlot))
    // We have created the content key. Return its name directly.
    return contentKeyName;

  // We haven't created the content key. Create one and add it into the database.
  AesKeyParams aesParams(128);
  contentKeyBits = AesAlgorithm::generateKey(aesParams).getKeyBits();
  database_->addContentKey(timeSlot, contentKeyBits);

  // Now we need to retrieve the E-KEYs for content key encryption.
  MillisecondsSince1970 timeCount = ::round(timeSlot);
  keyRequests_[timeCount] = ptr_lib::make_shared<KeyRequest>(eKeyInfo_.size());
  ptr_lib::shared_ptr<KeyRequest> keyRequest = keyRequests_[timeCount];

  // Check if the current E-KEYs can cover the content key.
  Exclude timeRange;
  excludeAfter(timeRange, Name::Component(Schedule::toIsoString(timeSlot)));
  for (map<Name, ptr_lib::shared_ptr<KeyInfo> >::iterator i = eKeyInfo_.begin();
       i != eKeyInfo_.end(); ++i) {
    // For each current E-KEY.
    const KeyInfo& keyInfo = *i->second;
    if (timeSlot < keyInfo.beginTimeSlot || timeSlot >= keyInfo.endTimeSlot) {
      // The current E-KEY cannot cover the content key, so retrieve one.
      keyRequest->repeatAttempts[i->first] = 0;
      sendKeyInterest
        (Interest(i->first).setExclude(timeRange).setChildSelector(1),
         timeSlot, onEncryptedKeys, onError);
    }
    else {
      // The current E-KEY can cover the content key.
      // Encrypt the content key directly.
      Name eKeyName(i->first);
      eKeyName.append(Schedule::toIsoString(keyInfo.beginTimeSlot));
      eKeyName.append(Schedule::toIsoString(keyInfo.endTimeSlot));
      encryptContentKey
        (keyInfo.keyBits, eKeyName, timeSlot, onEncryptedKeys, onError);
    }
  }

  return contentKeyName;
}

void
Producer::Impl::produce
  (Data& data, MillisecondsSince1970 timeSlot, const Blob& content,
   const EncryptError::OnError& onError)
{
  // Get a content key.
  Name contentKeyName = createContentKey(timeSlot, OnEncryptedKeys(), onError);
  Blob contentKey = database_->getContentKey(timeSlot);

  // Produce data.
  Name dataName(namespace_);
  dataName.append(Schedule::toIsoString(timeSlot));

  data.setName(dataName);
  EncryptParams params(ndn_EncryptAlgorithmType_AesCbc, 16);
  Encryptor::encryptData(data, content, contentKeyName, contentKey, params);
  keyChain_->sign(data);
}

MillisecondsSince1970
Producer::Impl::getRoundedTimeSlot(MillisecondsSince1970 timeSlot)
{
  return ::round(::floor(::round(timeSlot) / 3600000.0) * 3600000.0);
}

void
Producer::Impl::sendKeyInterest
  (const Interest& interest, MillisecondsSince1970 timeSlot,
   const OnEncryptedKeys& onEncryptedKeys,
   const EncryptError::OnError& onError)
{
  ptr_lib::shared_ptr<Interest> interestWithLink;
  const Interest* request;
  if (keyRetrievalLink_.getDelegations().size() == 0)
    // We can use the supplied interest without copying.
    request = &interest;
  else {
    // Copy the supplied interest and add the Link.
    interestWithLink.reset(new Interest(interest));
    // This will use a cached encoding if available.
    interestWithLink->setLinkWireEncoding(keyRetrievalLink_.wireEncode());

    request = interestWithLink.get();
  }

  face_->expressInterest
    (*request,
     bind(&Producer::Impl::handleCoveringKey, shared_from_this(), _1, _2,
          timeSlot, onEncryptedKeys, onError),
     bind(&Producer::Impl::handleTimeout, shared_from_this(), _1, timeSlot,
          onEncryptedKeys, onError),
     bind(&Producer::Impl::handleNetworkNack, shared_from_this(), _1, _2,
          timeSlot, onEncryptedKeys, onError));
}

void
Producer::Impl::handleTimeout
  (const ptr_lib::shared_ptr<const Interest>& interest,
   MillisecondsSince1970 timeSlot, const OnEncryptedKeys& onEncryptedKeys,
   const EncryptError::OnError& onError)
{
  MillisecondsSince1970 timeCount = ::round(timeSlot);
  ptr_lib::shared_ptr<KeyRequest> keyRequest = keyRequests_[timeCount];

  const Name& interestName = interest->getName();
  if (keyRequest->repeatAttempts[interestName] < maxRepeatAttempts_) {
    // Increase the retrial count.
    ++keyRequest->repeatAttempts[interestName];
    sendKeyInterest(*interest, timeSlot, onEncryptedKeys, onError);
  }
  else
    // Treat an eventual timeout as a network Nack.
    handleNetworkNack
      (interest, ptr_lib::make_shared<NetworkNack>(), timeSlot, onEncryptedKeys,
       onError);
}

void
Producer::Impl::handleNetworkNack
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<NetworkNack>& networkNack,
   MillisecondsSince1970 timeSlot,
   const OnEncryptedKeys& onEncryptedKeys,
   const EncryptError::OnError& onError)
{
  // We have run out of options....
  MillisecondsSince1970 timeCount = ::round(timeSlot);
  updateKeyRequest(keyRequests_[timeCount], timeCount, onEncryptedKeys);
}

void
Producer::Impl::updateKeyRequest
  (const ptr_lib::shared_ptr<KeyRequest>& keyRequest,
   MillisecondsSince1970 timeCount, const OnEncryptedKeys& onEncryptedKeys)
{
  --keyRequest->interestCount;
  if (keyRequest->interestCount == 0 && onEncryptedKeys) {
    try {
      onEncryptedKeys(keyRequest->encryptedKeys);
    } catch (const std::exception& ex) {
      _LOG_ERROR("Producer::Impl::updateKeyRequest: Error in onEncryptedKeys: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Producer::Impl::updateKeyRequest: Error in onEncryptedKeys.");
    }
    keyRequests_.erase(timeCount);
  }
}

void
Producer::Impl::handleCoveringKey
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data, MillisecondsSince1970 timeSlot,
   const OnEncryptedKeys& onEncryptedKeys,
   const EncryptError::OnError& onError)
{
  MillisecondsSince1970 timeCount = ::round(timeSlot);
  ptr_lib::shared_ptr<KeyRequest> keyRequest = keyRequests_[timeCount];

  const Name& interestName = interest->getName();
  const Name& keyName = data->getName();

  MillisecondsSince1970 begin = Schedule::fromIsoString
    (keyName.get(START_TIME_STAMP_INDEX).getValue().toRawStr());
  MillisecondsSince1970 end = Schedule::fromIsoString
    (keyName.get(END_TIME_STAMP_INDEX).getValue().toRawStr());

  if (timeSlot >= end) {
    // If the received E-KEY covers some earlier period, try to retrieve an
    // E-KEY covering a later one.
    Exclude timeRange(interest->getExclude());
    excludeBefore(timeRange, keyName.get(START_TIME_STAMP_INDEX));
    keyRequest->repeatAttempts[interestName] = 0;

    sendKeyInterest
      (Interest(interestName).setExclude(timeRange).setChildSelector(1),
       timeSlot, onEncryptedKeys, onError);
  }
  else {
    // If the received E-KEY covers the content key, encrypt the content.
    const Blob& encryptionKey = data->getContent();
    // If everything is correct, save the E-KEY as the current key.
    if (encryptContentKey
        (encryptionKey, keyName, timeSlot, onEncryptedKeys, onError)) {
      ptr_lib::shared_ptr<KeyInfo> keyInfo = eKeyInfo_[interestName];
      keyInfo->beginTimeSlot = begin;
      keyInfo->endTimeSlot = end;
      keyInfo->keyBits = encryptionKey;
    }
  }
}

bool
Producer::Impl::encryptContentKey
  (const Blob& encryptionKey, const Name& eKeyName,
   MillisecondsSince1970 timeSlot, const OnEncryptedKeys& onEncryptedKeys,
   const EncryptError::OnError& onError)
{
  MillisecondsSince1970 timeCount = ::round(timeSlot);
  ptr_lib::shared_ptr<KeyRequest> keyRequest = keyRequests_[timeCount];

  Name keyName(namespace_);
  keyName.append(Encryptor::getNAME_COMPONENT_C_KEY());
  keyName.append(Schedule::toIsoString(getRoundedTimeSlot(timeSlot)));

  Blob contentKey = database_->getContentKey(timeSlot);

  ptr_lib::shared_ptr<Data> cKeyData(new Data());
  cKeyData->setName(keyName);
  EncryptParams params(ndn_EncryptAlgorithmType_RsaOaep);
  try {
    Encryptor::encryptData
      (*cKeyData, contentKey, eKeyName, encryptionKey, params);
  } catch (const std::exception& ex) {
    try {
      onError(EncryptError::ErrorCode::EncryptionFailure, ex.what());
    } catch (const std::exception& ex) {
      _LOG_ERROR("Error in onError: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Error in onError.");
    }
    return false;
  }

  keyChain_->sign(*cKeyData);
  keyRequest->encryptedKeys.push_back(cKeyData);
  updateKeyRequest(keyRequest, timeCount, onEncryptedKeys);
  return true;
}

void
Producer::Impl::getExcludeEntries
  (const Exclude& exclude, vector<ExcludeEntry>& entries)
{
  entries.clear();

  for (size_t i = 0; i < exclude.size(); ++i) {
    if (exclude[i].getType() == ndn_Exclude_ANY) {
      if (entries.size() == 0)
        // Add a "beginning ANY".
        entries.push_back(ExcludeEntry(Name::Component(), true));
      else
        // Set anyFollowsComponent of the final component.
        entries[entries.size() - 1].anyFollowsComponent_ = true;
    }
    else
      entries.push_back(ExcludeEntry(exclude[i].getComponent(), false));
  }
}

void
Producer::Impl::setExcludeEntries
  (Exclude& exclude, const vector<ExcludeEntry>& entries)
{
  exclude.clear();

  for (size_t i = 0; i < entries.size(); ++i) {
    const ExcludeEntry& entry = entries[i];

    if (i == 0 && entry.component_.getValue().size() == 0 &&
        entry.anyFollowsComponent_)
      // This is a "beginning ANY".
      exclude.appendAny();
    else {
      exclude.appendComponent(entry.component_);
      if (entry.anyFollowsComponent_)
        exclude.appendAny();
    }
  }
}

int
Producer::Impl::findEntryBeforeOrAt
  (const vector<ExcludeEntry>& entries, const Name::Component& component)
{
  int i = entries.size() - 1;
  while (i >= 0) {
    if (entries[i].component_.compare(component) <= 0)
      break;
    --i;
  }

  return i;
}

void
Producer::Impl::excludeAfter(Exclude& exclude, const Name::Component& from)
{
  vector<ExcludeEntry> entries;
  getExcludeEntries(exclude, entries);

  int iNewFrom;
  int iFoundFrom = findEntryBeforeOrAt(entries, from);
  if (iFoundFrom < 0) {
    // There is no entry before "from" so insert at the beginning.
    entries.insert(entries.begin(), ExcludeEntry(from, true));
    iNewFrom = 0;
  }
  else {
    ExcludeEntry& foundFrom = entries[iFoundFrom];

    if (!foundFrom.anyFollowsComponent_) {
      if (foundFrom.component_.equals(from)) {
        // There is already an entry with "from", so just set the "ANY" flag.
        foundFrom.anyFollowsComponent_ = true;
        iNewFrom = iFoundFrom;
      }
      else {
        // Insert following the entry before "from".
        entries.insert(entries.begin() + iFoundFrom + 1, ExcludeEntry(from, true));
        iNewFrom = iFoundFrom + 1;
      }
    }
    else
      // The entry before "from" already has an "ANY" flag, so do nothing.
      iNewFrom = iFoundFrom;
  }

  // Remove entries after the new "from".
  int iRemoveBegin = iNewFrom + 1;
  int nRemoveNeeded = entries.size() - iRemoveBegin;
  for (int i = 0; i < nRemoveNeeded; ++i)
    entries.erase(entries.begin() + iRemoveBegin);

  setExcludeEntries(exclude, entries);
}

void
Producer::Impl::excludeRange
  (Exclude& exclude, const Name::Component& from, const Name::Component& to)
{
  if (from.compare(to) >= 0) {
    if (from.compare(to) == 0)
      throw runtime_error
        ("excludeRange: from == to. To exclude a single component, sue excludeOne.");
    else
      throw runtime_error
        ("excludeRange: from must be less than to. Invalid range: [" +
         from.toEscapedString() + ", " + to.toEscapedString() + "]");
  }

  vector<ExcludeEntry> entries;
  getExcludeEntries(exclude, entries);

  int iNewFrom;
  int iFoundFrom = findEntryBeforeOrAt(entries, from);
  if (iFoundFrom < 0) {
    // There is no entry before "from" so insert at the beginning.
    entries.insert(entries.begin(), ExcludeEntry(from, true));
    iNewFrom = 0;
  }
  else {
    ExcludeEntry& foundFrom = entries[iFoundFrom];

    if (!foundFrom.anyFollowsComponent_) {
      if (foundFrom.component_.equals(from)) {
        // There is already an entry with "from", so just set the "ANY" flag.
        foundFrom.anyFollowsComponent_ = true;
        iNewFrom = iFoundFrom;
      }
      else {
        // Insert following the entry before "from".
        entries.insert(entries.begin() + iFoundFrom + 1, ExcludeEntry(from, true));
        iNewFrom = iFoundFrom + 1;
      }
    }
    else
      // The entry before "from" already has an "ANY" flag, so do nothing.
      iNewFrom = iFoundFrom;
  }

  // We have at least one "from" before "to", so we know this will find an entry.
  int iFoundTo = findEntryBeforeOrAt(entries, to);
  ExcludeEntry& foundTo = entries[iFoundTo];
  if (iFoundTo == iNewFrom)
    // Insert the "to" immediately after the "from".
    entries.insert(entries.begin() + iNewFrom + 1, ExcludeEntry(to, false));
  else {
    int iRemoveEnd;
    if (!foundTo.anyFollowsComponent_) {
      if (foundTo.component_.equals(to))
        // The "to" entry already exists. Remove up to it.
        iRemoveEnd = iFoundTo;
      else {
        // Insert following the previous entry, which will be removed.
        entries.insert(entries.begin() + iFoundTo + 1, ExcludeEntry(to, false));
        iRemoveEnd = iFoundTo + 1;
      }
    }
    else
      // "to" follows a component which is already followed by "ANY", meaning
      // the new range now encompasses it, so remove the component.
      iRemoveEnd = iFoundTo + 1;

    // Remove intermediate entries since they are inside the range.
    int iRemoveBegin = iNewFrom + 1;
    int nRemoveNeeded = iRemoveEnd - iRemoveBegin;
    for (int i = 0; i < nRemoveNeeded; ++i)
      entries.erase(entries.begin() + iRemoveBegin);
  }

  setExcludeEntries(exclude, entries);
}

Link* Producer::noLink_ = 0;

}
