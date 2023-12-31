
/****************************************************************************
 *
 * Copyright 2020 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/****************************************************************************
 * netutils/netlib/netlib_setipv6type.c
 *
 *   Copyright (C) 2007-2009, 2011-2012 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>
#if defined(CONFIG_NET_IPv6) && CONFIG_NSOCKET_DESCRIPTORS > 0

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netutils/netlib.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: netlib_set_ipv6type
 *
 * Description:
 *   Set the network driver IPv6 address type
 *
 * Parameters:
 *   ifname   The name of the interface to use
 *   type     The type of address
 *              0 : EUI64
 *              1 : Pseudo Random Function(RFC 7217)
 *              2 : Use Mac address
 *
 * Return:
 *   0 on success; -1 on failure
 *
 ****************************************************************************/

int netlib_set_ipv6type(FAR const char *ifname, uint8_t type)
{
	int ret = ERROR;

	if (ifname) {
		/* Get a socket (only so that we get access to the INET subsystem) */

		int sockfd = socket(PF_INET6, NETLIB_SOCK_IOCTL, 0);
		if (sockfd >= 0) {
			struct lifreq req;
			memset(&req, 0, sizeof(struct lifreq));

			/* Put the driver name into the request */

			strncpy(req.lifr_name, ifname, IFNAMSIZ);

			/* Perform the ioctl to ipv6 ip type */

			req.lifr_addr_type = type;
			ret = ioctl(sockfd, SIOCSLIPTYPE, (unsigned long)&req);
			close(sockfd);
		}
	}

	return ret;
}

#endif
