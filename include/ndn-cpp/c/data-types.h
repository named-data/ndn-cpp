/**
 * Copyright (C) 2013 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_DATA_TYPES_H
#define NDN_DATA_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/** ndn_ContentType defines constants for the MetaInfo "type" field.  Note that the constants for BLOB, LINK and KEY
 * are the same as defined in the NDN-TLV spec.
 */
typedef enum {
  ndn_ContentType_BLOB = 0,
  // ContentType DATA is deprecated.  Use ndn_ContentType_BLOB.
  ndn_ContentType_DATA = 0,
  ndn_ContentType_LINK = 1,
  ndn_ContentType_KEY =  2,
  // ContentType ENCR, GONE and NACK are not supported in NDN-TLV encoding and are deprecated.
  ndn_ContentType_ENCR = 3,
  ndn_ContentType_GONE = 4,
  ndn_ContentType_NACK = 5
} ndn_ContentType;

#ifdef __cplusplus
}
#endif

#endif
