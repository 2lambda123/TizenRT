###########################################################################
#
# Copyright 2016 Samsung Electronics All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
# either express or implied. See the License for the specific
# language governing permissions and limitations under the License.
#
###########################################################################

############################################################################
# net/Makefile
#
#   Copyright (C) 2007, 2008, 2011-2015 Gregory Nutt. All rights reserved.
#   Author: Gregory Nutt <gnutt@nuttx.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name NuttX nor the names of its contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
############################################################################

-include $(TOPDIR)/Make.defs

ifeq ($(CONFIG_NET),y)

# Basic networking support

NET_ASRCS   =
NET_CSRCS   = net_initialize_netmgr.c

#LWIP stack support

LWIP_ASRCS   =
LWIP_CSRCS   =

# Socket support

SOCK_ASRCS  =
SOCK_CSRCS  =

# Support for operations on network devices

NETDEV_ASRCS =
NETDEV_CSRCS =

BLEDEV_ASRCS =
BLEDEV_CSRCS =

BLUETOOTH_ASRCS =
BLUETOOTH_CSRCS =

VPATH =
DEPPATH = --dep-path .

include netmgr/Make.defs

ifeq ($(CONFIG_DRIVERS_BLE),y)
include blemgr/Make.defs
endif

ifeq ($(CONFIG_BLUETOOTH),y)
include bluetooth/Make.defs
include bluetooth/iob/Make.defs
endif
#LWIP Support

ifeq ($(CONFIG_NET_LWIP),y)
include lwip/src/core/Make.defs
include lwip/src/api/Make.defs
include lwip/src/netif/Make.defs
include lwip/src/core/snmp/Make.defs
ifeq ($(CONFIG_NET_IPv4),y)
include lwip/src/core/ipv4/Make.defs
endif
ifeq ($(CONFIG_NET_IPv6),y)
include lwip/src/core/ipv6/Make.defs
endif
include lwip/src/netif/ppp/Make.defs
include lwip/sys/arch/Make.defs
endif
endif

ifeq ($(CONFIG_NET_LOCAL),y)
include local/Make.defs
include utils/Make.defs
endif


ASRCS		= $(SOCK_ASRCS) $(BLEDEV_ASRCS) $(NETDEV_ASRCS) $(NET_ASRCS) $(TLS_ASRCS) $(LWIP_ASRCS) $(BLUETOOTH_ASRCS)
AOBJS		= $(ASRCS:.S=$(OBJEXT))

CSRCS		= $(SOCK_CSRCS) $(BLEDEV_CSRCS) $(NETDEV_CSRCS) $(NET_CSRCS) $(TLS_CSRCS) $(LWIP_CSRCS) $(BLUETOOTH_CSRCS)
COBJS		= $(CSRCS:.c=$(OBJEXT))

SRCS		= $(ASRCS) $(CSRCS)
OBJS		= $(AOBJS) $(COBJS)

BIN		= libnet$(LIBEXT)

all:	$(BIN)
.PHONY: depend clean distclean

$(AOBJS): %$(OBJEXT): %.S
	$(call ASSEMBLE, $<, $@)

$(COBJS): %$(OBJEXT): %.c
	$(call COMPILE, $<, $@)

$(BIN): $(OBJS)
	$(call ARCHIVE, $@, $(OBJS))

.depend: Makefile $(SRCS)
ifeq ($(CONFIG_NET),y)
	$(Q) $(MKDEP) $(DEPPATH) "$(CC)" -- $(CFLAGS) -- $(SRCS) >Make.dep
endif
	$(Q) touch $@

depend: .depend

clean:
	$(call DELFILE, $(BIN))
	$(call CLEAN)

distclean: clean
	$(call DELFILE, Make.dep)
	$(call DELFILE, .depend)

-include Make.dep
