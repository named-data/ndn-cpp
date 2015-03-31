/**
 * Copyright (C) 2014-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
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

#include <math.h>
#include "tlv-name.h"
#include "tlv-key-locator.h"
#include "../../util/crypto.h"
#include "tlv-interest.h"

/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the TLVs in the body of the Exclude value.
 * @param context This is the ndn_Exclude struct pointer which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
static ndn_Error
encodeExcludeValue(const void *context, struct ndn_TlvEncoder *encoder)
{
  struct ndn_Exclude *exclude = (struct ndn_Exclude *)context;

  // TODO: Do we want to order the components (except for ANY)?
  ndn_Error error;
  size_t i;
  for (i = 0; i < exclude->nEntries; ++i) {
    struct ndn_ExcludeEntry *entry = &exclude->entries[i];

    if (entry->type == ndn_Exclude_COMPONENT) {
      if ((error = ndn_TlvEncoder_writeBlobTlv(encoder, ndn_Tlv_NameComponent, &entry->component.value)))
        return error;
    }
    else if (entry->type == ndn_Exclude_ANY) {
      if ((error = ndn_TlvEncoder_writeTypeAndLength(encoder, ndn_Tlv_Any, 0)))
        return error;
    }
    else
      return NDN_ERROR_unrecognized_ndn_ExcludeType;
  }

  return NDN_ERROR_success;
}

#ifndef ARDUINO // Skip deprecated binary XML to save space. (We will soon remove binary XML completely.)
/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the publisherPublicKeyDigest as a KeyLocatorDigest
 * in the body of the KeyLocator value.  (When we remove the deprecated publisherPublicKeyDigest, we won't need this.)
 * @param context This is the ndn_Interest struct pointer which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
static ndn_Error
encodeKeyLocatorSelectorPublisherPublicKeyDigestValue
  (const void *context, struct ndn_TlvEncoder *encoder)
{
  struct ndn_Interest *interest = (struct ndn_Interest *)context;

  ndn_Error error;
  if ((error = ndn_TlvEncoder_writeBlobTlv
       (encoder, ndn_Tlv_KeyLocatorDigest, &interest->publisherPublicKeyDigest.publisherPublicKeyDigest)))
    return error;

  return NDN_ERROR_success;
}
#endif

/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the TLVs in the body of the Selectors value.
 * @param context This is the ndn_Interest struct pointer which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
static ndn_Error
encodeSelectorsValue(const void *context, struct ndn_TlvEncoder *encoder)
{
  struct ndn_Interest *interest = (struct ndn_Interest *)context;
  ndn_Error error;
  size_t saveOffset;

  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlv
      (encoder, ndn_Tlv_MinSuffixComponents, interest->minSuffixComponents)))
    return error;
  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlv
      (encoder, ndn_Tlv_MaxSuffixComponents, interest->maxSuffixComponents)))
    return error;

#ifndef ARDUINO // Skip deprecated binary XML to save space. (We will soon remove binary XML completely.)
  // Save the offset and set omitZeroLength true so we can detect if the key locator is omitted to see if we need to
  //   write the publisherPublicKeyDigest.  (When we remove the deprecated publisherPublicKeyDigest, we can call
  //   with omitZeroLength true.)
  saveOffset = encoder->offset;
  if ((error = ndn_TlvEncoder_writeNestedTlv
       (encoder, ndn_Tlv_PublisherPublicKeyLocator, ndn_encodeTlvKeyLocatorValue,
        &interest->keyLocator, 1)))
    return error;
  if (encoder->offset == saveOffset) {
    // There is no keyLocator.  If there is a publisherPublicKeyDigest, then encode as KEY_LOCATOR_DIGEST.
    if (interest->publisherPublicKeyDigest.publisherPublicKeyDigest.length > 0) {
      if ((error = ndn_TlvEncoder_writeNestedTlv
           (encoder, ndn_Tlv_PublisherPublicKeyLocator,
            encodeKeyLocatorSelectorPublisherPublicKeyDigestValue, interest, 0)))
        return error;
    }
  }
#else
  if ((error = ndn_TlvEncoder_writeNestedTlv
       (encoder, ndn_Tlv_PublisherPublicKeyLocator, ndn_encodeTlvKeyLocatorValue,
        &interest->keyLocator, 0)))
    return error;
#endif

  if (interest->exclude.nEntries > 0)
    if ((error = ndn_TlvEncoder_writeNestedTlv(encoder, ndn_Tlv_Exclude, encodeExcludeValue, &interest->exclude, 0)))
      return error;

  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlv
      (encoder, ndn_Tlv_ChildSelector, interest->childSelector)))
    return error;

  // Instead of using ndn_Interest_getMustBeFresh, check answerOriginKind directly so that we can
  //   return an error if unsupported bits are set.
  if (interest->answerOriginKind < 0 || interest->answerOriginKind == 0) {
    // MustBeFresh == true.
    if ((error = ndn_TlvEncoder_writeTypeAndLength(encoder, ndn_Tlv_MustBeFresh, 0)))
      return error;
  }
  else if ((interest->answerOriginKind & ndn_Interest_ANSWER_STALE) != 0) {
    // MustBeFresh == false, so nothing to encode.
  }
  else
    // This error will be irrelevant when we drop support for binary XML answerOriginKind.
    return NDN_ERROR_Unsupported_answerOriginKind_bits_for_encoding_TLV_MustBeFresh;

  return NDN_ERROR_success;
}

/* An InterestValueContext is for passing the context to encodeInterestValue so
 *   that we can include signedPortionBeginOffset and signedPortionEndOffset.
 */
struct InterestValueContext {
  const struct ndn_Interest *interest;
  size_t *signedPortionBeginOffset;
  size_t *signedPortionEndOffset;
};

/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the TLVs in the body of the Interest value.
 * @param context This is the InterestValueContext struct pointer which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
static ndn_Error
encodeInterestValue(const void *context, struct ndn_TlvEncoder *encoder)
{
  const struct InterestValueContext *interestValueContext =
    (const struct InterestValueContext *)context;
  const struct ndn_Interest *interest = interestValueContext->interest;
  ndn_Error error;
  uint8_t nonceBuffer[4];
  struct ndn_Blob nonceBlob;

  if ((error = ndn_encodeTlvName
       (&interest->name, interestValueContext->signedPortionBeginOffset,
        interestValueContext->signedPortionEndOffset, encoder)))
    return error;
  // For Selectors, set omitZeroLength true.
  if ((error = ndn_TlvEncoder_writeNestedTlv(encoder, ndn_Tlv_Selectors, encodeSelectorsValue, interest, 1)))
    return error;

  // Encode the Nonce as 4 bytes.
  nonceBlob.length = sizeof(nonceBuffer);
  if (interest->nonce.length == 0) {
    // Generate a random nonce.
    ndn_generateRandomBytes(nonceBuffer, sizeof(nonceBuffer));
    nonceBlob.value = nonceBuffer;
  }
  else if (interest->nonce.length < 4) {
    // TLV encoding requires 4 bytes, so pad out to 4 using random bytes.
    ndn_memcpy(nonceBuffer, interest->nonce.value, interest->nonce.length);
    ndn_generateRandomBytes(nonceBuffer + interest->nonce.length, sizeof(nonceBuffer) - interest->nonce.length);
    nonceBlob.value = nonceBuffer;
  }
  else
    // TLV encoding requires 4 bytes, so truncate to 4.
    nonceBlob.value = interest->nonce.value;
  if ((error = ndn_TlvEncoder_writeBlobTlv(encoder, ndn_Tlv_Nonce, &nonceBlob)))
    return error;

  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlv
      (encoder, ndn_Tlv_Scope, interest->scope)))
    return error;
  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlvFromDouble
      (encoder, ndn_Tlv_InterestLifetime, interest->interestLifetimeMilliseconds)))
    return error;

  return NDN_ERROR_success;
}

ndn_Error
ndn_encodeTlvInterest
  (const struct ndn_Interest *interest, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset, struct ndn_TlvEncoder *encoder)
{
  // Create the context to pass to encodeInterestValue.
  struct InterestValueContext interestValueContext;
  interestValueContext.interest = interest;
  interestValueContext.signedPortionBeginOffset = signedPortionBeginOffset;
  interestValueContext.signedPortionEndOffset = signedPortionEndOffset;

  return ndn_TlvEncoder_writeNestedTlv
    (encoder, ndn_Tlv_Interest, encodeInterestValue, &interestValueContext, 0);
}

static ndn_Error
decodeExclude(struct ndn_Exclude *exclude, struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t endOffset;
  if ((error = ndn_TlvDecoder_readNestedTlvsStart(decoder, ndn_Tlv_Exclude, &endOffset)))
    return error;

  exclude->nEntries = 0;
  while (1) {
    int gotExpectedTag;
    int isAny;

    if ((error = ndn_TlvDecoder_peekType(decoder, ndn_Tlv_NameComponent, endOffset, &gotExpectedTag)))
      return error;
    if (gotExpectedTag) {
      // Component.
      struct ndn_Blob component;
      if ((error = ndn_TlvDecoder_readBlobTlv(decoder, ndn_Tlv_NameComponent, &component)))
        return error;

      if ((error = ndn_Exclude_appendComponent
           (exclude, component.value, component.length)))
        return error;
      continue;
    }

    if ((error = ndn_TlvDecoder_readBooleanTlv(decoder, ndn_Tlv_Any, endOffset, &isAny)))
      return error;
    if (isAny) {
      if ((error = ndn_Exclude_appendAny(exclude)))
        return error;
      continue;
    }

    // Else no more entries.
    break;
  }

  if ((error = ndn_TlvDecoder_finishNestedTlvs(decoder, endOffset)))
    return error;

  return NDN_ERROR_success;
}

static ndn_Error
decodeSelectors(struct ndn_Interest *interest, struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t endOffset;
  int gotExpectedType;
  int mustBeFresh;

  if ((error = ndn_TlvDecoder_readNestedTlvsStart(decoder, ndn_Tlv_Selectors, &endOffset)))
    return error;

  if ((error = ndn_TlvDecoder_readOptionalNonNegativeIntegerTlv
       (decoder, ndn_Tlv_MinSuffixComponents, endOffset, &interest->minSuffixComponents)))
    return error;
  if ((error = ndn_TlvDecoder_readOptionalNonNegativeIntegerTlv
       (decoder, ndn_Tlv_MaxSuffixComponents, endOffset, &interest->maxSuffixComponents)))
    return error;

#ifndef ARDUINO // Skip deprecated binary XML to save space. (We will soon remove binary XML completely.)
  // Initially set publisherPublicKeyDigest to none.
  ndn_Blob_initialize(&interest->publisherPublicKeyDigest.publisherPublicKeyDigest, 0, 0);
#endif
  if ((error = ndn_TlvDecoder_peekType
       (decoder, ndn_Tlv_PublisherPublicKeyLocator, endOffset, &gotExpectedType)))
    return error;
  if (gotExpectedType) {
    if ((error = ndn_decodeTlvKeyLocator
         (ndn_Tlv_PublisherPublicKeyLocator, &interest->keyLocator, decoder)))
      return error;
#ifndef ARDUINO // Skip deprecated binary XML to save space. (We will soon remove binary XML completely.)
    if (interest->keyLocator.type == ndn_KeyLocatorType_KEY_LOCATOR_DIGEST)
      // For backwards compatibility, also set the publisherPublicKeyDigest.
      interest->publisherPublicKeyDigest.publisherPublicKeyDigest = interest->keyLocator.keyData;
#endif
  }
  else
    // Clear the key locator.
    ndn_KeyLocator_initialize(&interest->keyLocator, interest->keyLocator.keyName.components, interest->keyLocator.keyName.maxComponents);


  if ((error = ndn_TlvDecoder_peekType(decoder, ndn_Tlv_Exclude, endOffset, &gotExpectedType)))
    return error;
  if (gotExpectedType) {
    if ((error = decodeExclude(&interest->exclude, decoder)))
      return error;
  }
  else
    interest->exclude.nEntries = 0;

  if ((error = ndn_TlvDecoder_readOptionalNonNegativeIntegerTlv
       (decoder, ndn_Tlv_ChildSelector, endOffset, &interest->childSelector)))
    return error;

  if ((error = ndn_TlvDecoder_readBooleanTlv(decoder, ndn_Tlv_MustBeFresh, endOffset, &mustBeFresh)))
    return error;
  // Setting the ndn_Interest_ANSWER_STALE bit means mustBeFresh is false.
  // -1 means not specified where mustBeFresh is default true.
  interest->answerOriginKind = (!mustBeFresh ? ndn_Interest_ANSWER_STALE : -1);

  if ((error = ndn_TlvDecoder_finishNestedTlvs(decoder, endOffset)))
    return error;

  return NDN_ERROR_success;
}

ndn_Error
ndn_decodeTlvInterest
  (struct ndn_Interest *interest, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset, struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t endOffset;
  int gotExpectedType;

  if ((error = ndn_TlvDecoder_readNestedTlvsStart(decoder, ndn_Tlv_Interest, &endOffset)))
    return error;

  if ((error = ndn_decodeTlvName
       (&interest->name, signedPortionBeginOffset, signedPortionEndOffset,
        decoder)))
    return error;

  if ((error = ndn_TlvDecoder_peekType(decoder, ndn_Tlv_Selectors, endOffset, &gotExpectedType)))
    return error;
  if (gotExpectedType) {
    if ((error = decodeSelectors(interest, decoder)))
      return error;
  }
  else {
    // Set selectors to none.
    interest->minSuffixComponents = -1;
    interest->maxSuffixComponents = -1;
#ifndef ARDUINO // Skip deprecated binary XML to save space. (We will soon remove binary XML completely.)
    ndn_PublisherPublicKeyDigest_initialize(&interest->publisherPublicKeyDigest);
#endif
    interest->exclude.nEntries = 0;
    interest->childSelector = -1;
    interest->answerOriginKind = -1;
  }

  // Require a Nonce, but don't force it to be 4 bytes.
  if ((error = ndn_TlvDecoder_readBlobTlv(decoder, ndn_Tlv_Nonce, &interest->nonce)))
    return error;

  if ((error = ndn_TlvDecoder_readOptionalNonNegativeIntegerTlv
       (decoder, ndn_Tlv_Scope, endOffset, &interest->scope)))
    return error;
  if ((error = ndn_TlvDecoder_readOptionalNonNegativeIntegerTlvAsDouble
       (decoder, ndn_Tlv_InterestLifetime, endOffset, &interest->interestLifetimeMilliseconds)))
    return error;

  if ((error = ndn_TlvDecoder_finishNestedTlvs(decoder, endOffset)))
    return error;

  return NDN_ERROR_success;
}
