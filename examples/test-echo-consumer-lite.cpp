/**
 * Copyright (C) 2014-2017 Regents of the University of California.
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

/**
 * This prompts for a <word>, sends an interest to "localhost" for
 * /testecho/<word> , then displays the received Data packet. This needs NFD
 * running on the local host. The works with test-publish-async-nfd or
 * test-publish-async-nfd-lite .
 * This uses the API for NDN-CPP Lite instead of the full NDN-CPP. Note that
 * this does not use the C++ Standard Library, or exceptions, or virtual
 * methods or malloc (except in the sendInterest functions which you can
 * change to not use malloc). Therefore the lightweight C++ code maps
 * directly onto the inner pure C functions, but has advantages of C++ such as
 * namespaces, inheritance, overloaded methods, "this" pointers, and calling
 * destructors automatically.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ndn-cpp/lite/transport/tcp-transport-lite.hpp>
#include <ndn-cpp/lite/encoding/tlv-0_2-wire-format-lite.hpp>
#include <ndn-cpp/lite/util/dynamic-malloc-uint8-array-lite.hpp>

using namespace ndn;

/**
 * The ElementReceiver class extends ElementListenerLite and has an
 * onReceivedElement to process the incoming Data packet.
 */
class ElementReceiver : public ElementListenerLite {
public:
  /**
   * Create an ElementReceiver.
   * @param originalInterest The original Interest so that onReceivedElement can
   * check if the incoming Data packet matches. This does not make a copy; the
   * referenced object must remain valid.
   */
  ElementReceiver(const InterestLite& originalInterest)
  : ElementListenerLite(&onReceivedElementWrapper),
    originalInterest_(originalInterest),
    callbackCount_(0)
  {
  }

  /**
   * This is called when an entire packet is received. If this is a Data packet
   * for the original Interest that was sent, display its name and content.
   * @param element pointer to the element. This buffer is only valid during
   * this call.  If you need the data later, you must copy.
   * @param elementLength length of element
   */
  void
  onReceivedElement(const uint8_t *element, size_t elementLength);

  int callbackCount_;

private:
  const InterestLite& originalInterest_;

  static void
  onReceivedElementWrapper
    (ElementListenerLite *self, const uint8_t *element, size_t elementLength);
};

void
ElementReceiver::onReceivedElement(const uint8_t *element, size_t elementLength)
{
  if (*element != 6)
    // Not a TLV Data packet.
    return;

  // Reserve space for a large maximum number of name components. If you know
  // your application requires less, you can use a smaller maximum.
  struct ndn_NameComponent nameComponents[100];
  struct ndn_NameComponent keyNameComponents[100];
  DataLite data
    (nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]),
     keyNameComponents, sizeof(keyNameComponents) / sizeof(keyNameComponents[0]));

  size_t signedPortionBeginOffset;
  size_t signedPortionEndOffset;
  ndn_Error error;
  if ((error = Tlv0_2WireFormatLite::decodeData
       (data, element, elementLength, &signedPortionBeginOffset,
        &signedPortionEndOffset))) {
    printf("Error decoding data: %s\n", ndn_getErrorString(error));
    return;
  }

  if (!originalInterest_.getName().match(data.getName()))
    // We got a Data packet that is not for us.
    return;

  ++callbackCount_;
  printf("Got data packet with name ");
  for (size_t iComponent = 0; iComponent < data.getName().size(); ++iComponent) {
    printf("/");
    const NameLite::Component& component = data.getName().get(iComponent);
    for (size_t i = 0; i < component.getValue().size(); ++i)
      printf("%c", (int)component.getValue().buf()[i]);
  }
  printf("\n");

  for (size_t i = 0; i < data.getContent().size(); ++i)
    printf("%c", (int)data.getContent().buf()[i]);
  printf("\n");
}

void
ElementReceiver::onReceivedElementWrapper
  (ElementListenerLite *self, const uint8_t *element, size_t elementLength)
{
  // Simply call the onReceivedElement method.
  ((ElementReceiver*)self)->onReceivedElement(element, elementLength);
}

ndn_Error
sendInterest(const InterestLite& interest, TcpTransportLite& transport);

int
main(int argc, char** argv)
{
  char word[100];
  printf("Enter a word to echo:\n");
  fgets(word, sizeof(word), stdin);

  // Create the Interest.
  struct ndn_NameComponent nameComponents[2];
  InterestLite interest
    (nameComponents, sizeof(nameComponents) / sizeof(nameComponents[0]),
     0, 0, 0, 0);
  interest.getName().append("testecho");
  interest.getName().append(word);
  interest.setInterestLifetimeMilliseconds(4000.0);

  // Create the transport using a buffer which is large enough to receive an
  // entire packet so that we don't have to malloc memory. If the maximum size
  // of an expected packet is smaller then the smaller value can be used.
  // Alternatively, DynamicMallocUInt8ArrayLite can be used if the your platform
  // supports malloc.
  uint8_t elementBufferBytes[MAX_NDN_PACKET_SIZE];
  DynamicUInt8ArrayLite elementBuffer
    (elementBufferBytes, sizeof(elementBufferBytes), 0);
  TcpTransportLite transport(elementBuffer);

  // Set up the ElementReceiver to receive the Data packet and connect.
  ElementReceiver elementReceiver(interest);
  ndn_Error error;
  if ((error = transport.connect("localhost", 6363, elementReceiver))) {
    printf("Error in transport connect: %s\n", ndn_getErrorString(error));
    return error;
  }

  printf("Express name testecho/%s\n", word);

  if ((error = sendInterest(interest, transport))) {
    printf("Error in sendInterest: %s\n", ndn_getErrorString(error));
    return error;
  }

  // The main event loop. 
  // We're not using the Pending Interest Table in the main NDN-CPP Face object,
  // so there is no automatic check for timeout. To keep this example
  // lightweight, we loop forever waiting for the Data packet. To check for
  // timeout, we could use ndn_getNowMilliseconds() to get the start time, then
  // check the elapsed time with ndn_getNowMilliseconds() in the loop.
  while (elementReceiver.callbackCount_ < 1) {
    // processEvents will use whatever buffer size is provided. A larger buffer
    // is more efficient but takes more memory.
    uint8_t buffer[1000];
    if ((error = transport.processEvents(buffer, sizeof(buffer)))) {
      printf("Error in processEvents: %s\n", ndn_getErrorString(error));
      return error;
    }

    // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
    // If your platform doesn't have usleep then you can use another sleep
    // function, or don't sleep at all which could use more CPU.
    ::usleep(10000);
  }

  return 0;
}

/**
 * Encode the interest and send it through the transport.
 * @param interest The Interest to encode and send.
 * @param transport This calls transport.send.
 * @return 0 for success, else an error code.
 */
ndn_Error
sendInterest(const InterestLite& interest, TcpTransportLite& transport)
{
  // Here we use a DynamicMallocUInt8ArrayLite. If your platform doesn't support
  // malloc then set "#if 0" to use DynamicUInt8ArrayLite directly with a
  // fixed-size array.
#if 1
  DynamicMallocUInt8ArrayLite encoding(50);
#else
  // Set this smaller if you expect a smaller max encoding size.
  uint8_t encodingBuffer[MAX_NDN_PACKET_SIZE];
  DynamicUInt8ArrayLite encoding(encodingBuffer, sizeof(encodingBuffer), 0);
#endif

  ndn_Error error;
  size_t encodingLength;
  size_t dummySignedPortionBeginOffset, dummySignedPortionEndOffset;
  if ((error = Tlv0_2WireFormatLite::encodeInterest
       (interest, &dummySignedPortionBeginOffset, &dummySignedPortionEndOffset,
        encoding, &encodingLength)))
    return error;

  if ((error = transport.send(encoding.getArray(), encodingLength)))
    return error;

  return NDN_ERROR_success;
}
