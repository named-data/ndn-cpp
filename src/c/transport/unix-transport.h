/*
 * Copyright (C) 2014 Regents of the University of California.
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

#ifndef NDN_UNIX_TRANSPORT_H
#define NDN_UNIX_TRANSPORT_H

#include "socket-transport.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ndn_UnixTransport {
  struct ndn_SocketTransport base;
};

/**
 * Initialize the ndn_UnixTransport struct with default values for no connection yet.
 * @param self A pointer to the ndn_UnixTransport struct.
 */
static inline void ndn_UnixTransport_initialize(struct ndn_UnixTransport *self)
{
  ndn_SocketTransport_initialize(&self->base);
}

/**
 * Connect with a Unix Socket to the socket filePath.
 * @param self A pointer to the ndn_UnixTransport struct.
 * @param filePath The file path of the Unix socket to connect to.
 * @return 0 for success, else an error code.
 */
static inline ndn_Error ndn_UnixTransport_connect
  (struct ndn_UnixTransport *self, char *filePath)
{
  return ndn_SocketTransport_connect(&self->base, SOCKET_UNIX, filePath, 0);
}

/**
 * Send data to the socket.
 * @param self A pointer to the ndn_UnixTransport struct.
 * @param data A pointer to the buffer of data to send.
 * @param dataLength The number of bytes in data.
 * @return 0 for success, else an error code.
 */
static inline ndn_Error ndn_UnixTransport_send
  (struct ndn_UnixTransport *self, const uint8_t *data, size_t dataLength)
{
  return ndn_SocketTransport_send(&self->base, data, dataLength);
}

/**
 * Check if there is data ready on the socket to be received with
 * ndn_UnixTransport_receive. This does not block, and returns immediately.
 * @param self A pointer to the ndn_UnixTransport struct.
 * @param receiveIsReady This will be set to 1 if data is ready, 0 if not.
 * @return 0 for success, else an error code.
 */
static inline ndn_Error ndn_UnixTransport_receiveIsReady
  (struct ndn_UnixTransport *self, int *receiveIsReady)
{
  return ndn_SocketTransport_receiveIsReady(&self->base, receiveIsReady);
}

/**
 * Receive data from the socket.  NOTE: This is a blocking call.
 * You should first call ndn_SocketTransport_receiveIsReady to make sure there
 * is data ready to receive.
 * @param self A pointer to the ndn_UnixTransport struct.
 * @param buffer A pointer to the buffer to receive the data.
 * @param bufferLength The maximum length of buffer.
 * @param nBytes Return the number of bytes received into buffer.
 * @return 0 for success, else an error code.
 */
static inline ndn_Error ndn_UnixTransport_receive
  (struct ndn_UnixTransport *self, uint8_t *buffer, size_t bufferLength,
   size_t *nBytes)
{
  return ndn_SocketTransport_receive(&self->base, buffer, bufferLength, nBytes);
}

/**
 * Close the socket.
 * @param self A pointer to the ndn_UnixTransport struct.
 * @return 0 for success, else an error code.
 */
static inline ndn_Error ndn_UnixTransport_close(struct ndn_UnixTransport *self)
{
  return ndn_SocketTransport_close(&self->base);
}

#ifdef __cplusplus
}
#endif

#endif
