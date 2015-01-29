/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Yingdi Yu <yingdi@cs.ucla.edu>
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

#include <fstream>
#include <sstream>
#include <CoreFoundation/CoreFoundation.h>

#include "../../util/logging.hpp"
#include <ndn-cpp/security/identity/osx-private-key-storage.hpp>
#include <ndn-cpp/security/security-exception.hpp>

using namespace std;

INIT_LOGGER("ndn.OSXPrivateKeyStorage");

namespace ndn
{
  OSXPrivateKeyStorage::~OSXPrivateKeyStorage()
  {
  }

  void
  OSXPrivateKeyStorage::generateKeyPair
    (const Name & keyName, const KeyParams& params)
  {

    if (doesKeyExist(keyName, KEY_CLASS_PUBLIC))
      throw SecurityException("keyName already exists");

    string keyNameUri = toInternalKeyName(keyName, KEY_CLASS_PUBLIC);

    CFReleaser<CFStringRef> keyLabel = CFStringCreateWithCString(NULL,
                                                     keyNameUri.c_str(),
                                                     kCFStringEncodingUTF8);

    CFReleaser<CFMutableDictionaryRef> attrDict = CFDictionaryCreateMutable(NULL,
                                                             3,
                                                             &kCFTypeDictionaryKeyCallBacks,
                                                             NULL);

    uint32_t keySize;
    if (params.getKeyType() == KEY_TYPE_RSA)
      keySize = static_cast<const RsaKeyParams&>(params).getKeySize();
    else if (params.getKeyType() == KEY_TYPE_ECDSA)
      keySize = static_cast<const EcdsaKeyParams&>(params).getKeySize();
    else
      throw SecurityException("generateKeyPair: Unsupported key type");

    CFReleaser<CFNumberRef> cfKeySize = CFNumberCreate(0, kCFNumberIntType, &keySize);

    CFDictionaryAddValue(attrDict.get(), kSecAttrKeyType, getAsymmetricKeyType
      (params.getKeyType()));
    CFDictionaryAddValue(attrDict.get(), kSecAttrKeySizeInBits, cfKeySize.get());
    CFDictionaryAddValue(attrDict.get(), kSecAttrLabel, keyLabel.get());

    CFReleaser<SecKeyRef> publicKey, privateKey;
    OSStatus res = SecKeyGeneratePair((CFDictionaryRef)attrDict.get(),
                                      &publicKey.get(), &privateKey.get());

    if (res != errSecSuccess)
      throw SecurityException("Fail to create a key pair");
  }

  void
  OSXPrivateKeyStorage::deleteKeyPair(const Name& keyName)
  {
    CFReleaser<CFStringRef> keyLabel =
      CFStringCreateWithCString(0,
                                keyName.toUri().c_str(),
                                kCFStringEncodingUTF8);

    CFReleaser<CFMutableDictionaryRef> searchDict =
      CFDictionaryCreateMutable(0, 5,
                                &kCFTypeDictionaryKeyCallBacks,
                                &kCFTypeDictionaryValueCallBacks);

    CFDictionaryAddValue(searchDict.get(), kSecClass, kSecClassKey);
    CFDictionaryAddValue(searchDict.get(), kSecAttrLabel, keyLabel.get());
    CFDictionaryAddValue(searchDict.get(), kSecMatchLimit, kSecMatchLimitAll);
    SecItemDelete(searchDict.get());
  }

  void
  OSXPrivateKeyStorage::generateKey(const Name & keyName, const KeyParams& params)
  {
    throw SecurityException("OSXPrivateKeyStorage::generateKey is not supported");
#if 0
    if(doesKeyExist(keyName, KEY_CLASS_SYMMETRIC))
        throw SecurityException("keyName has existed!");

    string keyNameUri =  toInternalKeyName(keyName, KEY_CLASS_SYMMETRIC);

    CFReleaser<CFMutableDictionaryRef> attrDict = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                                0,
                                                                &kCFTypeDictionaryKeyCallBacks,
                                                                &kCFTypeDictionaryValueCallBacks);

    CFReleaser<CFStringRef> keyLabel = CFStringCreateWithCString(NULL,
                                                     keyNameUri.c_str(),
                                                     kCFStringEncodingUTF8);

    CFReleaser<CFNumberRef> cfKeySize = CFNumberCreate(0, kCFNumberIntType, &keySize);

    CFDictionaryAddValue(attrDict.get(), kSecAttrKeyType, getSymmetricKeyType(keyType));
    CFDictionaryAddValue(attrDict.get(), kSecAttrKeySizeInBits, cfKeySize.get());
    CFDictionaryAddValue(attrDict.get(), kSecAttrIsPermanent, kCFBooleanTrue);
    CFDictionaryAddValue(attrDict.get(), kSecAttrLabel, keyLabel.get());

    CFReleaser<CFErrorRef> error = NULL;

    SecKeyRef symmetricKey = SecKeyGenerateSymmetric(attrDict.get(), &error.get());

    if (error.get() != 0)
        throw SecurityException("Fail to create a symmetric key");
#endif
  }

  ptr_lib::shared_ptr<PublicKey> OSXPrivateKeyStorage::getPublicKey(const Name & keyName)
  {
    _LOG_TRACE("OSXPrivateKeyStorage::getPublickey");

    CFReleaser<SecKeychainItemRef> publicKey = getKey(keyName, KEY_CLASS_PUBLIC);
    if (publicKey.get() == 0)
      throw SecurityException("The requested public key [" + keyName.toUri() + "] does not exist in the OSX Keychain");

    CFReleaser<CFDataRef> exportedKey;
    OSStatus res = SecItemExport(publicKey.get(),
                                  kSecFormatOpenSSL,
                                  0,
                                  NULL,
                                  &exportedKey.get());
    if (res != errSecSuccess)
      throw SecurityException("Cannot export the requested public key from the OSX Keychain");

    Blob blob(CFDataGetBytePtr(exportedKey.get()), CFDataGetLength(exportedKey.get()));

    return ptr_lib::shared_ptr<PublicKey>(new PublicKey(blob));
  }

  Blob OSXPrivateKeyStorage::sign(const uint8_t *data, size_t dataLength, const Name & keyName, DigestAlgorithm digestAlgo)
  {
    _LOG_TRACE("OSXPrivateKeyStorage::Sign");

    CFReleaser<CFDataRef> dataRef = CFDataCreateWithBytesNoCopy(0,
                                                                data,
                                                                dataLength,
                                                                kCFAllocatorNull);
    CFReleaser<SecKeychainItemRef> privateKey = getKey(keyName, KEY_CLASS_PRIVATE);
    if (privateKey.get() == 0)
      throw SecurityException("The private key [" + keyName.toUri() + "] does not exist in the OSX Keychain");

    CFReleaser<CFErrorRef> error;
    CFReleaser<SecTransformRef> signer = SecSignTransformCreate((SecKeyRef)privateKey.get(),
                                                                &error.get());
    if (error.get() != 0)
      throw SecurityException("Fail to create signer");

    Boolean set_res = SecTransformSetAttribute(signer.get(),
                                               kSecTransformInputAttributeName,
                                               dataRef.get(),
                                               &error.get());
    if (error.get() != 0)
      throw SecurityException("Fail to configure input of signer");

    set_res = SecTransformSetAttribute(signer.get(),
                                       kSecDigestTypeAttribute,
                                       getDigestAlgorithm(digestAlgo),
                                       &error.get());
    if (error.get() != 0)
      throw SecurityException("Fail to configure digest algorithm of signer");

    long digestSize = getDigestSize(digestAlgo);
    CFReleaser<CFNumberRef> cfDigestSize = CFNumberCreate(0, kCFNumberLongType, &digestSize);
    set_res = SecTransformSetAttribute(signer.get(),
                                       kSecDigestLengthAttribute,
                                       cfDigestSize.get(),
                                       &error.get());
    if (error.get() != 0)
      throw SecurityException("Fail to configure digest size of signer");

    CFReleaser<CFDataRef> signature = (CFDataRef) SecTransformExecute(signer.get(), &error.get());
    if (error.get() != 0) {
      CFShow(error.get());
      throw SecurityException("Fail to sign data");
    }

    if (signature.get() == 0)
      throw SecurityException("Signature is NULL!\n");

    return Blob(CFDataGetBytePtr(signature.get()), CFDataGetLength(signature.get()));
  }

  Blob OSXPrivateKeyStorage::decrypt(const Name & keyName, const uint8_t* data, size_t dataLength, bool sym)
  {
    throw SecurityException("OSXPrivateKeyStorage::decrypt is not supported");
#if 0
    _LOG_TRACE("OSXPrivateKeyStorage::Decrypt");

    KeyClass keyClass;
    if(sym)
        keyClass = KEY_CLASS_SYMMETRIC;
    else
        keyClass = KEY_CLASS_PRIVATE;

    CFReleaser<CFDataRef> dataRef = CFDataCreate(NULL,
                                      reinterpret_cast<const unsigned char*>(data),
                                      dataLength
                                      );

    CFReleaser<SecKeychainItemRef> decryptKey = getKey(keyName, keyClass);

    CFReleaser<CFErrorRef> error;
    CFReleaser<SecTransformRef> decrypt = SecDecryptTransformCreate((SecKeyRef)decryptKey.get(), &error.get());
    if (error.get() != 0)
      throw SecurityException("Fail to create decrypt");

    Boolean set_res = SecTransformSetAttribute(decrypt.get(),
                                               kSecTransformInputAttributeName,
                                               dataRef.get(),
                                               &error.get());
    if (error.get() != 0)
      throw SecurityException("Fail to configure decrypt");

    CFReleaser<CFDataRef> output = (CFDataRef) SecTransformExecute(decrypt.get(), &error.get());
    if (error.get() != 0)
      {
        CFShow(error.get());
        throw SecurityException("Fail to decrypt data");
      }
    if (output.get() == 0)
      throw SecurityException("Output is NULL!\n");

    return Blob(CFDataGetBytePtr(output.get()), CFDataGetLength(output.get()));
#endif
  }

  bool OSXPrivateKeyStorage::setACL(const Name & keyName, KeyClass keyClass, int acl, const string & appPath)
  {
    throw SecurityException("OSXPrivateKeyStorage::setACL is not supported");
#if 0
    SecKeychainItemRef privateKey = getKey(keyName, keyClass);

    SecAccessRef accRef;
    OSStatus acc_res = SecKeychainItemCopyAccess(privateKey, &accRef);

    CFArrayRef signACL = SecAccessCopyMatchingACLList(accRef,
                                                       kSecACLAuthorizationSign);

    SecACLRef aclRef = (SecACLRef) CFArrayGetValueAtIndex(signACL, 0);

    CFArrayRef appList;
    CFStringRef description;
    SecKeychainPromptSelector promptSelector;
    OSStatus acl_res = SecACLCopyContents(aclRef,
                                           &appList,
                                           &description,
                                           &promptSelector);

    CFMutableArrayRef newAppList = CFArrayCreateMutableCopy(NULL,
                                                            0,
                                                            appList);

    SecTrustedApplicationRef trustedApp;
    acl_res = SecTrustedApplicationCreateFromPath(appPath.c_str(),
                                                   &trustedApp);

    CFArrayAppendValue(newAppList, trustedApp);


    CFArrayRef authList = SecACLCopyAuthorizations(aclRef);

    acl_res = SecACLRemove(aclRef);

    SecACLRef newACL;
    acl_res = SecACLCreateWithSimpleContents(accRef,
                                              newAppList,
                                              description,
                                              promptSelector,
                                              &newACL);

    acl_res = SecACLUpdateAuthorizations(newACL, authList);

    acc_res = SecKeychainItemSetAccess(privateKey, accRef);

    return true;
#endif
  }

  Blob OSXPrivateKeyStorage::encrypt(const Name & keyName, const uint8_t* data, size_t dataLength, bool sym)
  {
    throw SecurityException("OSXPrivateKeyStorage::encrypt is not supported");
#if 0
    _LOG_TRACE("OSXPrivateKeyStorage::Encrypt");

    KeyClass keyClass;
    if(sym)
        keyClass = KEY_CLASS_SYMMETRIC;
    else
        keyClass = KEY_CLASS_PUBLIC;

    CFReleaser<CFDataRef> dataRef = CFDataCreate(NULL,
                                      reinterpret_cast<const unsigned char*>(data),
                                      dataLength
                                      );

    CFReleaser<SecKeychainItemRef> encryptKey = getKey(keyName, keyClass);

    CFReleaser<CFErrorRef> error;
    CFReleaser<SecTransformRef> encrypt = SecEncryptTransformCreate((SecKeyRef)encryptKey.get(), &error.get());
    if (error.get() != 0)
      throw SecurityException("Fail to create encrypt");

    Boolean set_res = SecTransformSetAttribute(encrypt.get(),
                                               kSecTransformInputAttributeName,
                                               dataRef.get(),
                                               &error.get());
    if (error.get() != 0)
      throw SecurityException("Fail to configure encrypt");

    CFReleaser<CFDataRef> output = (CFDataRef) SecTransformExecute(encrypt.get(), &error.get());
    if (error.get() != 0)
      throw SecurityException("Fail to encrypt data");

    if (output.get() == 0)
      throw SecurityException("Output is NULL!\n");

    return Blob(CFDataGetBytePtr(output.get()), CFDataGetLength(output.get()));
#endif
  }

  bool OSXPrivateKeyStorage::doesKeyExist(const Name & keyName, KeyClass keyClass)
  {
    _LOG_TRACE("OSXPrivateKeyStorage::doesKeyExist");

    string keyNameUri = toInternalKeyName(keyName, keyClass);

    CFReleaser<CFStringRef> keyLabel = CFStringCreateWithCString(NULL,
                                                     keyNameUri.c_str(),
                                                     kCFStringEncodingUTF8);

    CFReleaser<CFMutableDictionaryRef> attrDict = CFDictionaryCreateMutable(NULL,
                                                                3,
                                                                &kCFTypeDictionaryKeyCallBacks,
                                                                NULL);

    CFDictionaryAddValue(attrDict.get(), kSecAttrKeyClass, getKeyClass(keyClass));
    CFDictionaryAddValue(attrDict.get(), kSecAttrLabel, keyLabel.get());
    CFDictionaryAddValue(attrDict.get(), kSecReturnRef, kCFBooleanTrue);

    CFReleaser<SecKeychainItemRef> itemRef;
    OSStatus res = SecItemCopyMatching((CFDictionaryRef)attrDict.get(), (CFTypeRef*)&itemRef.get());

    if(res == errSecItemNotFound)
      return true;
    else
      return false;

  }

  CFReleaser<SecKeychainItemRef> OSXPrivateKeyStorage::getKey(const Name & keyName, KeyClass keyClass)
  {
    string keyNameUri = toInternalKeyName(keyName, keyClass);

    CFReleaser<CFStringRef> keyLabel = CFStringCreateWithCString(NULL,
                                                     keyNameUri.c_str(),
                                                     kCFStringEncodingUTF8);

    CFReleaser<CFMutableDictionaryRef> attrDict = CFDictionaryCreateMutable(NULL,
                                                             5,
                                                             &kCFTypeDictionaryKeyCallBacks,
                                                             NULL);

    CFDictionaryAddValue(attrDict.get(), kSecClass, kSecClassKey);
    CFDictionaryAddValue(attrDict.get(), kSecAttrLabel, keyLabel.get());
    CFDictionaryAddValue(attrDict.get(), kSecAttrKeyClass, getKeyClass(keyClass));
    CFDictionaryAddValue(attrDict.get(), kSecReturnRef, kCFBooleanTrue);

    CFReleaser<SecKeychainItemRef> keyItem;
    OSStatus res = SecItemCopyMatching((CFDictionaryRef)attrDict.get(), (CFTypeRef*)&keyItem.get());

    if(res != errSecSuccess)
      return NULL;
    else
      return keyItem;
  }

  string OSXPrivateKeyStorage::toInternalKeyName(const Name & keyName, KeyClass keyClass)
  {
    string keyUri = keyName.toUri();

    if(KEY_CLASS_SYMMETRIC == keyClass)
      return keyUri + "/symmetric";
    else
      return keyUri;
  }

  const CFTypeRef OSXPrivateKeyStorage::getAsymmetricKeyType(KeyType keyType)
  {
    switch(keyType){
    case KEY_TYPE_RSA:
      return kSecAttrKeyTypeRSA;
    case KEY_TYPE_ECDSA:
      return kSecAttrKeyTypeECDSA;
    default:
      return NULL;
    }
  }

  const CFTypeRef OSXPrivateKeyStorage::getSymmetricKeyType(KeyType keyType)
  {
    switch(keyType){
    case KEY_TYPE_AES:
      return kSecAttrKeyTypeAES;
    default:
      return NULL;
    }
  }

  const CFTypeRef OSXPrivateKeyStorage::getKeyClass(KeyClass keyClass)
  {
    switch(keyClass){
    case KEY_CLASS_PRIVATE:
      return kSecAttrKeyClassPrivate;
    case KEY_CLASS_PUBLIC:
      return kSecAttrKeyClassPublic;
    case KEY_CLASS_SYMMETRIC:
      return kSecAttrKeyClassSymmetric;
    default:
      return NULL;
    }
  }

  const CFStringRef OSXPrivateKeyStorage::getDigestAlgorithm(DigestAlgorithm digestAlgorithm)
  {
    switch(digestAlgorithm){
    // case DIGEST_MD2:
    //   return kSecDigestMD2;
    // case DIGEST_MD5:
    //   return kSecDigestMD5;
    // case DIGEST_SHA1:
    //   return kSecDigestSHA1;
    case DIGEST_ALGORITHM_SHA256:
      return kSecDigestSHA2;
    default:
      return NULL;
    }
  }

  long OSXPrivateKeyStorage::getDigestSize(DigestAlgorithm digestAlgo)
  {
    switch(digestAlgo){
    case DIGEST_ALGORITHM_SHA256:
      return 256;
    // case DIGEST_SHA1:
    // case DIGEST_MD2:
    // case DIGEST_MD5:
    //   return 0;
    default:
      return -1;
    }
  }

}

#endif // NDN_CPP_HAVE_OSX_SECURITY
