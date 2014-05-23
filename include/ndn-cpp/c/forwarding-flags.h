/**
 * Copyright (C) 2013-2014 Regents of the University of California.
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#ifndef NDN_FORWARDING_FLAGS_H
#define NDN_FORWARDING_FLAGS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An ndn_ForwardingFlags object holds the flags which specify how the forwarding daemon should forward an interest for
 * a registered prefix.  We use a separate ForwardingFlags object to retain future compatibility if the daemon forwarding
 * bits are changed, amended or deprecated.
 */
struct ndn_ForwardingFlags {
  int active; /**< 1 if the flag is set, 0 if cleared. */
  int childInherit;
  int advertise;
  int last;
  int capture;
  int local;
  int tap;
  int captureOk;  
};

/**
 * Initialize an ndn_ForwardingFlags struct with the default with "active" and "childInherit" set and all other flags cleared.
 * @param self A pointer to the ndn_ForwardingFlags struct.
 */
void ndn_ForwardingFlags_initialize(struct ndn_ForwardingFlags *self);

/**
 * Get an integer with the bits set according to the flags as used by the ForwardingEntry message.
 * @param self A pointer to the ndn_ForwardingFlags struct.
 * @return An integer with the bits set.
 */
int ndn_ForwardingFlags_getForwardingEntryFlags(struct ndn_ForwardingFlags *self);

/**
 * Set the flags according to the bits in forwardingEntryFlags as used by the ForwardingEntry message.
 * @param self A pointer to the ndn_ForwardingFlags struct.
 * @param flags An integer with the bits set.
 */
void ndn_ForwardingFlags_setForwardingEntryFlags(struct ndn_ForwardingFlags *self, int forwardingEntryFlags);

/**
 * Get an integer with the bits set according to the NFD forwarding flags as 
 * used in the ControlParameters of the command interest.
 * @param self A pointer to the ndn_ForwardingFlags struct.
 * @return An integer with the bits set.
 */
int ndn_ForwardingFlags_getNfdForwardingFlags(struct ndn_ForwardingFlags *self);

/**
 * Set the flags according to the NFD forwarding flags as used in the 
 * ControlParameters of the command interest.
 * @param self A pointer to the ndn_ForwardingFlags struct.
 * @param nfdForwardingFlags An integer with the bits set.
 */
void ndn_ForwardingFlags_setNfdForwardingFlags
  (struct ndn_ForwardingFlags *self, int nfdForwardingFlags);

#ifdef __cplusplus
}
#endif

#endif
