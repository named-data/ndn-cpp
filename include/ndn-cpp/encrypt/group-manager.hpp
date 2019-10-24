/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2019 Regents of the University of California.
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

#ifndef NDN_GROUP_MANAGER_HPP
#define NDN_GROUP_MANAGER_HPP

#include "../security/certificate/identity-certificate.hpp"
#include "group-manager-db.hpp"

// Give friend access to the tests.
class TestGroupManager_CreateDKeyData_Test;
class TestGroupManager_CreateEKeyData_Test;
class TestGroupManager_CalculateInterval_Test;

namespace ndn {

class KeyChain;

/**
 * A GroupManager manages keys and schedules for group members in a particular
 * namespace.
 * @note This class is an experimental feature. The API may change.
 */
class GroupManager {
public:
  /**
   * Create a group manager with the given values. The group manager namespace
   * is /{prefix}/read/{dataType} .
   * @param prefix The prefix for the group manager namespace.
   * @param dataType The data type for the group manager namespace.
   * @param database The GroupManagerDb for storing the group management
   * information (including user public keys and schedules).
   * @param keySize The group key will be an RSA key with keySize bits.
   * @param freshnessHours The number of hours of the freshness period of data
   *   packets carrying the keys.
   * @param keyChain The KeyChain to use for signing data packets. This signs
   * with the default identity. This is only a pointer to a KeyChain object
   * which must remain valid for the life of this GroupManager.
   */
  GroupManager
    (const Name& prefix, const Name& dataType,
     const ptr_lib::shared_ptr<GroupManagerDb>& database, uint32_t keySize,
     int freshnessHours, KeyChain* keyChain);

  /**
   * Create a group key for the interval into which timeSlot falls. This creates
   * a group key if it doesn't exist, and encrypts the key using the public key
   * of each eligible member.
   * @param timeSlot The time slot to cover as milliseconds since Jan 1, 1970 UTC.
   * @param result This clears result and sets it to a List of Data packets
   * where the first is the E-KEY data packet with the group's public key and
   * the rest are the D-KEY data packets with the group's private key encrypted
   * with the public key of each eligible member.
   * @param needRegenerate needRegenerate (optional) should be true if this is
   * the first time this method is called, or a member was removed.
   * needRegenerate can be false if this is not the first time this method is
   * called, or a member was added. If omitted, use true.
   * @throws GroupManagerDb::Error for a database error.
   * @throws SecurityException for an error using the security KeyChain.
   */
  void
  getGroupKey
    (MillisecondsSince1970 timeSlot,
     std::vector<ptr_lib::shared_ptr<Data> >& result,
     bool needRegenerate = true);

  /**
   * Add a schedule with the given scheduleName.
   * @param scheduleName The name of the schedule. The name cannot be empty.
   * @param schedule The Schedule to add.
   * @throws GroupManagerDb::Error if a schedule with the same name already
   * exists, if the name is empty, or other database error.
   */
  void
  addSchedule(const std::string& scheduleName, const Schedule& schedule)
  {
    database_->addSchedule(scheduleName, schedule);
  }

  /**
   * Delete the schedule with the given scheduleName. Also delete members which
   * use this schedule. If there is no schedule with the name, then do nothing.
   * @param scheduleName The name of the schedule.
   * @throws GroupManagerDb::Error for a database error.
   */
  void
  deleteSchedule(const std::string& scheduleName)
  {
    database_->deleteSchedule(scheduleName);
  }

  /**
   * Update the schedule with scheduleName and replace the old object with the
   * given schedule. Otherwise, if no schedule with name exists, a new schedule
   * with name and the given schedule will be added to database.
   * @param scheduleName The name of the schedule. The name cannot be empty.
   * @param schedule The Schedule to update or add.
   * @throws GroupManagerDb::Error if the name is empty, or other database error.
   */
  void
  updateSchedule(const std::string& scheduleName, const Schedule& schedule)
  {
    database_->updateSchedule(scheduleName, schedule);
  }

  /**
   * Add a new member with the given memberCertificate into a schedule named
   * scheduleName. If cert is an IdentityCertificate made from memberCertificate,
   * then the member's identity name is cert.getPublicKeyName().getPrefix(-1).
   * @param scheduleName The schedule name.
   * @param memberCertificate The member's certificate.
   * @throws GroupManagerDb::Error If there's no schedule named scheduleName, if
   * the member's identity name already exists, or other database error.
   * @throws DerDecodingException for error decoding memberCertificate as a
   * certificate.
   */
  void
  addMember(const std::string& scheduleName, const Data& memberCertificate)
  {
    IdentityCertificate cert(memberCertificate);
    database_->addMember
      (scheduleName, cert.getPublicKeyName(), cert.getPublicKeyInfo().getKeyDer());
  }

  /**
   * Remove a member with the given identity name. If there is no member with
   * the identity name, then do nothing.
   * @param identity The member's identity name.
   * @throws GroupManagerDb::Error for a database error.
   */
  void
  removeMember(const Name& identity)
  {
    database_->deleteMember(identity);
  }

  /**
   * Change the name of the schedule for the given member's identity name.
   * @param identity The member's identity name.
   * @param scheduleName The new schedule name.
   * @throws GroupManagerDb::Error if there's no member with the given identity
   * name in the database, or there's no schedule named scheduleName.
   */
  void
  updateMemberSchedule(const Name& identity, const std::string& scheduleName)
  {
    database_->updateMemberSchedule(identity, scheduleName);
  }

  /**
   * Delete all the EKeys in the database.
   * The database will keep growing because EKeys will keep being added, so this
   * method should be called periodically.
   * @throws GroupManagerDb::Error for a database error.
   */
  void
  cleanEKeys() { database_->cleanEKeys(); }

private:
  // Give friend access to the tests.
  friend class ::TestGroupManager_CreateDKeyData_Test;
  friend class ::TestGroupManager_CreateEKeyData_Test;
  friend class ::TestGroupManager_CalculateInterval_Test;

  /**
   * Calculate an Interval that covers the timeSlot.
   * @param timeSlot The time slot to cover as milliseconds since Jan 1, 1970 UTC.
   * @param memberKeys First clear memberKeys then fill it with the info of
   * members who are allowed to access the interval. The map's key is the Name
   * of the public key and the value is the Blob of the public key DER.
   * @return The Interval covering the time slot.
   * @throws GroupManagerDb::Error for a database error.
   */
  Interval
  calculateInterval
    (MillisecondsSince1970 timeSlot, std::map<Name, Blob>& memberKeys);

  /**
   * Generate an RSA key pair according to keySize_.
   * @param privateKeyBlob Set privateKeyBlob to the encoding Blob of the
   * private key.
   * @param publicKeyBlob Set publicKeyBlob to the encoding Blob of the
   * public key.
   */
  void
  generateKeyPair(Blob& privateKeyBlob, Blob& publicKeyBlob);

  /**
   * Create an E-KEY Data packet for the given public key.
   * @param startTimeStamp The start time stamp string to put in the name.
   * @param endTimeStamp The end time stamp string to put in the name.
   * @param publicKeyBlob A Blob of the public key DER.
   * @return The Data packet.
   * @throws SecurityException for an error using the security KeyChain.
   */
  ptr_lib::shared_ptr<Data>
  createEKeyData
    (const std::string& startTimeStamp, const std::string& endTimeStamp,
     const Blob& publicKeyBlob);

  /**
   * Create a D-KEY Data packet with an EncryptedContent for the given private
   * key, encrypted with the certificate key.
   * @param startTimeStamp The start time stamp string to put in the name.
   * @param endTimeStamp The end time stamp string to put in the name.
   * @param keyName The key name to put in the data packet name and the
   * EncryptedContent key locator.
   * @param privateKeyBlob A Blob of the encoded private key.
   * @param certificateKey The certificate key encoding, used to encrypt the
   * private key.
   * @return The Data packet.
   * @throws SecurityException for an error using the security KeyChain.
   */
  ptr_lib::shared_ptr<Data>
  createDKeyData
    (const std::string& startTimeStamp, const std::string& endTimeStamp,
     const Name& keyName, const Blob& privateKeyBlob, const Blob& certificateKey);

  /**
   * Add the EKey with name eKeyName to the database.
   * @param eKeyName The name of the EKey. This copies the Name.
   * @param publicKey The encoded public Key of the group key pair.
   * @param privateKey The encoded private Key of the group key pair.
   * @throws GroupManagerDb::Error If a key with name eKeyName already exists in
   * the database, or other database error.
   */
  void
  addEKey(const Name& eKeyName, const Blob& publicKey, const Blob& privateKey)
  {
    database_->addEKey(eKeyName, publicKey, privateKey);
  }

  /**
   * Get the group key pair with the name eKeyName from the database.
   * @param eKeyName The name of the EKey.
   * @param publicKey Set publicKey to the encoded public Key.
   * @param privateKey Set publicKey to the encoded private Key.
   * @throws GroupManagerDb::Error If the key with name eKeyName does not exist
   * in the database, or other database error.
   */
  void
  getEKey(const Name& eKeyName, Blob& publicKey, Blob& privateKey)
  {
    database_->getEKey(eKeyName, publicKey, privateKey);
  }

  /**
   * Delete the EKey with name eKeyName from the database. If no key with the
   * name exists in the database, do nothing.
   * @param eKeyName The name of the EKey.
   * @throws GroupManagerDb::Error for a database error.
   */
  void
  deleteEKey(const Name& eKeyName)
  {
    database_-> deleteEKey(eKeyName);
  }

  Name namespace_;
  ptr_lib::shared_ptr<GroupManagerDb> database_;
  uint32_t keySize_;
  int freshnessHours_;
  KeyChain* keyChain_;
  static const uint64_t MILLISECONDS_IN_HOUR = 3600 * 1000;
};

}

#endif
