/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/group-manager-db https://github.com/named-data/ndn-group-encrypt
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

#ifndef NDN_SQLITE3_GROUP_MANAGER_DB_HPP
#define NDN_SQLITE3_GROUP_MANAGER_DB_HPP

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_SQLITE3.
#include "../ndn-cpp-config.h"
#ifdef NDN_CPP_HAVE_SQLITE3

#include <map>
#include "../common.hpp"
#include "group-manager-db.hpp"

struct sqlite3;

namespace ndn {

/**
 * Sqlite3GroupManagerDb extends GroupManagerDb to implement the storage of
 * data used by the GroupManager using SQLite3.
 * @note This class is an experimental feature. The API may change.
 */
class Sqlite3GroupManagerDb : public GroupManagerDb {
public:
  /**
   * Create an Sqlite3GroupManagerDb to use the given SQLite3 file.
   * @param databaseFilePath The path of the SQLite file.
   */
  Sqlite3GroupManagerDb(const std::string& databaseFilePath);

  ////////////////////////////////////////////////////// Schedule management.

  /**
   * Check if there is a schedule with the given name.
   * @param name The name of the schedule.
   * @return True if there is a schedule.
   * @throws GroupManagerDb::Error for a database error.
   */
  virtual bool
  hasSchedule(const std::string& name);

  /**
   * Append all the names of the schedules to the nameList.
   * @param nameList Append result names to nameList. This first clears the list.
   * @throws GroupManagerDb::Error for a database error.
   */
  virtual void
  listAllScheduleNames(std::vector<std::string>& nameList);

  /**
   * Get a schedule with the given name.
   * @param name The name of the schedule.
   * @return A new Schedule object.
   * @throws GroupManagerDb::Error if the schedule does not exist or other
   * database error.
   */
  virtual ptr_lib::shared_ptr<Schedule>
  getSchedule(const std::string& name);

  /**
   * For each member using the given schedule, get the name and public key DER
   * of the member's key.
   * @param name The name of the schedule.
   * @param memberMap Put results in this map where the map's key is the Name of
   * the public key and the value is the Blob of the public key DER. This first
   * clears the map. Note that the member's identity name is
   * keyName.getPrefix(-1). If the schedule name is not found, the map is empty.
   * @throws GroupManagerDb::Error for a database error.
   */
  virtual void
  getScheduleMembers
    (const std::string& name, std::map<Name, Blob>& memberMap);

  /**
   * Add a schedule with the given name.
   * @param name The name of the schedule. The name cannot be empty.
   * @param schedule The Schedule to add.
   * @throws GroupManagerDb::Error if a schedule with the same name already exists,
   * if the name is empty, or other database error.
   */
  virtual void
  addSchedule(const std::string& name, const Schedule& schedule);

  /**
   * Delete the schedule with the given name. Also delete members which use this
   * schedule. If there is no schedule with the name, then do nothing.
   * @param name The name of the schedule.
   * @throws GroupManagerDb::Error for a database error.
   */
  virtual void
  deleteSchedule(const std::string& name);

  /**
   * Rename a schedule with oldName to newName.
   * @param oldName The name of the schedule to be renamed.
   * @param newName The new name of the schedule. The name cannot be empty.
   * @throws GroupManagerDb::Error If a schedule with newName already exists, if
   * the schedule with oldName does not exist, if newName is empty, or other
   * database error.
   */
  virtual void
  renameSchedule(const std::string& oldName, const std::string& newName);

  /**
   * Update the schedule with name and replace the old object with the given
   * schedule. Otherwise, if no schedule with name exists, a new schedule
   * with name and the given schedule will be added to database.
   * @param name The name of the schedule. The name cannot be empty.
   * @param schedule The Schedule to update or add.
   * @throws GroupManagerDb::Error if the name is empty, or other database error.
   */
  virtual void
  updateSchedule(const std::string& name, const Schedule& schedule);

  ////////////////////////////////////////////////////// Member management.

  /**
   * Check if there is a member with the given identity name.
   * @param identity The member's identity name.
   * @return True if there is a member.
   * @throws GroupManagerDb::Error for a database error.
   */
  virtual bool
  hasMember(const Name& identity);

  /**
   * Append all the members to the nameList.
   * @param nameList Append result names to nameList. This first clears the list.
   * @throws GroupManagerDb::Error for a database error.
   */
  virtual void
  listAllMembers(std::vector<Name>& nameList);

  /**
   * Get the name of the schedule for the given member's identity name.
   * @param identity The member's identity name.
   * @return The name of the schedule.
   * @throws GroupManagerDb::Error if there's no member with the given identity
   * name in the database, or other database error.
   */
  virtual std::string
  getMemberSchedule(const Name& identity);

  /**
   * Add a new member with the given key named keyName into a schedule named
   * scheduleName. The member's identity name is keyName.getPrefix(-1).
   * @param scheduleName The schedule name.
   * @param keyName The name of the key.
   * @param key A Blob of the public key DER.
   * @throws GroupManagerDb::Error If there's no schedule named scheduleName, if
   * the member's identity name already exists, or other database error.
   */
  virtual void
  addMember
    (const std::string& scheduleName, const Name& keyName, const Blob& key);

  /**
   * Change the name of the schedule for the given member's identity name.
   * @param identity The member's identity name.
   * @param scheduleName The new schedule name.
   * @throws GroupManagerDb::Error if there's no member with the given identity
   * name in the database, or there's no schedule named scheduleName, or other
   * database error.
   */
  virtual void
  updateMemberSchedule(const Name& identity, const std::string& scheduleName);

  /**
   * Delete a member with the given identity name. If there is no member with
   * the identity name, then do nothing.
   * @param identity The member's identity name.
   * @throws GroupManagerDb::Error for a database error.
   */
  virtual void
  deleteMember(const Name& identity);

  /**
   * Check if there is an EKey with the name eKeyName in the database.
   * @param eKeyName The name of the EKey.
   * @return True if the EKey exists.
   * @throws GroupManagerDb::Error for a database error.
   */
  virtual bool
  hasEKey(const Name& eKeyName);

  /**
   * Add the EKey with name eKeyName to the database.
   * @param eKeyName The name of the EKey. This copies the Name.
   * @param publicKey The encoded public Key of the group key pair.
   * @param privateKey The encoded private Key of the group key pair.
   * @throws GroupManagerDb::Error If a key with name eKeyName already exists in
   * the database, or other database error.
   */
  virtual void
  addEKey(const Name& eKeyName, const Blob& publicKey, const Blob& privateKey);

  /**
   * Get the group key pair with the name eKeyName from the database.
   * @param eKeyName The name of the EKey.
   * @param publicKey Set publicKey to the encoded public Key.
   * @param privateKey Set publicKey to the encoded private Key.
   * @throws GroupManagerDb::Error If the key with name eKeyName does not exist
   * in the database, or other database error.
   */
  virtual void
  getEKey(const Name& eKeyName, Blob& publicKey, Blob& privateKey);

  /**
   * Delete all the EKeys in the database.
   * The database will keep growing because EKeys will keep being added, so this
   * method should be called periodically.
   * @throws GroupManagerDb::Error for a database error.
   */
  virtual void
  cleanEKeys();

  /**
   * Delete the EKey with name eKeyName from the database. If no key with the
   * name exists in the database, do nothing.
   * @param eKeyName The name of the EKey.
   * @throws GroupManagerDb::Error for a database error.
   */
  virtual void
  deleteEKey(const Name& eKeyName);

private:
  /**
   * Get the ID for the schedule.
   * @param name The schedule name.
   * @return The ID, or -1 if the schedule name is not found.
   * @throws GroupManagerDb.Error for a database error.
   */
  int
  getScheduleId(const std::string& name);

  struct sqlite3 *database_;
  std::map<Name, Blob> privateKeyBase_;
};

}

#endif // NDN_CPP_HAVE_SQLITE3

#endif
