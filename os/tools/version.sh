#!/bin/bash
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
# version.sh
#
#   Copyright (C) 2011 Gregory Nutt. All rights reserved.
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

WD=`pwd`

# Get command line parameters

USAGE="USAGE: $0 [-d|-h] [-b <build>] -v <major.minor> <outfile-path>"
ADVICE="Try '$0 -h' for more information"

unset VERSION
unset BUILD
unset OUTFILE

while [ ! -z "$1" ]; do
	case $1 in
	-b )
		shift
		BUILD=$1
		;;
	-d )
		set -x
		;;
	-v )
		shift
		VERSION=$1
		;;
	-h )
		echo "$0 is a tool for generation of proper version files for the TinyAra build"
		echo ""
		echo $USAGE
		echo ""
		echo "Where:"
		echo "	-b <build>"
		echo "		Use this build identification string.  Default: use GIT build ID"
		echo "		NOTE: GIT build information may not be available in a snapshot"
		echo "	-d"
		echo "		Enable script debug"
		echo "	-h"
		echo "		show this help message and exit"
		echo "	-v <major.minor>"
		echo "		The TinyAra version number expressed as a major and minor number separated"
		echo "		by a period"
		echo " 	<outfile-path>"
		echo "		The full path to the version file to be created"
		exit 0
		;;
	* )
		break;
		;;
	esac
	shift
done

OUTFILE=$1

# Make sure we know what is going on

if [ -z ${VERSION} ] ; then
	echo "Missing versioning information"
	echo $USAGE
	echo $ADVICE
	exit 1
fi

if [ -z ${OUTFILE} ] ; then
	echo "Missing path to the output file"
	echo $USAGE
	echo $ADVICE
	exit 1
fi

# Get the major and minor version numbers

MAJOR=`echo ${VERSION} | cut -d'.' -f1`
if [ "X${MAJOR}" = "X${VERSION}" ]; then
	echo "Missing minor version number"
	echo $USAGE
	echo $ADVICE
	exit 2
fi
MINOR=`echo ${VERSION} | cut -d'.' -f2`

# Get GIT information (if not provided on the command line)

if [ -z "${BUILD}" ]; then
	GITINFO=`git log 2>/dev/null | head -1`
	if [ -z "${GITINFO}" ]; then
		echo "GIT version information is not available"
		BUILD=NA
	else
		BUILD=`echo ${GITINFO} | cut -d' ' -f2 | cut -c -7`
		if [ -z "${BUILD}" ]; then
			echo "GIT build information not found"
			BUILD=NA
		fi
	fi
fi

# Write a version file into the TinyAra directoy. The syntax of file is
# such that it may be sourced by a bash script or included by a Makefile.

echo "#!/bin/bash" >${OUTFILE}
echo "" >>${OUTFILE}
echo "CONFIG_VERSION_STRING=\"${VERSION}\"" >>${OUTFILE}
echo "CONFIG_VERSION_MAJOR=${MAJOR}" >>${OUTFILE}
echo "CONFIG_VERSION_MINOR=${MINOR}" >>${OUTFILE}
echo "CONFIG_VERSION_BUILD=\"${BUILD}\"" >>${OUTFILE}
echo "CONFIG_VERSION_BUILD_USER=\"`whoami`@`hostname`\"" >>${OUTFILE}
echo "CONFIG_VERSION_BUILD_TIME=\"`date '+%Y-%m-%d %T %Z'`\"" >>${OUTFILE}
