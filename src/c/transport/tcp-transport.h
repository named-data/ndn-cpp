/*
 * Copyright (C) 2013-2015 Regents of the University of California.
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

#ifndef NDN_TCP_TRANSPORT_H
#define NDN_TCP_TRANSPORT_H

#include "socket-transport.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the ndn_TcpTransport struct with default values for no connection 
 * yet and to use the given buffer for the ElementReader. Note that
 * the ElementReader is not valid until you call ndn_TcpTransport_connect.
 * @param self A pointer to the ndn_TcpTransport struct.
 * @param buffer the allocated buffer used by ElementReader. If reallocFunction
 * is 0, this should be large enough to save a full element, perhaps
 * MAX_NDN_PACKET_SIZE bytes.
 * @param bufferLength the length of the buffer.
 * @param reallocFunction see ndn_DynamicUInt8Array_ensureLength. This may be 0.
 */
static __inline void ndn_TcpTransport_initialize
  (struct ndn_TcpTransport *self, uint8_t *buffer, size_t bufferLength,
   ndn_ReallocFunction reallocFunction)
{
  ndn_SocketTransport_initialize
    (&self->base, buffer, bufferLength, reallocFunction);
}

/**
 * Connect with TCP to the host:port.
 * @param self A pointer to the ndn_TcpTransport struct.
 * @param host The host to connect to.
 * @param port The port to connect to.
 * @param elementListener A pointer to the ndn_ElementListener used by
 * ndn_SocketTransport_processEvents, which remain valid during the life of this
 * object or until replaced by the next call to connect.
 * @return 0 for success, else an error code.
 */
static __inline ndn_Error ndn_TcpTransport_connect
  (struct ndn_TcpTransport *self, char *host, unsigned short port,
   struct ndn_ElementListener *elementListener)
{
  return ndn_SocketTransport_connect
    (&self->base, SOCKET_TCP, host, port, elementListener);
}

/**
 * Send data to the socket.
 * @param self A pointer to the ndn_TcpTransport struct.
 * @param data A pointer to the buffer of data to send.
 * @param dataLength The number of bytes in data.
 * @return 0 for success, else an error code.
 */
static inline ndn_Error ndn_TcpTransport_send
  (struct ndn_TcpTransport *self, const uint8_t *data, size_t dataLength)
{
  return ndn_SocketTransport_send(&self->base, data, dataLength);
}

/**
 * Process any data to receive.  For each element received, call
 * (*elementListener->onReceivedElement)(element, elementLength) for the
 * elementListener in the elementReader given to connect(). This is non-blocking
 * and will return immediately if there is no data to receive.
 * @param self A pointer to the ndn_TcpTransport struct.
 * @param buffer A pointer to a buffer for receiving data. Note that this is
 * only for temporary use and is not the way that this function supplies data.
 * It supplies the data by calling the onReceivedElement callback.
 * @param bufferLength The size of buffer. The buffer should be as large as
 * resources permit up to MAX_NDN_PACKET_SIZE, but smaller sizes will work
 * however may be less efficient due to multiple calls to socket receive and
 * more processing by the ElementReader.
 * @return 0 for success, else an error code.
 */
static inline ndn_Error
ndn_TcpTransport_processEvents
  (struct ndn_TcpTransport *self, uint8_t *buffer, size_t bufferLength)
{
  return ndn_SocketTransport_processEvents(&self->base, buffer, bufferLength);
}

/**
 * Close the socket.
 * @param self A pointer to the ndn_TcpTransport struct.
 * @return 0 for success, else an error code.
 */
static inline ndn_Error ndn_TcpTransport_close(struct ndn_TcpTransport *self)
{
  return ndn_SocketTransport_close(&self->base);
}

#ifdef __cplusplus
}
#endif

#endif
