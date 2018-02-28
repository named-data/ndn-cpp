/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/tpm/back-end-osx.cpp
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

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_OSX_SECURITY 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_OSX_SECURITY

#include <stdexcept>
#include <ndn-cpp/security/key-params.hpp>
#include <ndn-cpp/security/tpm/tpm.hpp>
#include <ndn-cpp/security/tpm/tpm-private-key.hpp>
#include <ndn-cpp/security/tpm/tpm-key-handle-osx.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end-osx.hpp>

using namespace std;

namespace ndn {

static CFTypeRef
getAsymKeyType(KeyType keyType)
{
  switch (keyType) {
  case KEY_TYPE_RSA:
    return kSecAttrKeyTypeRSA;
  case KEY_TYPE_EC:
    return kSecAttrKeyTypeECDSA;
  default:
    throw Tpm::Error("Unsupported key type");
  }
}

static CFTypeRef
getDigestAlgorithm(DigestAlgorithm digestAlgorithm)
{
  switch (digestAlgorithm) {
  case DIGEST_ALGORITHM_SHA256:
    return kSecDigestSHA2;
  default:
    return 0;
  }
}

static long
getDigestSize(DigestAlgorithm digestAlgorithm)
{
  switch (digestAlgorithm) {
  case DIGEST_ALGORITHM_SHA256:
    return 256;
  default:
    return -1;
  }
}

TpmBackEndOsx::TpmBackEndOsx(const std::string& location)
: isTerminalMode_(false)
{
  SecKeychainSetUserInteractionAllowed(!isTerminalMode_);

  OSStatus res = SecKeychainCopyDefault(&keyChainRef_);

  if (res == errSecNoDefaultKeychain)
    throw Error("There is no default maxOS Keychain. Create one first");
}

bool
TpmBackEndOsx::isTerminalMode() const { return isTerminalMode_; }

void
TpmBackEndOsx::setTerminalMode(bool isTerminal) const
{
  const_cast<TpmBackEndOsx*>(this)->isTerminalMode_ = isTerminal;
  SecKeychainSetUserInteractionAllowed(!isTerminal);
}

bool
TpmBackEndOsx::isTpmLocked() const
{
  SecKeychainStatus keychainStatus;

  OSStatus res = SecKeychainGetStatus(keyChainRef_, &keychainStatus);
  if (res != errSecSuccess)
    return true;
  else
    return ((kSecUnlockStateStatus & keychainStatus) == 0);
}

bool
TpmBackEndOsx::unlockTpm(const uint8_t* password, size_t passwordLength) const
{
  // If the default key chain is already unlocked, return immediately.
  if (!isTpmLocked())
    return true;

  if (isTerminalMode_)
    // Use the supplied password.
    SecKeychainUnlock(keyChainRef_, passwordLength, password, true);
  else
    // Get the password from the GUI.
    SecKeychainUnlock(keyChainRef_, 0, nullptr, false);

  return !isTpmLocked();
}

Blob
TpmBackEndOsx::sign
  (const KeyRefOsx& key, DigestAlgorithm digestAlgorithm,
   const uint8_t* data, size_t dataLength)
{
  CFReleaser<CFDataRef> dataRef = CFDataCreateWithBytesNoCopy
    (nullptr, data, dataLength, kCFAllocatorNull);

  CFReleaser<CFErrorRef> error;
  // C-style cast is used as per Apple convention.
  CFReleaser<SecTransformRef> signer = SecSignTransformCreate
    (key.get(), &error.get());
  if (error.get() != 0)
    throw Error("Failed to create the signer");

  // Set the input.
  SecTransformSetAttribute
    (signer.get(), kSecTransformInputAttributeName, dataRef.get(), &error.get());
  if (error.get() != 0)
    throw Error("Failed to configure the input of the signer");
  
  // Set the padding type.
  SecTransformSetAttribute
    (signer.get(), kSecPaddingKey, kSecPaddingPKCS1Key, &error.get());
  if (error.get() != 0)
    throw Error("Failed to configure the padding algorithm of the signer");

  // Set the digest attribute.
  SecTransformSetAttribute
    (signer.get(), kSecDigestTypeAttribute, getDigestAlgorithm(digestAlgorithm),
     &error.get());
  if (error.get() != 0)
    throw Error("Failed to configure the digest algorithm of the signer");

  // Set the digest size attribute.
  long digestSize = getDigestSize(digestAlgorithm);
  CFReleaser<CFNumberRef> cfDigestSize = CFNumberCreate
    (nullptr, kCFNumberLongType, &digestSize);
  SecTransformSetAttribute
    (signer.get(), kSecDigestLengthAttribute, cfDigestSize.get(), &error.get());
  if (error.get() != 0)
    throw Error("Failed to configure the digest size of the signer");

  // Actually sign.
  // C-style cast is used as per Apple convention.
  CFReleaser<CFDataRef> signature = (CFDataRef)SecTransformExecute
    (signer.get(), &error.get());
  if (error.get() != 0) {
    CFShow(error.get());
    throw Error("Failed to sign the data");
  }

  if (signature.get() == 0)
    throw Error("The signature is NULL");

  return Blob(CFDataGetBytePtr(signature.get()), CFDataGetLength(signature.get()));
}

Blob
TpmBackEndOsx::decrypt
  (const KeyRefOsx& key, const uint8_t* cipherText, size_t cipherTextLength)
{
  CFReleaser<CFDataRef> dataRef = CFDataCreateWithBytesNoCopy
    (nullptr, cipherText, cipherTextLength, kCFAllocatorNull);

  CFReleaser<CFErrorRef> error;
  CFReleaser<SecTransformRef> decryptor = SecDecryptTransformCreate
    (key.get(), &error.get());
  if (error.get() != 0)
    throw Error("Failed to create the decryptor");

  SecTransformSetAttribute
    (decryptor.get(), kSecTransformInputAttributeName, dataRef.get(), &error.get());
  if (error.get() != 0)
    throw Error("Failed to configure the decryptor");

  SecTransformSetAttribute
    (decryptor.get(), kSecPaddingKey, kSecPaddingOAEPKey, &error.get());
  if (error.get() != 0)
    throw Error("Failed to configure the decryptor #2");

  CFReleaser<CFDataRef> output = (CFDataRef)SecTransformExecute
    (decryptor.get(), &error.get());
  if (error.get() != 0)
    throw Error("Failed to decrypt the cipherText");

  if (output.get() == 0)
    throw Error("The output is NULL");

  return Blob(CFDataGetBytePtr(output.get()), CFDataGetLength(output.get()));
}

Blob
TpmBackEndOsx::derivePublicKey(const KeyRefOsx& key)
{
  CFReleaser<CFDataRef> exportedKey;
  OSStatus res = SecItemExport(key.get(),           // secItemOrArray
                               kSecFormatOpenSSL,   // outputFormat
                               0,                   // flags
                               nullptr,             // keyParams
                               &exportedKey.get()); // exportedData

  if (res != errSecSuccess) {
    if (res == errSecAuthFailed)
      throw Error("Failed to unlock the keychain");
    else
      throw Error("Failed to export the private key");
  }

  TpmPrivateKey privateKey;
  privateKey.loadPkcs1
    (CFDataGetBytePtr(exportedKey.get()), CFDataGetLength(exportedKey.get()));
  return privateKey.derivePublicKey();
}

bool
TpmBackEndOsx::doHasKey(const Name& keyName) const
{
  CFReleaser<CFStringRef> keyLabel = CFStringCreateWithCString
    (nullptr, keyName.toUri().c_str(), kCFStringEncodingUTF8);

  CFReleaser<CFMutableDictionaryRef> attrDict =
    CFDictionaryCreateMutable(nullptr, 4, &kCFTypeDictionaryKeyCallBacks, nullptr);

  CFDictionaryAddValue(attrDict.get(), kSecClass, kSecClassKey);
  CFDictionaryAddValue(attrDict.get(), kSecAttrLabel, keyLabel.get());
  CFDictionaryAddValue(attrDict.get(), kSecReturnRef, kCFBooleanTrue);

  CFReleaser<SecKeychainItemRef> itemRef;
  // C-style cast is used as per Apple convention.
  OSStatus res = SecItemCopyMatching
    ((CFDictionaryRef)attrDict.get(), (CFTypeRef*)&itemRef.get());
  itemRef.retain();

  return (res == errSecSuccess);
}

ptr_lib::shared_ptr<TpmKeyHandle>
TpmBackEndOsx::doGetKeyHandle(const Name& keyName) const
{
  CFReleaser<SecKeychainItemRef> keyItem;
  try {
    keyItem = getKey(keyName);
  }
  catch (const domain_error&) {
    return ptr_lib::shared_ptr<TpmKeyHandle>();
  }

  return ptr_lib::make_shared<TpmKeyHandleOsx>((SecKeyRef)keyItem.get());
}

ptr_lib::shared_ptr<TpmKeyHandle>
TpmBackEndOsx::doCreateKey(const Name& identityName, const KeyParams& params)
{
  KeyType keyType = params.getKeyType();
  uint32_t keySize;
  switch (keyType) {
    case KEY_TYPE_RSA: {
      const RsaKeyParams& rsaParams = static_cast<const RsaKeyParams&>(params);
      keySize = rsaParams.getKeySize();
      break;
    }
    case KEY_TYPE_EC: {
      const EcKeyParams& ecParams = static_cast<const EcKeyParams&>(params);
      keySize = ecParams.getKeySize();
      break;
    }
    default: {
      throw Tpm::Error("Failed to create a key pair: Unsupported key type");
    }
  }
  CFReleaser<CFNumberRef> cfKeySize = CFNumberCreate
    (nullptr, kCFNumberIntType, &keySize);

  CFReleaser<CFMutableDictionaryRef> attrDict =
    CFDictionaryCreateMutable(nullptr, 2, &kCFTypeDictionaryKeyCallBacks, nullptr);
  CFDictionaryAddValue(attrDict.get(), kSecAttrKeyType, getAsymKeyType(keyType));
  CFDictionaryAddValue(attrDict.get(), kSecAttrKeySizeInBits, cfKeySize.get());

  KeyRefOsx publicKey, privateKey;
  // C-style cast is used as per Apple convention.
  OSStatus res = SecKeyGeneratePair
    ((CFDictionaryRef)attrDict.get(), &publicKey.get(), &privateKey.get());

  // BOOST_ASSERT(privateKey != nullptr);

  publicKey.retain();
  privateKey.retain();

  // BOOST_ASSERT(privateKey != nullptr);

  if (res != errSecSuccess) {
    if (res == errSecAuthFailed)
      throw Error("Fail to unlock the keychain");
    else
      throw Error("Fail to create a key pair");
  }

  ptr_lib::shared_ptr<TpmKeyHandle> keyHandle =
    ptr_lib::make_shared<TpmKeyHandleOsx>(privateKey.get());
  setKeyName(*keyHandle, identityName, params);

  SecKeychainAttribute attrs[1]; // maximum number of attributes
  SecKeychainAttributeList attrList = { 0, attrs };
  std::string keyUri = keyHandle->getKeyName().toUri();
  {
    attrs[attrList.count].tag = kSecKeyPrintName;
    attrs[attrList.count].length = keyUri.size();
    attrs[attrList.count].data = const_cast<char*>(keyUri.data());
    attrList.count++;
  }

  SecKeychainItemModifyAttributesAndData
    ((SecKeychainItemRef)privateKey.get(), &attrList, 0, nullptr);
  SecKeychainItemModifyAttributesAndData
    ((SecKeychainItemRef)publicKey.get(), &attrList, 0, nullptr);

  return keyHandle;
}

void
TpmBackEndOsx::doDeleteKey(const Name& keyName)
{
  CFReleaser<CFStringRef> keyLabel = CFStringCreateWithCString
    (nullptr, keyName.toUri().c_str(), kCFStringEncodingUTF8);

  CFReleaser<CFMutableDictionaryRef> searchDict = CFDictionaryCreateMutable
    (nullptr, 5, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

  CFDictionaryAddValue(searchDict.get(), kSecClass, kSecClassKey);
  CFDictionaryAddValue(searchDict.get(), kSecAttrLabel, keyLabel.get());
  CFDictionaryAddValue(searchDict.get(), kSecMatchLimit, kSecMatchLimitAll);
  OSStatus res = SecItemDelete(searchDict.get());

  if (res != errSecSuccess) {
    if (res == errSecAuthFailed)
      throw Error("Fail to unlock the keychain");
    else if (res != errSecItemNotFound)
      throw Error("Fail to delete a key pair");
  }
}

CFReleaser<SecKeychainItemRef>
TpmBackEndOsx::getKey(const Name& keyName)
{
  CFReleaser<CFStringRef> keyLabel = CFStringCreateWithCString
    (nullptr, keyName.toUri().c_str(), kCFStringEncodingUTF8);

  CFReleaser<CFMutableDictionaryRef> attrDict =
    CFDictionaryCreateMutable(nullptr, 5, &kCFTypeDictionaryKeyCallBacks, nullptr);

  CFDictionaryAddValue(attrDict.get(), kSecClass, kSecClassKey);
  CFDictionaryAddValue(attrDict.get(), kSecAttrLabel, keyLabel.get());
  CFDictionaryAddValue(attrDict.get(), kSecAttrKeyClass, kSecAttrKeyClassPrivate);
  CFDictionaryAddValue(attrDict.get(), kSecReturnRef, kCFBooleanTrue);

  CFReleaser<SecKeychainItemRef> keyItem;
  // C-style cast is used as per Apple convention.
  OSStatus res = SecItemCopyMatching
    ((CFDictionaryRef)attrDict.get(), (CFTypeRef*)&keyItem.get());
  keyItem.retain();

  if (res != errSecSuccess) {
    if (res == errSecAuthFailed)
      throw Error("Fail to unlock the keychain");

    throw domain_error("Key does not exist");
  }

  return keyItem;
}

}

#endif // NDN_CPP_HAVE_OSX_SECURITY
