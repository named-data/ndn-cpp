/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_INTEREST_H
#define NDN_INTEREST_H

#include <ndn-cpp/c/interest-types.h>
#include "name.h"
#include "publisher-public-key-digest.h"
#include "key-locator.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An ndn_ExcludeEntry holds an ndn_ExcludeType, and if it is a COMPONENT, it holds a pointer to the component value.
 */
struct ndn_ExcludeEntry {
  ndn_ExcludeType type;
  struct ndn_NameComponent component;
};

/**
 * 
 * @param self pointer to the ndn_NameComponent struct
 * @param type one of the ndn_ExcludeType enum
 * @param component the pre-allocated buffer for the component value, only used if type is ndn_Exclude_COMPONENT
 * @param componentLength the number of bytes in value, only used if type is ndn_Exclude_COMPONENT
 */
static __inline void ndn_ExcludeEntry_initialize(struct ndn_ExcludeEntry *self, ndn_ExcludeType type, const uint8_t *component, size_t componentLength) 
{
  self->type = type;
  ndn_NameComponent_initialize(&self->component, component, componentLength);
}

/**
 * An ndn_Exclude holds an array of ndn_ExcludeEntry.
 */
struct ndn_Exclude {
  struct ndn_ExcludeEntry *entries;  /**< pointer to the array of entries. */
  size_t maxEntries;                 /**< the number of elements in the allocated entries array */
  size_t nEntries;                   /**< the number of entries in the exclude, 0 for no exclude */
};
/**
 * Initialize an ndn_Exclude struct with the entries array.
 * @param self A pointer to the ndn_Exclude struct.
 * @param entries the pre-allocated array of ndn_ExcludeEntry
 * @param maxEntries the number of elements in the allocated entries array
 */
static __inline void ndn_Exclude_initialize(struct ndn_Exclude *self, struct ndn_ExcludeEntry *entries, size_t maxEntries) 
{
  self->entries = entries;
  self->maxEntries = maxEntries;
  self->nEntries = 0;
}

/**
 * Compare the components using NDN component ordering.
 * A component is less if it is shorter, otherwise if equal length do a byte comparison.
 * @param component1 A pointer to the first name component.
 * @param component2 A pointer to the second name component.
 * @return -1 if component1 is less than component2, 1 if greater or 0 if equal.
 */
int ndn_Exclude_compareComponents(struct ndn_NameComponent *component1, struct ndn_NameComponent *component2);

/**
 * An ndn_Interest holds an ndn_Name and other fields for an interest.
 */
struct ndn_Interest {
  struct ndn_Name name;
  int minSuffixComponents;  /**< -1 for none */
  int maxSuffixComponents;  /**< -1 for none */
  /** @deprecated.  The Interest publisherPublicKeyDigest is deprecated.  If you need a publisher public key digest, 
   * set the keyLocator keyLocatorType to KEY_LOCATOR_DIGEST and set its key data to the digest. */
  struct ndn_PublisherPublicKeyDigest publisherPublicKeyDigest;
  struct ndn_KeyLocator keyLocator;
  struct ndn_Exclude exclude;
  int childSelector;        /**< -1 for none */
  int answerOriginKind;     /**< -1 for none. If >= 0 and the ndn_Interest_ANSWER_STALE bit is not set, then MustBeFresh. */
  int scope;                /**< -1 for none */
  ndn_Milliseconds interestLifetimeMilliseconds; /**< -1.0 for none */
  struct ndn_Blob nonce;    /**< The blob whose value is a pointer to a pre-allocated buffer.  0 for none */
};

/**
 * Initialize an ndn_Interest struct with the pre-allocated nameComponents and excludeEntries,
 * and defaults for all the values.
 * @param self pointer to the ndn_Interest struct
 * @param nameComponents the pre-allocated array of ndn_NameComponent
 * @param maxNameComponents the number of elements in the allocated nameComponents array
 * @param excludeEntries the pre-allocated array of ndn_ExcludeEntry
 * @param maxExcludeEntries the number of elements in the allocated excludeEntries array
 * @param keyNameComponents The pre-allocated array of ndn_NameComponent for the keyLocator.
 * @param maxKeyNameComponents The number of elements in the allocated keyNameComponents array.
 */
static __inline void ndn_Interest_initialize
  (struct ndn_Interest *self, struct ndn_NameComponent *nameComponents, size_t maxNameComponents,
   struct ndn_ExcludeEntry *excludeEntries, size_t maxExcludeEntries, struct ndn_NameComponent *keyNameComponents, 
   size_t maxKeyNameComponents) 
{
  ndn_Name_initialize(&self->name, nameComponents, maxNameComponents);
  self->minSuffixComponents = -1;
  self->maxSuffixComponents = -1;
  ndn_PublisherPublicKeyDigest_initialize(&self->publisherPublicKeyDigest);
  ndn_Exclude_initialize(&self->exclude, excludeEntries, maxExcludeEntries);
  self->childSelector = -1;
  self->answerOriginKind = -1;
  self->scope = -1;
  self->interestLifetimeMilliseconds = -1.0;
  ndn_Blob_initialize(&self->nonce, 0, 0);
  ndn_KeyLocator_initialize(&self->keyLocator, keyNameComponents, maxKeyNameComponents);
}

/**
 * Return true if answerOriginKind indicates that the content must be fresh. If
 * answerOriginKind is not specified, the default is true.
 * @param self A pointer to the ndn_Interest struct.
 * @return 1 if must be fresh, otherwise 0.
 */
static __inline int ndn_Interest_getMustBeFresh(struct ndn_Interest *self)
{
  if (self->answerOriginKind < 0)
    return 1;
  else
    return (self->answerOriginKind & ndn_Interest_ANSWER_STALE) == 0 ? 1 : 0;
}

#ifdef __cplusplus
}
#endif

#endif

