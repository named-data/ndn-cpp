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
  ndn_Tlv_Interest =         5,
  ndn_Tlv_Data =             6,
  ndn_Tlv_Name =             7,
  ndn_Tlv_NameComponent =    8,
  ndn_Tlv_Selectors =        9,
  ndn_Tlv_Nonce =            10,
  ndn_Tlv_Scope =            11,
  ndn_Tlv_InterestLifetime = 12,
  ndn_Tlv_MinSuffixComponents = 13,
  ndn_Tlv_MaxSuffixComponents = 14,
  ndn_Tlv_PublisherPublicKeyLocator = 15,
  ndn_Tlv_Exclude =          16,
  ndn_Tlv_ChildSelector =    17,
  ndn_Tlv_MustBeFresh =      18,
  ndn_Tlv_Any =              19,
  ndn_Tlv_MetaInfo =         20,
  ndn_Tlv_Content =          21,
  ndn_Tlv_SignatureInfo =    22,
  ndn_Tlv_SignatureValue =   23,
  ndn_Tlv_ContentType =      24,
  ndn_Tlv_FreshnessPeriod =  25,
  ndn_Tlv_FinalBlockId =     26,
  ndn_Tlv_SignatureType =    27,
  ndn_Tlv_KeyLocator =       28,
  ndn_Tlv_KeyLocatorDigest = 29,
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

enum {
  ndn_Tlv_NfdCommand_ControlResponse = 101,
  ndn_Tlv_NfdCommand_StatusCode =      102,
  ndn_Tlv_NfdCommand_StatusText =      103
};

enum {
  ndn_Tlv_PrefixRegOptions_PrefixRegOptions = 101,
  ndn_Tlv_PrefixRegOptions_FaceId =           102,
  ndn_Tlv_PrefixRegOptions_Flags =            103,
  ndn_Tlv_PrefixRegOptions_Cost =             104,
  ndn_Tlv_PrefixRegOptions_ExpirationPeriod = 105,
  ndn_Tlv_PrefixRegOptions_Protocol =         107
};
  
#ifdef __cplusplus
}
#endif

#endif
