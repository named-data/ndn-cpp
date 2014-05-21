/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#ifndef NDN_SOCKETTRANSPORT_H
#define NDN_SOCKETTRANSPORT_H

#include <sys/socket.h>
#include <ndn-cpp/c/common.h>
#include "../errors.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  SOCKET_TCP,
  SOCKET_UDP
} ndn_SocketType;
  
struct ndn_SocketTransport {
  int socketDescriptor; /**< -1 if not connected */
};
  
/**
 * Initialize the ndn_SocketTransport struct with default values for no connection yet.
 * @param self A pointer to the ndn_SocketTransport struct.
 */
static inline void ndn_SocketTransport_initialize(struct ndn_SocketTransport *self)
{
  self->socketDescriptor = -1;
}

/**
 * Connect with TCP or UDP to the host:port.
 * @param self A pointer to the ndn_SocketTransport struct.
 * @param socketType SOCKET_TCP or SOCKET_UDP.
 * @param host The host to connect to.
 * @param port The port to connect to.
 * @return 0 for success, else an error code.
 */
ndn_Error ndn_SocketTransport_connect(struct ndn_SocketTransport *self, ndn_SocketType socketType, char *host, unsigned short port);

/**
 * Send data to the socket.
 * @param self A pointer to the ndn_SocketTransport struct.
 * @param data A pointer to the buffer of data to send.
 * @param dataLength The number of bytes in data.
 * @return 0 for success, else an error code.
 */
ndn_Error ndn_SocketTransport_send(struct ndn_SocketTransport *self, const uint8_t *data, size_t dataLength);

/**
 * Check if there is data ready on the socket to be received with ndn_SocketTransport_receive.
 * This does not block, and returns immediately.
 * @param self A pointer to the ndn_SocketTransport struct.
 * @param receiveIsReady This will be set to 1 if data is ready, 0 if not.
 * @return 0 for success, else an error code.
 */
ndn_Error ndn_SocketTransport_receiveIsReady(struct ndn_SocketTransport *self, int *receiveIsReady);

/**
 * Receive data from the socket.  NOTE: This is a blocking call.  You should first call ndn_SocketTransport_receiveIsReady
 * to make sure there is data ready to receive.
 * @param self A pointer to the ndn_SocketTransport struct.
 * @param buffer A pointer to the buffer to receive the data.
 * @param bufferLength The maximum length of buffer.
 * @param nBytes Return the number of bytes received into buffer.
 * @return 0 for success, else an error code.
 */
ndn_Error ndn_SocketTransport_receive
  (struct ndn_SocketTransport *self, uint8_t *buffer, size_t bufferLength, size_t *nBytes);

/**
 * Close the socket.
 * @param self A pointer to the ndn_SocketTransport struct.
 * @return 0 for success, else an error code.
 */
ndn_Error ndn_SocketTransport_close(struct ndn_SocketTransport *self);

#ifdef __cplusplus
}
#endif

#endif
