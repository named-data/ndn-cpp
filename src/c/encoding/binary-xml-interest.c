/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * Derived from Interest.js by Meki Cheraoui.
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

#include "binary-xml-name.h"
#include "binary-xml-publisher-public-key-digest.h"
#include "binary-xml-interest.h"

static ndn_Error encodeExclude(struct ndn_Exclude *exclude, struct ndn_BinaryXmlEncoder *encoder)
{
  ndn_Error error;
  size_t i;

  if (exclude->nEntries == 0)
    return NDN_ERROR_success;

  if ((error = ndn_BinaryXmlEncoder_writeElementStartDTag(encoder, ndn_BinaryXml_DTag_Exclude)))
    return error;

  // TODO: Do we want to order the components (except for ANY)?
  for (i = 0; i < exclude->nEntries; ++i) {
    struct ndn_ExcludeEntry *entry = &exclude->entries[i];

    if (entry->type == ndn_Exclude_COMPONENT) {
      if ((error = ndn_BinaryXmlEncoder_writeBlobDTagElement(encoder, ndn_BinaryXml_DTag_Component, &entry->component.value)))
        return error;
    }
    else if (entry->type == ndn_Exclude_ANY) {
      if ((error = ndn_BinaryXmlEncoder_writeElementStartDTag(encoder, ndn_BinaryXml_DTag_Any)))
        return error;
      if ((error = ndn_BinaryXmlEncoder_writeElementClose(encoder)))
        return error;
    }
    else
      return NDN_ERROR_unrecognized_ndn_ExcludeType;
  }

  if ((error = ndn_BinaryXmlEncoder_writeElementClose(encoder)))
    return error;

  return NDN_ERROR_success;
}

static ndn_Error decodeExclude(struct ndn_Exclude *exclude, struct ndn_BinaryXmlDecoder *decoder)
{
  ndn_Error error;
  if ((error = ndn_BinaryXmlDecoder_readElementStartDTag(decoder, ndn_BinaryXml_DTag_Exclude)))
    return error;

  exclude->nEntries = 0;
  while (1) {
    int gotExpectedTag;

    if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_Component, &gotExpectedTag)))
      return error;
    if (gotExpectedTag) {
      // Component
      struct ndn_Blob component;
      if ((error = ndn_BinaryXmlDecoder_readBinaryDTagElement(decoder, ndn_BinaryXml_DTag_Component, 0, &component)))
        return error;

      // Add the component entry.
      if (exclude->nEntries >= exclude->maxEntries)
        return NDN_ERROR_cannot_add_an_entry_past_the_maximum_number_of_entries_allowed_in_the_exclude;
      ndn_ExcludeEntry_initialize(exclude->entries + exclude->nEntries, ndn_Exclude_COMPONENT, component.value, component.length);
      ++exclude->nEntries;

      continue;
    }

    if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_Any, &gotExpectedTag)))
      return error;
    if (gotExpectedTag) {
      // Any
      if ((error = ndn_BinaryXmlDecoder_readElementStartDTag(decoder, ndn_BinaryXml_DTag_Any)))
        return error;
      if ((error = ndn_BinaryXmlDecoder_readElementClose(decoder)))
        return error;

      // Add the any entry.
      if (exclude->nEntries >= exclude->maxEntries)
        return NDN_ERROR_cannot_add_an_entry_past_the_maximum_number_of_entries_allowed_in_the_exclude;
      ndn_ExcludeEntry_initialize(exclude->entries + exclude->nEntries, ndn_Exclude_ANY, 0, 0);
      ++exclude->nEntries;

      continue;
    }

    if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_Bloom, &gotExpectedTag)))
      return error;
    if (gotExpectedTag) {
      // Skip the Bloom and treat it as Any.
      struct ndn_Blob value;
      if ((error = ndn_BinaryXmlDecoder_readBinaryDTagElement(decoder, ndn_BinaryXml_DTag_Bloom, 0, &value)))
        return error;

      // Add the any entry.
      if (exclude->nEntries >= exclude->maxEntries)
        return NDN_ERROR_cannot_add_an_entry_past_the_maximum_number_of_entries_allowed_in_the_exclude;
      ndn_ExcludeEntry_initialize(exclude->entries + exclude->nEntries, ndn_Exclude_ANY, 0, 0);
      ++exclude->nEntries;

      continue;
    }

    // Else no more entries.
    break;
  }

  if ((error = ndn_BinaryXmlDecoder_readElementClose(decoder)))
    return error;

  return NDN_ERROR_success;
}

ndn_Error ndn_encodeBinaryXmlInterest
  (struct ndn_Interest *interest, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset, struct ndn_BinaryXmlEncoder *encoder)
{
  ndn_Error error;
  if ((error = ndn_BinaryXmlEncoder_writeElementStartDTag(encoder, ndn_BinaryXml_DTag_Interest)))
    return error;

  if ((error = ndn_encodeBinaryXmlName
       (&interest->name, signedPortionBeginOffset, signedPortionEndOffset,
        encoder)))
    return error;

  if ((error = ndn_BinaryXmlEncoder_writeOptionalUnsignedDecimalIntDTagElement
      (encoder, ndn_BinaryXml_DTag_MinSuffixComponents, interest->minSuffixComponents)))
    return error;
  if ((error = ndn_BinaryXmlEncoder_writeOptionalUnsignedDecimalIntDTagElement
      (encoder, ndn_BinaryXml_DTag_MaxSuffixComponents, interest->maxSuffixComponents)))
    return error;

  if (interest->keyLocator.type == ndn_KeyLocatorType_KEY_LOCATOR_DIGEST && interest->keyLocator.keyData.length > 0) {
    // There is a KEY_LOCATOR_DIGEST.  Use this instead of the publisherPublicKeyDigest.
    if ((error = ndn_BinaryXmlEncoder_writeBlobDTagElement
        (encoder, ndn_BinaryXml_DTag_PublisherPublicKeyDigest, &interest->keyLocator.keyData)))
      return error;
  }
  else {
    // This will skip encoding if there is no publisherPublicKeyDigest.
    if ((error = ndn_encodeBinaryXmlPublisherPublicKeyDigest(&interest->publisherPublicKeyDigest, encoder)))
      return error;
  }

  // This will skip encoding if there is no exclude.
  if ((error = encodeExclude(&interest->exclude, encoder)))
    return error;

  if ((error = ndn_BinaryXmlEncoder_writeOptionalUnsignedDecimalIntDTagElement
      (encoder, ndn_BinaryXml_DTag_ChildSelector, interest->childSelector)))
    return error;
  if (interest->answerOriginKind >= 0 && interest->answerOriginKind != ndn_Interest_DEFAULT_ANSWER_ORIGIN_KIND) {
    if ((error = ndn_BinaryXmlEncoder_writeUnsignedDecimalIntDTagElement
        (encoder, ndn_BinaryXml_DTag_AnswerOriginKind, (unsigned int)interest->answerOriginKind)))
      return error;
  }
  if ((error = ndn_BinaryXmlEncoder_writeOptionalUnsignedDecimalIntDTagElement
      (encoder, ndn_BinaryXml_DTag_Scope, interest->scope)))
    return error;

  if ((error = ndn_BinaryXmlEncoder_writeOptionalTimeMillisecondsDTagElement
      (encoder, ndn_BinaryXml_DTag_InterestLifetime, interest->interestLifetimeMilliseconds)))
    return error;

  if ((error = ndn_BinaryXmlEncoder_writeOptionalBlobDTagElement(encoder, ndn_BinaryXml_DTag_Nonce, &interest->nonce)))
    return error;

  if ((error = ndn_BinaryXmlEncoder_writeElementClose(encoder)))
    return error;

  return NDN_ERROR_success;
}

ndn_Error ndn_decodeBinaryXmlInterest
  (struct ndn_Interest *interest, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset, struct ndn_BinaryXmlDecoder *decoder)
{
  ndn_Error error;
  int gotExpectedTag;

  if ((error = ndn_BinaryXmlDecoder_readElementStartDTag(decoder, ndn_BinaryXml_DTag_Interest)))
    return error;

  if ((error = ndn_decodeBinaryXmlName
       (&interest->name, signedPortionBeginOffset, signedPortionEndOffset,
        decoder)))
    return error;

  if ((error = ndn_BinaryXmlDecoder_readOptionalUnsignedIntegerDTagElement
      (decoder, ndn_BinaryXml_DTag_MinSuffixComponents, &interest->minSuffixComponents)))
    return error;
  if ((error = ndn_BinaryXmlDecoder_readOptionalUnsignedIntegerDTagElement
      (decoder, ndn_BinaryXml_DTag_MaxSuffixComponents, &interest->maxSuffixComponents)))
    return error;

  // Initially clear the keyLocator.
  ndn_KeyLocator_initialize(&interest->keyLocator, interest->keyLocator.keyName.components, interest->keyLocator.keyName.maxComponents);
  if ((error = ndn_decodeOptionalBinaryXmlPublisherPublicKeyDigest(&interest->publisherPublicKeyDigest, decoder)))
    return error;
  if (interest->publisherPublicKeyDigest.publisherPublicKeyDigest.length > 0) {
    // We keep the deprecated publisherPublicKeyDigest for backwards compatibility.  Also set the key locator.
    interest->keyLocator.type = ndn_KeyLocatorType_KEY_LOCATOR_DIGEST;
    interest->keyLocator.keyData = interest->publisherPublicKeyDigest.publisherPublicKeyDigest;
  }

  if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_Exclude, &gotExpectedTag)))
    return error;
  if (gotExpectedTag) {
    if ((error = decodeExclude(&interest->exclude, decoder)))
      return error;
  }
  else
    interest->exclude.nEntries = 0;

  if ((error = ndn_BinaryXmlDecoder_readOptionalUnsignedIntegerDTagElement
      (decoder, ndn_BinaryXml_DTag_ChildSelector, &interest->childSelector)))
    return error;
  if ((error = ndn_BinaryXmlDecoder_readOptionalUnsignedIntegerDTagElement
      (decoder, ndn_BinaryXml_DTag_AnswerOriginKind, &interest->answerOriginKind)))
    return error;
  if ((error = ndn_BinaryXmlDecoder_readOptionalUnsignedIntegerDTagElement
      (decoder, ndn_BinaryXml_DTag_Scope, &interest->scope)))
    return error;

  if ((error = ndn_BinaryXmlDecoder_readOptionalTimeMillisecondsDTagElement
       (decoder, ndn_BinaryXml_DTag_InterestLifetime, &interest->interestLifetimeMilliseconds)))
    return error;

  if ((error = ndn_BinaryXmlDecoder_readOptionalBinaryDTagElement
      (decoder, ndn_BinaryXml_DTag_Nonce, 0, &interest->nonce)))
    return error;

  if ((error = ndn_BinaryXmlDecoder_readElementClose(decoder)))
    return error;

  return NDN_ERROR_success;
}
