/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/pib/pib-sqlite3.cpp
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

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_SQLITE3.
#include <ndn-cpp/ndn-cpp-config.h>
#ifdef NDN_CPP_HAVE_SQLITE3

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "../../util/sqlite3-statement.hpp"
#include <ndn-cpp/security/pib/pib-sqlite3.hpp>

using namespace std;

namespace ndn {

static const string INITIALIZATION = "\
CREATE TABLE IF NOT EXISTS                         \n\
  tpmInfo(                                         \n\
    tpm_locator           BLOB                     \n\
  );                                               \n\
                                                   \n\
CREATE TABLE IF NOT EXISTS                         \n\
  identities(                                      \n\
    id                    INTEGER PRIMARY KEY,     \n\
    identity              BLOB NOT NULL,           \n\
    is_default            INTEGER DEFAULT 0        \n\
  );                                               \n\
                                                   \n\
CREATE UNIQUE INDEX IF NOT EXISTS                  \n\
  identityIndex ON identities(identity);           \n\
                                                   \n\
CREATE TRIGGER IF NOT EXISTS                       \n\
  identity_default_before_insert_trigger           \n\
  BEFORE INSERT ON identities                      \n\
  FOR EACH ROW                                     \n\
  WHEN NEW.is_default=1                            \n\
  BEGIN                                            \n\
    UPDATE identities SET is_default=0;            \n\
  END;                                             \n\
                                                   \n\
CREATE TRIGGER IF NOT EXISTS                       \n\
  identity_default_after_insert_trigger            \n\
  AFTER INSERT ON identities                       \n\
  FOR EACH ROW                                     \n\
  WHEN NOT EXISTS                                  \n\
    (SELECT id                                     \n\
       FROM identities                             \n\
       WHERE is_default=1)                         \n\
  BEGIN                                            \n\
    UPDATE identities                              \n\
      SET is_default=1                             \n\
      WHERE identity=NEW.identity;                 \n\
  END;                                             \n\
                                                   \n\
CREATE TRIGGER IF NOT EXISTS                       \n\
  identity_default_update_trigger                  \n\
  BEFORE UPDATE ON identities                      \n\
  FOR EACH ROW                                     \n\
  WHEN NEW.is_default=1 AND OLD.is_default=0       \n\
  BEGIN                                            \n\
    UPDATE identities SET is_default=0;            \n\
  END;                                             \n\
                                                   \n\
CREATE TABLE IF NOT EXISTS                         \n\
  keys(                                            \n\
    id                    INTEGER PRIMARY KEY,     \n\
    identity_id           INTEGER NOT NULL,        \n\
    key_name              BLOB NOT NULL,           \n\
    key_bits              BLOB NOT NULL,           \n\
    is_default            INTEGER DEFAULT 0,       \n\
    FOREIGN KEY(identity_id)                       \n\
      REFERENCES identities(id)                    \n\
      ON DELETE CASCADE                            \n\
      ON UPDATE CASCADE                            \n\
  );                                               \n\
                                                   \n\
CREATE UNIQUE INDEX IF NOT EXISTS                  \n\
  keyIndex ON keys(key_name);                      \n\
                                                   \n\
CREATE TRIGGER IF NOT EXISTS                       \n\
  key_default_before_insert_trigger                \n\
  BEFORE INSERT ON keys                            \n\
  FOR EACH ROW                                     \n\
  WHEN NEW.is_default=1                            \n\
  BEGIN                                            \n\
    UPDATE keys                                    \n\
      SET is_default=0                             \n\
      WHERE identity_id=NEW.identity_id;           \n\
  END;                                             \n\
                                                   \n\
CREATE TRIGGER IF NOT EXISTS                       \n\
  key_default_after_insert_trigger                 \n\
  AFTER INSERT ON keys                             \n\
  FOR EACH ROW                                     \n\
  WHEN NOT EXISTS                                  \n\
    (SELECT id                                     \n\
       FROM keys                                   \n\
       WHERE is_default=1                          \n\
         AND identity_id=NEW.identity_id)          \n\
  BEGIN                                            \n\
    UPDATE keys                                    \n\
      SET is_default=1                             \n\
      WHERE key_name=NEW.key_name;                 \n\
  END;                                             \n\
                                                   \n\
CREATE TRIGGER IF NOT EXISTS                       \n\
  key_default_update_trigger                       \n\
  BEFORE UPDATE ON keys                            \n\
  FOR EACH ROW                                     \n\
  WHEN NEW.is_default=1 AND OLD.is_default=0       \n\
  BEGIN                                            \n\
    UPDATE keys                                    \n\
      SET is_default=0                             \n\
      WHERE identity_id=NEW.identity_id;           \n\
  END;                                             \n\
                                                   \n\
CREATE TABLE IF NOT EXISTS                         \n\
  certificates(                                    \n\
    id                    INTEGER PRIMARY KEY,     \n\
    key_id                INTEGER NOT NULL,        \n\
    certificate_name      BLOB NOT NULL,           \n\
    certificate_data      BLOB NOT NULL,           \n\
    is_default            INTEGER DEFAULT 0,       \n\
    FOREIGN KEY(key_id)                            \n\
      REFERENCES keys(id)                          \n\
      ON DELETE CASCADE                            \n\
      ON UPDATE CASCADE                            \n\
  );                                               \n\
                                                   \n\
CREATE UNIQUE INDEX IF NOT EXISTS                  \n\
  certIndex ON certificates(certificate_name);     \n\
                                                   \n\
CREATE TRIGGER IF NOT EXISTS                       \n\
  cert_default_before_insert_trigger               \n\
  BEFORE INSERT ON certificates                    \n\
  FOR EACH ROW                                     \n\
  WHEN NEW.is_default=1                            \n\
  BEGIN                                            \n\
    UPDATE certificates                            \n\
      SET is_default=0                             \n\
      WHERE key_id=NEW.key_id;                     \n\
  END;                                             \n\
                                                   \n\
CREATE TRIGGER IF NOT EXISTS                       \n\
  cert_default_after_insert_trigger                \n\
  AFTER INSERT ON certificates                     \n\
  FOR EACH ROW                                     \n\
  WHEN NOT EXISTS                                  \n\
    (SELECT id                                     \n\
       FROM certificates                           \n\
       WHERE is_default=1                          \n\
         AND key_id=NEW.key_id)                    \n\
  BEGIN                                            \n\
    UPDATE certificates                            \n\
      SET is_default=1                             \n\
      WHERE certificate_name=NEW.certificate_name; \n\
  END;                                             \n\
                                                   \n\
CREATE TRIGGER IF NOT EXISTS                       \n\
  cert_default_update_trigger                      \n\
  BEFORE UPDATE ON certificates                    \n\
  FOR EACH ROW                                     \n\
  WHEN NEW.is_default=1 AND OLD.is_default=0       \n\
  BEGIN                                            \n\
    UPDATE certificates                            \n\
      SET is_default=0                             \n\
      WHERE key_id=NEW.key_id;                     \n\
  END;                                             \n\
";

PibSqlite3::PibSqlite3
  (const string& databaseDirectoryPathIn, const string& databaseFilename)
{
  string databaseDirectoryPath;
  if (databaseDirectoryPathIn != "") {
    databaseDirectoryPath = databaseDirectoryPathIn;
    if (databaseDirectoryPath[databaseDirectoryPath.size() - 1] == '/' ||
        databaseDirectoryPath[databaseDirectoryPath.size() - 1] == '\\')
      // Strip the ending path separator.
      databaseDirectoryPath.erase(databaseDirectoryPath.size() - 1);
  }
  else
    databaseDirectoryPath = getDefaultDatabaseDirectoryPath();

  // TODO: Handle non-unix file systems which don't have "mkdir -p".
  ::system(("mkdir -p \"" + databaseDirectoryPath + "\"").c_str());

  // Open the PIB.
  string databaseFilePath = databaseDirectoryPath + '/' + databaseFilename;
  int result = sqlite3_open(databaseFilePath.c_str(), &database_);

  if (result != SQLITE_OK)
    throw PibImpl::Error
      ("PIB database cannot be opened/created: " + databaseFilePath);

  // Enable foreign keys.
  sqlite3_exec(database_, "PRAGMA foreign_keys=ON", NULL, NULL, NULL);

  // Initialize the PIB tables.
  char* errorMessage = 0;
  result = sqlite3_exec
    (database_, INITIALIZATION.c_str(), NULL, NULL, &errorMessage);
  if (result != SQLITE_OK && errorMessage != 0) {
    sqlite3_free(errorMessage);
    throw PibImpl::Error("PIB database cannot be initialized");
  }
}

PibSqlite3::~PibSqlite3()
{
  sqlite3_close(database_);
}

string
PibSqlite3::getScheme() { return NDN_PIB_SQLITE3_SCHEME; }

void
PibSqlite3::setTpmLocator(const string& tpmLocator)
{
  Sqlite3Statement statement(database_, "UPDATE tpmInfo SET tpm_locator=?");
  statement.bind(1, tpmLocator);
  statement.step();

  if (sqlite3_changes(database_) == 0) {
    // No row was updated, so tpmLocator does not exist. Insert it directly.
    Sqlite3Statement insertStatement
      (database_, "INSERT INTO tpmInfo (tpm_locator) values (?)");
    insertStatement.bind(1, tpmLocator);
    insertStatement.step();
  }
}

string
PibSqlite3::getTpmLocator() const
{
  Sqlite3Statement statement(database_, "SELECT tpm_locator FROM tpmInfo");
  int res = statement.step();
  if (res == SQLITE_ROW)
    return statement.getString(0);
  else
    return "";
}

bool
PibSqlite3::hasIdentity(const Name& identityName) const
{
  Sqlite3Statement statement
    (database_, "SELECT id FROM identities WHERE identity=?");
  statement.bind(1, identityName.wireEncode());
  return (statement.step() == SQLITE_ROW);
}

void
PibSqlite3::addIdentity(const Name& identityName)
{
  if (!hasIdentity(identityName)) {
    Sqlite3Statement statement
      (database_, "INSERT INTO identities (identity) values (?)");
    statement.bind(1, identityName.wireEncode());
    statement.step();
  }

  if (!hasDefaultIdentity())
    setDefaultIdentity(identityName);
}

void
PibSqlite3::removeIdentity(const Name& identityName)
{
  Sqlite3Statement statement(database_, "DELETE FROM identities WHERE identity=?");
  statement.bind(1, identityName.wireEncode());
  statement.step();
}

void
PibSqlite3::clearIdentities()
{
  Sqlite3Statement statement(database_, "DELETE FROM identities");
  statement.step();
}

set<Name>
PibSqlite3::getIdentities() const
{
  set<Name> identities;
  Sqlite3Statement statement(database_, "SELECT identity FROM identities");

  while (statement.step() == SQLITE_ROW) {
    Name name;
    name.wireDecode(statement.getBuf(0), statement.getSize(0));
    identities.insert(name);
  }

  return identities;
}

void
PibSqlite3::setDefaultIdentity(const Name& identityName)
{
  if (!hasIdentity(identityName)) {
    Sqlite3Statement statement
      (database_, "INSERT INTO identities (identity) values (?)");
    statement.bind(1, identityName.wireEncode());
    statement.step();
  }

  Sqlite3Statement statement
    (database_, "UPDATE identities SET is_default=1 WHERE identity=?");
  statement.bind(1, identityName.wireEncode());
  statement.step();
}

Name
PibSqlite3::getDefaultIdentity() const
{
  Sqlite3Statement statement
    (database_, "SELECT identity FROM identities WHERE is_default=1");

  if (statement.step() == SQLITE_ROW) {
    Name name;
    name.wireDecode(statement.getBuf(0), statement.getSize(0));
    return name;
  }
  else
    throw Pib::Error("No default identity");
}

bool
PibSqlite3::hasKey(const Name& keyName) const
{
  Sqlite3Statement statement(database_, "SELECT id FROM keys WHERE key_name=?");
  statement.bind(1, keyName.wireEncode());

  return (statement.step() == SQLITE_ROW);
}

void
PibSqlite3::addKey
  (const Name& identityName, const Name& keyName, const uint8_t* key,
   size_t keyLength)
{
  // Ensure the identity exists.
  addIdentity(identityName);

  if (!hasKey(keyName)) {
    Sqlite3Statement statement(database_,
"INSERT INTO keys (identity_id, key_name, key_bits) \
VALUES ((SELECT id FROM identities WHERE identity=?), ?, ?)");
    statement.bind(1, identityName.wireEncode());
    statement.bind(2, keyName.wireEncode());
    statement.bind(3, key, keyLength, SQLITE_STATIC);
    statement.step();
  }
  else {
    Sqlite3Statement statement
      (database_, "UPDATE keys SET key_bits=? WHERE key_name=?");
    statement.bind(1, key, keyLength, SQLITE_STATIC);
    statement.bind(2, keyName.wireEncode());
    statement.step();
  }

  if (!hasDefaultKeyOfIdentity(identityName))
    setDefaultKeyOfIdentity(identityName, keyName);
}

void
PibSqlite3::removeKey(const Name& keyName)
{
  Sqlite3Statement statement(database_, "DELETE FROM keys WHERE key_name=?");
  statement.bind(1, keyName.wireEncode());
  statement.step();
}

Blob
PibSqlite3::getKeyBits(const Name& keyName) const
{
  Sqlite3Statement statement
    (database_, "SELECT key_bits FROM keys WHERE key_name=?");
  statement.bind(1, keyName.wireEncode());

  if (statement.step() == SQLITE_ROW)
    return statement.getBlob(0);
  else
    throw Pib::Error("Key `" + keyName.toUri() + "` does not exist");
}

set<Name>
PibSqlite3::getKeysOfIdentity(const Name& identityName) const
{
  set<Name> keyNames;

  Sqlite3Statement statement(database_,
"SELECT key_name \
FROM keys JOIN identities ON keys.identity_id=identities.id \
WHERE identities.identity=?");
  statement.bind(1, identityName.wireEncode());

  while (statement.step() == SQLITE_ROW) {
    Name name;
    name.wireDecode(statement.getBuf(0), statement.getSize(0));
    keyNames.insert(name);
  }

  return keyNames;
}

void
PibSqlite3::setDefaultKeyOfIdentity
  (const Name& identityName, const Name& keyName)
{
  if (!hasKey(keyName))
    throw Pib::Error("Key `" + keyName.toUri() + "` does not exist");

  Sqlite3Statement statement
    (database_, "UPDATE keys SET is_default=1 WHERE key_name=?");
  statement.bind(1, keyName.wireEncode());
  statement.step();
}

Name
PibSqlite3::getDefaultKeyOfIdentity(const Name& identityName) const
{
  if (!hasIdentity(identityName))
    throw Pib::Error("Identity `" + identityName.toUri() + "` does not exist");

  Sqlite3Statement statement(database_,
"SELECT key_name \
FROM keys JOIN identities ON keys.identity_id=identities.id \
WHERE identities.identity=? AND keys.is_default=1");
  statement.bind(1, identityName.wireEncode());

  if (statement.step() == SQLITE_ROW) {
    Name name;
    name.wireDecode(statement.getBuf(0), statement.getSize(0));
    return name;
  }
  else
    throw Pib::Error("No default key for identity `" + identityName.toUri() + "`");
}

bool
PibSqlite3::hasCertificate(const Name& certificateName) const
{
  Sqlite3Statement statement
    (database_, "SELECT id FROM certificates WHERE certificate_name=?");
  statement.bind(1, certificateName.wireEncode());
  return (statement.step() == SQLITE_ROW);
}

void
PibSqlite3::addCertificate(const CertificateV2& certificate)
{
  // Ensure the key exists.
  const Blob& content = certificate.getContent();
  addKey
    (certificate.getIdentity(), certificate.getKeyName(),
     content.buf(), content.size());

  if (!hasCertificate(certificate.getName())) {
    Sqlite3Statement statement(database_,
"INSERT INTO certificates \
(key_id, certificate_name, certificate_data) \
VALUES ((SELECT id FROM keys WHERE key_name=?), ?, ?)");
    statement.bind(1, certificate.getKeyName().wireEncode());
    statement.bind(2, certificate.getName().wireEncode());
    // The wire encoding Blob stays in the certificate, so it is static.
    statement.bind(3, certificate.wireEncode(), true);
    statement.step();
  }
  else {
    Sqlite3Statement statement
      (database_,
       "UPDATE certificates SET certificate_data=? WHERE certificate_name=?");
    statement.bind(1, certificate.wireEncode(), true);
    statement.bind(2, certificate.getName().wireEncode());
    statement.step();
  }

  if (!hasDefaultCertificateOfKey(certificate.getKeyName()))
    setDefaultCertificateOfKey(certificate.getKeyName(), certificate.getName());
}

void
PibSqlite3::removeCertificate(const Name& certificateName)
{
  Sqlite3Statement statement
    (database_, "DELETE FROM certificates WHERE certificate_name=?");
  statement.bind(1, certificateName.wireEncode());
  statement.step();
}

ptr_lib::shared_ptr<CertificateV2>
PibSqlite3::getCertificate(const Name& certificateName) const
{
  Sqlite3Statement statement
    (database_,
     "SELECT certificate_data FROM certificates WHERE certificate_name=?");
  statement.bind(1, certificateName.wireEncode());

  if (statement.step() == SQLITE_ROW) {
    ptr_lib::shared_ptr<CertificateV2> certificate(new CertificateV2());
    certificate->wireDecode(statement.getBuf(0), statement.getSize(0));
    return certificate;
  }
  else
    throw Pib::Error("Certificate `" + certificateName.toUri() + "` does not exit");
}

set<Name>
PibSqlite3::getCertificatesOfKey(const Name& keyName) const
{
  set<Name> certNames;

  Sqlite3Statement statement(database_,
"SELECT certificate_name \
FROM certificates JOIN keys ON certificates.key_id=keys.id \
WHERE keys.key_name=?");
  statement.bind(1, keyName.wireEncode());

  while (statement.step() == SQLITE_ROW) {
    Name name;
    name.wireDecode(statement.getBuf(0), statement.getSize(0));
    certNames.insert(name);
  }

  return certNames;
}

void
PibSqlite3::setDefaultCertificateOfKey
  (const Name& keyName, const Name& certificateName)
{
  if (!hasCertificate(certificateName))
    throw Pib::Error("Certificate `" + certificateName.toUri() + "` does not exist");

  Sqlite3Statement statement
    (database_, "UPDATE certificates SET is_default=1 WHERE certificate_name=?");
  statement.bind(1, certificateName.wireEncode());
  statement.step();
}

ptr_lib::shared_ptr<CertificateV2>
PibSqlite3::getDefaultCertificateOfKey(const Name& keyName) const
{
  Sqlite3Statement statement(database_,
"SELECT certificate_data \
FROM certificates JOIN keys ON certificates.key_id=keys.id \
WHERE certificates.is_default=1 AND keys.key_name=?");
  statement.bind(1, keyName.wireEncode());

  if (statement.step() == SQLITE_ROW) {
    ptr_lib::shared_ptr<CertificateV2> certificate(new CertificateV2());
    certificate->wireDecode(statement.getBuf(0), statement.getSize(0));
    return certificate;
  }
  else
    throw Pib::Error("No default certificate for key `" + keyName.toUri() + "`");
}

string
PibSqlite3::getDefaultDatabaseDirectoryPath()
{
  // Note: We don't use <filesystem> support because it is not "header-only"
  // and requires linking to libraries.
  const char* home = getenv("HOME");
  if (!home || *home == '\0')
    // Don't expect this to happen;
    home = ".";
  string homeDir(home);
  if (homeDir[homeDir.size() - 1] == '/' || homeDir[homeDir.size() - 1] == '\\')
    // Strip the ending path separator.
    homeDir.erase(homeDir.size() - 1);

  // TODO: Handle non-unix file systems which don't use "/".
  return homeDir + '/' + ".ndn";
}

bool
PibSqlite3::hasDefaultIdentity() const
{
  Sqlite3Statement statement
    (database_, "SELECT identity FROM identities WHERE is_default=1");
  return (statement.step() == SQLITE_ROW);
}

bool
PibSqlite3::hasDefaultKeyOfIdentity(const Name& identityName) const
{
  Sqlite3Statement statement(database_,
"SELECT key_name \
FROM keys JOIN identities ON keys.identity_id=identities.id \
WHERE identities.identity=? AND keys.is_default=1");
  Blob encoding = identityName.wireEncode();
  statement.bind(1, encoding);

  return (statement.step() == SQLITE_ROW);
}

bool
PibSqlite3::hasDefaultCertificateOfKey(const Name& keyName) const
{
  Sqlite3Statement statement(database_,
"SELECT certificate_data \
FROM certificates JOIN keys ON certificates.key_id=keys.id \
WHERE certificates.is_default=1 AND keys.key_name=?");
  Blob encoding = keyName.wireEncode();
  statement.bind(1, encoding);

  return (statement.step() == SQLITE_ROW);
}

}

#endif // NDN_CPP_HAVE_SQLITE3
