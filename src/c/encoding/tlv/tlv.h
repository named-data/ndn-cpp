/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * Derived from tlv.hpp by Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_H
#define NDN_TLV_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
  ndn_Tlv_Interest      = 1,
  ndn_Tlv_Data          = 2,
  ndn_Tlv_Name          = 3,
  ndn_Tlv_NameComponent = 4,
  ndn_Tlv_Selectors     = 5,
  ndn_Tlv_Nonce         = 6,
  ndn_Tlv_Scope         = 7,
  ndn_Tlv_InterestLifetime          = 8,
  ndn_Tlv_MinSuffixComponents       = 9,
  ndn_Tlv_MaxSuffixComponents       = 10,
  ndn_Tlv_PublisherPublicKeyLocator = 11,
  ndn_Tlv_Exclude       = 12,
  ndn_Tlv_ChildSelector = 13,
  ndn_Tlv_MustBeFresh   = 14,
  ndn_Tlv_Any           = 15,
  ndn_Tlv_MetaInfo      = 16,
  ndn_Tlv_Content       = 17,
  ndn_Tlv_SignatureInfo = 18,
  ndn_Tlv_SignatureValue = 19,
  ndn_Tlv_ContentType   = 20,
  ndn_Tlv_FreshnessPeriod = 21,
  ndn_Tlv_SignatureType = 22,
  ndn_Tlv_KeyLocator    = 23,
  ndn_Tlv_KeyLocatorDigest = 24,

  ndn_Tlv_AppPrivateBlock1 = 128,
  ndn_Tlv_AppPrivateBlock2 = 32767
};

enum {
  ndn_Tlv_SignatureType_DigestSha256 = 0,
  ndn_Tlv_SignatureType_SignatureSha256WithRsa = 1
};

enum {
  ndn_Tlv_ContentType_Default = 0,
  ndn_Tlv_ContentType_Link = 1,
  ndn_Tlv_ContentType_Key = 2
};
  
#ifdef __cplusplus
}
#endif

#endif
