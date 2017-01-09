/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/schedule https://github.com/named-data/ndn-group-encrypt
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

#include <stdexcept>
#include "../c/util/time.h"
#include "../c/encoding/tlv/tlv-encoder.h"
#include "../c/encoding/tlv/tlv-decoder.h"
#include "../c/util/blob.h"
#include "../util/dynamic-uint8-vector.hpp"
#include <ndn-cpp/encrypt/schedule.hpp>

using namespace std;

namespace ndn {

Schedule::Result
Schedule::getCoveringInterval(MillisecondsSince1970 timeStamp) const
{
  Interval blackPositiveResult(true);
  Interval whitePositiveResult(true);

  Interval blackNegativeResult;
  Interval whiteNegativeResult;

  // Get the black result.
  calculateIntervalResult
    (blackIntervalList_, timeStamp, blackPositiveResult, blackNegativeResult);

  // If the black positive result is not empty, then isPositive must be false.
  if (!blackPositiveResult.isEmpty())
    return Result(false, blackPositiveResult);

  // Get the whiteResult.
  calculateIntervalResult
    (whiteIntervalList_, timeStamp, whitePositiveResult, whiteNegativeResult);

  if (whitePositiveResult.isEmpty() && !whiteNegativeResult.isValid()) {
    // There is no white interval covering the time stamp.
    // Return false and a 24-hour interval.
    double timeStampDateOnly =
      RepetitiveInterval::toDateOnlyMilliseconds(timeStamp);
    return Result
      (false, Interval(timeStampDateOnly, timeStampDateOnly + MILLISECONDS_IN_DAY));
  }

  if (!whitePositiveResult.isEmpty()) {
    // There is white interval covering the time stamp.
    // Return true and calculate the intersection.
    if (blackNegativeResult.isValid())
      return Result(true, whitePositiveResult.intersectWith(blackNegativeResult));
    else
      return Result(true, whitePositiveResult);
  }
  else
    // There is no white interval covering the time stamp.
    // Return false.
    return Result(false, whiteNegativeResult);
}

Blob
Schedule::wireEncode() const
{
  DynamicUInt8Vector output(256);
  ndn_Error error;
  struct ndn_TlvEncoder encoder;
  ndn_TlvEncoder_initialize(&encoder, &output);
  if ((error = ndn_TlvEncoder_writeNestedTlv
       (&encoder, ndn_Tlv_Encrypt_Schedule, encodeScheduleValue, this, 0)))
    throw runtime_error(ndn_getErrorString(error));

  return output.finish(encoder.offset);
}

void
Schedule::wireDecode(const uint8_t *input, size_t inputLength)
{
  struct ndn_TlvDecoder decoder;
  ndn_TlvDecoder_initialize(&decoder, input, inputLength);

  ndn_Error error;
  size_t endOffset;

  if ((error = ndn_TlvDecoder_readNestedTlvsStart
       (&decoder, ndn_Tlv_Encrypt_Schedule, &endOffset)))
    throw runtime_error(ndn_getErrorString(error));

  // Decode the whiteIntervalList.
  whiteIntervalList_.clear();
  size_t listEndOffset;
  if ((error = ndn_TlvDecoder_readNestedTlvsStart
       (&decoder, ndn_Tlv_Encrypt_WhiteIntervalList, &listEndOffset)))
    throw runtime_error(ndn_getErrorString(error));
  while (decoder.offset < listEndOffset)
    sortedSetAdd(whiteIntervalList_, decodeRepetitiveInterval(&decoder));
  if ((error = ndn_TlvDecoder_finishNestedTlvs(&decoder, listEndOffset)))
    throw runtime_error(ndn_getErrorString(error));

  // Decode the blackIntervalList.
  blackIntervalList_.clear();
  if ((error = ndn_TlvDecoder_readNestedTlvsStart
       (&decoder, ndn_Tlv_Encrypt_BlackIntervalList, &listEndOffset)))
    throw runtime_error(ndn_getErrorString(error));
  while (decoder.offset < listEndOffset)
    sortedSetAdd(blackIntervalList_, decodeRepetitiveInterval(&decoder));
  if ((error = ndn_TlvDecoder_finishNestedTlvs(&decoder, listEndOffset)))
    throw runtime_error(ndn_getErrorString(error));

  if ((error = ndn_TlvDecoder_finishNestedTlvs(&decoder, endOffset)))
    throw runtime_error(ndn_getErrorString(error));
}

MillisecondsSince1970
Schedule::fromIsoString(const string& dateString)
{
  ndn_MillisecondsSince1970 result;
  ndn_Error error;
  if ((error = ndn_fromIsoString(dateString.c_str(), &result)))
    throw runtime_error(ndn_getErrorString(error));

  return result;
}

string
Schedule::toIsoString(MillisecondsSince1970 msSince1970)
{
  char isoString[25];
  ndn_Error error;
  if ((error = ndn_toIsoString(msSince1970, 0, isoString)))
    throw runtime_error(ndn_getErrorString(error));

  return isoString;
}

void
Schedule::sortedSetAdd
  (vector<ptr_lib::shared_ptr<RepetitiveInterval> >& list,
   const ptr_lib::shared_ptr<RepetitiveInterval>& element)
{
  // Find the index of the first element where it is not less than element.
  size_t i = 0;
  while (i < list.size()) {
    int comparison = list[i]->compare(*element);
    if (comparison == 0)
      // Don't add a duplicate.
      return;
    if (!(comparison < 0))
      break;

    ++i;
  }

  list.insert(list.begin() + i, element);
}

/**
 * This private function is called by ndn_TlvEncoder_writeNestedTlv to write the TLVs
 * in the body of the RepetitiveInterval value.
 * @param context This is the RepetitiveInterval pointer which was passed to
 * writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
ndn_Error
Schedule::encodeRepetitiveIntervalValue
  (const void *context, ndn_TlvEncoder *encoder)
{
  const RepetitiveInterval& repetitiveInterval =
    *(const RepetitiveInterval*)context;
  ndn_Error error;

  string startDateString = toIsoString(repetitiveInterval.getStartDate());
  ndn_Blob startDateBlob;
  ndn_Blob_initialize
    (&startDateBlob, (const uint8_t*)startDateString.c_str(), startDateString.size());
  if ((error = ndn_TlvEncoder_writeBlobTlv
       (encoder, ndn_Tlv_Encrypt_StartDate, &startDateBlob)))
    return error;

  string endDateString = toIsoString(repetitiveInterval.getEndDate());
  ndn_Blob endDateBlob;
  ndn_Blob_initialize
    (&endDateBlob, (const uint8_t*)endDateString.c_str(), endDateString.size());
  if ((error = ndn_TlvEncoder_writeBlobTlv
       (encoder, ndn_Tlv_Encrypt_EndDate, &endDateBlob)))
    return error;

  if ((error = ndn_TlvEncoder_writeNonNegativeIntegerTlv
       (encoder, ndn_Tlv_Encrypt_IntervalStartHour,
        repetitiveInterval.getIntervalStartHour())))
    return error;
  if ((error = ndn_TlvEncoder_writeNonNegativeIntegerTlv
       (encoder, ndn_Tlv_Encrypt_IntervalEndHour,
        repetitiveInterval.getIntervalEndHour())))
    return error;
  if ((error = ndn_TlvEncoder_writeNonNegativeIntegerTlv
       (encoder, ndn_Tlv_Encrypt_NRepeats,
        repetitiveInterval.getNRepeats())))
    return error;
  if ((error = ndn_TlvEncoder_writeNonNegativeIntegerTlv
       (encoder, ndn_Tlv_Encrypt_RepeatUnit,
        repetitiveInterval.getRepeatUnit())))
    return error;

  return NDN_ERROR_success;
}

/**
 * This private function is called by ndn_TlvEncoder_writeNestedTlv to write the TLVs
 * in the body of the RepetitiveInterval list value, such as
 * WhiteIntervalList.
 * @param context This is the vector<ptr_lib::shared_ptr<RepetitiveInterval> >
 * pointer which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
ndn_Error
Schedule::encodeRepetitiveIntervalListValue
  (const void *context, ndn_TlvEncoder *encoder)
{
  const vector<ptr_lib::shared_ptr<RepetitiveInterval> >& list =
    *(const vector<ptr_lib::shared_ptr<RepetitiveInterval> >*)context;
  ndn_Error error;

  for (size_t i = 0; i < list.size(); ++i) {
    if ((error = ndn_TlvEncoder_writeNestedTlv
         (encoder, ndn_Tlv_Encrypt_RepetitiveInterval,
          encodeRepetitiveIntervalValue, list[i].get(), 0)))
      return error;
  }

  return NDN_ERROR_success;
}

/**
 * This private function is called by ndn_TlvEncoder_writeNestedTlv to write the TLVs
 * in the body of the Schedule value.
 * @param context This is the Schedule pointer which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
ndn_Error
Schedule::encodeScheduleValue(const void *context, ndn_TlvEncoder *encoder)
{
  const Schedule& schedule = *(const Schedule*)context;
  ndn_Error error;

  if ((error = ndn_TlvEncoder_writeNestedTlv
       (encoder, ndn_Tlv_Encrypt_WhiteIntervalList,
        encodeRepetitiveIntervalListValue, &schedule.whiteIntervalList_, 0)))
    return error;
  if ((error = ndn_TlvEncoder_writeNestedTlv
       (encoder, ndn_Tlv_Encrypt_BlackIntervalList,
        encodeRepetitiveIntervalListValue, &schedule.blackIntervalList_, 0)))
    return error;

  return NDN_ERROR_success;
}

ptr_lib::shared_ptr<RepetitiveInterval>
Schedule::decodeRepetitiveInterval(ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t endOffset;
  ndn_Blob startDate;
  ndn_Blob endDate;
  uint64_t intervalStartHour;
  uint64_t intervalEndHour;
  uint64_t nRepeats;
  uint64_t repeatUnit;

  if ((error = ndn_TlvDecoder_readNestedTlvsStart
       (decoder, ndn_Tlv_Encrypt_RepetitiveInterval, &endOffset)))
    throw runtime_error(ndn_getErrorString(error));

  if ((error = ndn_TlvDecoder_readBlobTlv
       (decoder, ndn_Tlv_Encrypt_StartDate, &startDate)))
    throw runtime_error(ndn_getErrorString(error));
  if ((error = ndn_TlvDecoder_readBlobTlv
       (decoder, ndn_Tlv_Encrypt_EndDate, &endDate)))
    throw runtime_error(ndn_getErrorString(error));
  if ((error = ndn_TlvDecoder_readNonNegativeIntegerTlv
       (decoder, ndn_Tlv_Encrypt_IntervalStartHour, &intervalStartHour)))
    throw runtime_error(ndn_getErrorString(error));
  if ((error = ndn_TlvDecoder_readNonNegativeIntegerTlv
       (decoder, ndn_Tlv_Encrypt_IntervalEndHour, &intervalEndHour)))
    throw runtime_error(ndn_getErrorString(error));
  if ((error = ndn_TlvDecoder_readNonNegativeIntegerTlv
       (decoder, ndn_Tlv_Encrypt_NRepeats, &nRepeats)))
    throw runtime_error(ndn_getErrorString(error));
  if ((error = ndn_TlvDecoder_readNonNegativeIntegerTlv
       (decoder, ndn_Tlv_Encrypt_RepeatUnit, &repeatUnit)))
    throw runtime_error(ndn_getErrorString(error));

  if ((error = ndn_TlvDecoder_finishNestedTlvs(decoder, endOffset)))
    throw runtime_error(ndn_getErrorString(error));

  // Use Blob to make a string.
  // The RepeatUnit enum has the same values as the encoding.
  return ptr_lib::make_shared<RepetitiveInterval>
    (fromIsoString(Blob(startDate).toRawStr()),
     fromIsoString(Blob(endDate).toRawStr()), intervalStartHour,
     intervalEndHour, nRepeats, (RepetitiveInterval::RepeatUnit)repeatUnit);
}

void
Schedule::calculateIntervalResult
  (const vector<ptr_lib::shared_ptr<RepetitiveInterval> >& list,
   MillisecondsSince1970 timeStamp, Interval& positiveResult,
   Interval& negativeResult)
{
  for (size_t i = 0; i < list.size(); ++i) {
    const RepetitiveInterval& element = *list[i];

    RepetitiveInterval::Result result = element.getInterval(timeStamp);
    Interval tempInterval = result.interval;
    if (result.isPositive == true) {
      try {
        positiveResult.unionWith(tempInterval);
      } catch (const Interval::Error& ex) {
        // We don't expect to get this error.
        throw runtime_error("Error in Interval.unionWith: " + ex.Msg());
      }
    }
    else {
      if (!negativeResult.isValid())
        negativeResult.set(tempInterval);
      else
        negativeResult.intersectWith(tempInterval);
    }
  }
}

}
