/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/group-manager https://github.com/named-data/ndn-group-encrypt
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

#include <ndn-cpp/security/key-chain.hpp>
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/encrypt/algo/encryptor.hpp>
#include <ndn-cpp/encrypt/algo/rsa-algorithm.hpp>
#include <ndn-cpp/encrypt/group-manager.hpp>

using namespace std;

namespace ndn {

GroupManager::GroupManager
  (const Name& prefix, const Name& dataType,
   const ptr_lib::shared_ptr<GroupManagerDb>& database, uint32_t keySize,
   int freshnessHours, KeyChain* keyChain)
: database_(database),
  keySize_(keySize),
  freshnessHours_(freshnessHours),
  keyChain_(keyChain)
{
  namespace_ = Name(prefix).append(Encryptor::getNAME_COMPONENT_READ())
    .append(dataType);
}

void
GroupManager::getGroupKey
  (MillisecondsSince1970 timeSlot, vector<ptr_lib::shared_ptr<Data> >& result)
{
  result.clear();
  map<Name, Blob> memberKeys;

  // Get the time interval.
  Interval finalInterval = calculateInterval(timeSlot, memberKeys);
  if (finalInterval.isValid() == false)
    return;

  string startTimeStamp = Schedule::toIsoString(finalInterval.getStartTime());
  string endTimeStamp = Schedule::toIsoString(finalInterval.getEndTime());

  // Generate the private and public keys.
  Blob privateKeyBlob;
  Blob publicKeyBlob;
  generateKeyPair(privateKeyBlob, publicKeyBlob);

  // Add the first element to the result.
  // The E-KEY (public key) data packet name convention is:
  // /<data_type>/E-KEY/[start-ts]/[end-ts]
  ptr_lib::shared_ptr<Data> data = createEKeyData
    (startTimeStamp, endTimeStamp, publicKeyBlob);
  result.push_back(data);

  // Encrypt the private key with the public key from each member's certificate.
  for (map<Name, Blob>::iterator i = memberKeys.begin(); i != memberKeys.end(); ++i) {
    const Name& keyName = i->first;
    Blob& certificateKey = i->second;

    // Generate the name of the packet.
    // The D-KEY (private key) data packet name convention is:
    // /<data_type>/D-KEY/[start-ts]/[end-ts]/[member-name]
    data = createDKeyData
      (startTimeStamp, endTimeStamp, keyName, privateKeyBlob, certificateKey);
    result.push_back(data);
  }
}

Interval
GroupManager::calculateInterval
  (MillisecondsSince1970 timeSlot, map<Name, Blob>& memberKeys)
{
  // Prepare.
  Interval positiveResult;
  Interval negativeResult;
  memberKeys.clear();

  // Get the all intervals from the schedules.
  vector<string> scheduleNames;
  database_->listAllScheduleNames(scheduleNames);
  for (size_t i = 0; i < scheduleNames.size(); ++i) {
    string& scheduleName = scheduleNames[i];

    ptr_lib::shared_ptr<Schedule> schedule = database_->getSchedule(scheduleName);
    Schedule::Result result = schedule->getCoveringInterval(timeSlot);
    Interval tempInterval = result.interval;

    if (result.isPositive) {
      if (!positiveResult.isValid())
        positiveResult = tempInterval;
      positiveResult.intersectWith(tempInterval);

      map<Name, Blob> map;
      database_->getScheduleMembers(scheduleName, map);
      memberKeys.insert(map.begin(), map.end());
    }
    else {
      if (!negativeResult.isValid())
        negativeResult = tempInterval;
      negativeResult.intersectWith(tempInterval);
    }
  }
  if (!positiveResult.isValid())
    // Return an invalid interval when there is no member which has an
    // interval covering the time slot.
    return Interval(false);

  // Get the final interval result.
  Interval finalInterval;
  if (negativeResult.isValid())
    finalInterval = positiveResult.intersectWith(negativeResult);
  else
    finalInterval = positiveResult;

  return finalInterval;
}

void
GroupManager::generateKeyPair(Blob& privateKeyBlob, Blob& publicKeyBlob)
{
  RsaKeyParams params(keySize_);

  DecryptKey privateKey = RsaAlgorithm::generateKey(params);
  privateKeyBlob = privateKey.getKeyBits();

  EncryptKey publicKey = RsaAlgorithm::deriveEncryptKey(privateKeyBlob);
  publicKeyBlob = publicKey.getKeyBits();
}

ptr_lib::shared_ptr<Data>
GroupManager::createEKeyData
  (const string& startTimeStamp, const string& endTimeStamp,
   const Blob& publicKeyBlob)
{
  Name name(namespace_);
  name.append(Encryptor::getNAME_COMPONENT_E_KEY()).append(startTimeStamp)
    .append(endTimeStamp);

  ptr_lib::shared_ptr<Data> data(new Data(name));
  data->getMetaInfo().setFreshnessPeriod(freshnessHours_ * MILLISECONDS_IN_HOUR);
  data->setContent(publicKeyBlob);
  keyChain_->sign(*data);
  return data;
}

ptr_lib::shared_ptr<Data>
GroupManager::createDKeyData
  (const string& startTimeStamp, const string& endTimeStamp,
   const Name& keyName, const Blob& privateKeyBlob, const Blob& certificateKey)
{
  Name name(namespace_);
  name.append(Encryptor::getNAME_COMPONENT_D_KEY());
  name.append(startTimeStamp).append(endTimeStamp);
  ptr_lib::shared_ptr<Data> data(new Data(name));
  data->getMetaInfo().setFreshnessPeriod(freshnessHours_ * MILLISECONDS_IN_HOUR);
  EncryptParams encryptParams(ndn_EncryptAlgorithmType_RsaOaep);
  try {
    Encryptor::encryptData
      (*data, privateKeyBlob, keyName, certificateKey, encryptParams);
  } catch (const std::exception& ex) {
    // Consolidate errors such as InvalidKeyException.
    throw SecurityException
      (string("createDKeyData: Error in encryptData: ") + ex.what());
  }

  keyChain_->sign(*data);
  return data;
}

}
