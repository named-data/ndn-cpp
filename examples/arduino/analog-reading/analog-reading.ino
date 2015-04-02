/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015 Regents of the University of California.
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

// Note: To compile this sketch, you must fix NDN_CPP_ROOT in ndn_cpp_root.h .
/** 
 * This Arduino (Yun) sketch registers the prefix /testarduino/voltage. When it
 * receives an interest it returns a data packet where the name is 
 * appeneded with the reading number and the content is the decimal 
 * reading of analogRead(0).
 * This communicates using the NDN-CPP Lite class ArduinoYunTcpTransportLite.
 * This works with examples/arduino/analog-reading-consumer.cpp .
 */

#ifdef ARDUINO

#include <Bridge.h>
#include <ndn-cpp/lite/transport/arduino-yun-tcp-transport-lite.hpp>
#include <ndn-cpp/lite/data-lite.hpp>
#include <ndn-cpp/lite/interest-lite.hpp>
#include <ndn-cpp/lite/encoding/tlv-0_1_1-wire-format-lite.hpp>
// TODO: Make an API for these.
extern "C" {
void ndn_digestSha256(const uint8_t *data, size_t dataLength, uint8_t *digest);
void ndn_computeHmacWithSha256
  (const uint8_t *key, size_t keyLength, const uint8_t *data, size_t dataLength, 
   uint8_t *digest);
void ndn_generateRandomBytes(uint8_t *buffer, size_t bufferLength);
}

using namespace ndn;

static void
onReceivedElement(ndn::ElementListenerLite* self, uint8_t *element, size_t elementLength);

// Set up the transport to call onReceivedElement.
// elementBufferBytes is big enough for an interest packet.
uint8_t elementBufferBytes[60];
DynamicUInt8ArrayLite elementBuffer(elementBufferBytes, sizeof(elementBufferBytes), 0);
ElementListenerLite elementListener(onReceivedElement);
ArduinoYunTcpTransportLite transport(elementBuffer);

uint8_t hmacKey[64];
uint8_t hmacKeyDigest[ndn_SHA256_DIGEST_SIZE];

// We will increment readingNumber to put in the data packet name.
int readingNumber = 0;

// Initialize prefixComponents in setup().
ndn_NameComponent prefixComponents[2];
NameLite prefix(prefixComponents, sizeof(prefixComponents));

/** 
 * Decode the element as an interest and check the prefix. 
 */
static ndn_Error
replyToInterest(uint8_t *element, size_t elementLength)
{
  // Decode the element as an InterestLite.
  ndn_NameComponent interestNameComponents[3];
  struct ndn_ExcludeEntry excludeEntries[2];
  InterestLite interest
    (interestNameComponents, sizeof(interestNameComponents) / sizeof(interestNameComponents[0]), 
     excludeEntries, sizeof(excludeEntries) / sizeof(excludeEntries[0]), 0, 0);
  size_t signedPortionBeginOffset, signedPortionEndOffset;
  ndn_Error error;
  if ((error = Tlv0_1_1WireFormatLite::decodeInterest
       (interest, element, elementLength, &signedPortionBeginOffset, 
        &signedPortionEndOffset)))
    return error;
  
  // We expect the interest name to be /testarduino/voltage. We will append the reading number
  // component in the response data packet, but don't accept it here since we don't return
  // prefiously-generated readings.
  if (!interest.getName().equals(prefix))
    // Ignore an unexpected prefix.
    return NDN_ERROR_success;

  // Create the response data packet.
  ndn_NameComponent dataNameComponents[3];
  DataLite data(dataNameComponents, sizeof(dataNameComponents) / sizeof(dataNameComponents[0]), 0, 0);
  data.setName(interest.getName());
  // Append the reading number to the name.
  ++readingNumber;
  char readingNumberBuffer[12];
  itoa(readingNumber, readingNumberBuffer, 10);
  data.getName().append(BlobLite((const uint8_t*)readingNumberBuffer, strlen(readingNumberBuffer)));
  // Set the content to an analog reading.
  int reading = analogRead(0);
  char contentBuffer[12];
  itoa(reading, contentBuffer, 10);
  data.setContent(BlobLite((const uint8_t*)contentBuffer, strlen(contentBuffer)));
  
  // Set up the signature with the hmacKeyDigest key locator digest.
  // TODO: Change to ndn_SignatureType_HmacWithSha256Signature.
  data.getSignature().setType(ndn_SignatureType_Sha256WithRsaSignature);
  data.getSignature().getKeyLocator().setType(ndn_KeyLocatorType_KEY_LOCATOR_DIGEST);
  data.getSignature().getKeyLocator().setKeyData(BlobLite(hmacKeyDigest, sizeof(hmacKeyDigest)));
  
  // Encode once to get the signed portion.
  uint8_t encoding[120];
  DynamicUInt8ArrayLite output(encoding, sizeof(encoding), 0);
  size_t encodingLength;
  if ((error = Tlv0_1_1WireFormatLite::encodeData
       (data, &signedPortionBeginOffset, &signedPortionEndOffset, 
	output, &encodingLength)))
    return error;

  // Get the signature for the signed portion.
  uint8_t signatureValue[ndn_SHA256_DIGEST_SIZE];
  ndn_computeHmacWithSha256
    (hmacKey, sizeof(hmacKey), encoding + signedPortionBeginOffset,
     signedPortionEndOffset - signedPortionBeginOffset, signatureValue);
  data.getSignature().setSignature(BlobLite(signatureValue, ndn_SHA256_DIGEST_SIZE));
  
  // Encode again to include the signature.
  if ((error = Tlv0_1_1WireFormatLite::encodeData
       (data, &signedPortionBeginOffset, &signedPortionEndOffset, 
	output, &encodingLength)))
    return error;

  transport.send(encoding, encodingLength);  
  Serial.print("Sent reading #");
  Serial.println(readingNumberBuffer);

  return NDN_ERROR_success;
}

static void
onReceivedElement(ndn::ElementListenerLite* self, uint8_t *element, size_t elementLength)
{
  const int interestTlvType = 5;
  if (element[0] != interestTlvType)
    // We only want an interest.
    return;

  ndn_Error error;
  if ((error = replyToInterest(element, elementLength))) {
    Serial.print("OnInterest error "); 
    Serial.println((int)error);
    return;
  }
}

/** 
 * The Arduino calls this first.
 */
void setup() {
  Bridge.begin();
  Serial.begin(57600);
  while (!Serial); // Wait untilSerial is ready.
  
  // Set the expected prefix.
  prefix.append("testarduino");
  prefix.append("voltage");
  
  // Set hmacKey to a default test value.
  memset(hmacKey, 0, sizeof(hmacKey));
  // Set hmacKeyDigest to sha256(hmacKey).
  ndn_digestSha256(hmacKey, sizeof(hmacKey), hmacKeyDigest);

  // Connect the transport to the NFD host. If NFD is remote, it must be configured
  // to accept localhop connections.
  const char* host = "88.5.44.216";
  ndn_Error error;
  if ((error = transport.connect(host, 6363, elementListener))) {
    Serial.println(error);
    return;
  }
  Serial.println("connected");

  // This is a hard-wired remote (localhop) registration request for /testarduino/voltage.
  uint8_t REGISTER_INTEREST_ENCODING[] {
    0x05, 0x6b, 0x07, 0x5b, 0x08, 0x08, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f, 0x70, 0x08,
    0x03, 0x6e, 0x66, 0x64, 0x08, 0x03, 0x72, 0x69, 0x62, 0x08, 0x08, 0x72, 0x65, 0x67, 0x69, 0x73,
    0x74, 0x65, 0x72, 0x08, 0x1a, 0x68, 0x18, 0x07, 0x16, 0x08, 0x0b, 0x74, 0x65, 0x73, 0x74, 0x61,
    0x72, 0x64, 0x75, 0x69, 0x6e, 0x6f, 0x08, 0x07, 0x76, 0x6f, 0x6c, 0x74, 0x61, 0x67, 0x65, 0x08,
    0x08, 0x00, 0x00, 0x01, 0x4c, 0x76, 0x66, 0x83, 0xc5, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x08, 0x09, 0x16, 0x07, 0x1b, 0x01, 0x01, 0x1c, 0x02, 0x07, 0x00, 0x08, 0x00,
    0x09, 0x02, 0x12, 0x00, 0x0a, 0x04, 0xbb, 0x27, 0x86, 0x2e, 0x0c, 0x02, 0x07, 0xd0
  };
  // Update the command nonce name component.
  const size_t commandNonceOffset = 74;
  const size_t commandNonceLength = 8;
  ndn_generateRandomBytes(REGISTER_INTEREST_ENCODING + commandNonceOffset, commandNonceLength);
  
  // Send the registration require. We don't decode the response data packet because it's too big.
  transport.send(REGISTER_INTEREST_ENCODING, sizeof(REGISTER_INTEREST_ENCODING));
}

/**
 * The Arduino calls this repeatedly. Call processEvents.
 */
void loop() {
  ndn_Error error;
  uint8_t buffer[50];
  error = transport.processEvents(buffer, sizeof(buffer));
  if (error == NDN_ERROR_DynamicUInt8Array_realloc_function_pointer_not_supplied) {
    // For publish, we expect this error for the large register response data packet.
    // To save stack space, elementBufferBytes is only big enough for the interest packet.
  }
  else if (error) {
    Serial.print("processEvents "); 
    Serial.println(error);
    return;
  }
}

#endif // ARDUINO
