/**
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

#include "forwarding-entry.h"
#include "control-parameters.h"
#include "forwarding-flags-impl.h"

void ndn_ForwardingFlags_initialize(struct ndn_ForwardingFlags *self)
{
  self->active = 1;
  self->childInherit = 1;
  self->advertise = 0;
  self->last = 0;
  self->capture = 0;
  self->local = 0;
  self->tap = 0;
  self->captureOk = 0;
}

int ndn_ForwardingFlags_getForwardingEntryFlags(const struct ndn_ForwardingFlags *self)
{
  int result = 0;

  if (self->active)
    result |= ndn_ForwardingEntryFlags_ACTIVE;
  if (self->childInherit)
    result |= ndn_ForwardingEntryFlags_CHILD_INHERIT;
  if (self->advertise)
    result |= ndn_ForwardingEntryFlags_ADVERTISE;
  if (self->last)
    result |= ndn_ForwardingEntryFlags_LAST;
  if (self->capture)
    result |= ndn_ForwardingEntryFlags_CAPTURE;
  if (self->local)
    result |= ndn_ForwardingEntryFlags_LOCAL;
  if (self->tap)
    result |= ndn_ForwardingEntryFlags_TAP;
  if (self->captureOk)
    result |= ndn_ForwardingEntryFlags_CAPTURE_OK;

  return result;
}

void ndn_ForwardingFlags_setForwardingEntryFlags(struct ndn_ForwardingFlags *self, int forwardingEntryFlags)
{
  self->active = (forwardingEntryFlags & ndn_ForwardingEntryFlags_ACTIVE) ? 1 : 0;
  self->childInherit = (forwardingEntryFlags & ndn_ForwardingEntryFlags_CHILD_INHERIT) ? 1 : 0;
  self->advertise = (forwardingEntryFlags & ndn_ForwardingEntryFlags_ADVERTISE) ? 1 : 0;
  self->last = (forwardingEntryFlags & ndn_ForwardingEntryFlags_LAST) ? 1 : 0;
  self->capture = (forwardingEntryFlags & ndn_ForwardingEntryFlags_CAPTURE) ? 1 : 0;
  self->local = (forwardingEntryFlags & ndn_ForwardingEntryFlags_LOCAL) ? 1 : 0;
  self->tap = (forwardingEntryFlags & ndn_ForwardingEntryFlags_TAP) ? 1 : 0;
  self->captureOk = (forwardingEntryFlags & ndn_ForwardingEntryFlags_CAPTURE_OK) ? 1 : 0;
}

int
ndn_ForwardingFlags_getNfdForwardingFlags(const struct ndn_ForwardingFlags *self)
{
  int result = 0;

  if (self->childInherit)
    result |= ndn_NfdForwardingFlags_CHILD_INHERIT;
  if (self->capture)
    result |= ndn_NfdForwardingFlags_CAPTURE;

  return result;
}

void
ndn_ForwardingFlags_setNfdForwardingFlags
  (struct ndn_ForwardingFlags *self, int nfdForwardingFlags)
{
  self->childInherit = (nfdForwardingFlags & ndn_NfdForwardingFlags_CHILD_INHERIT) ? 1 : 0;
  self->capture = (nfdForwardingFlags & ndn_NfdForwardingFlags_CAPTURE) ? 1 : 0;
}
