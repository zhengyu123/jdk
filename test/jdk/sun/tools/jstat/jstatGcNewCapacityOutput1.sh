#
# Copyright (c) 2004, 2025, Oracle and/or its affiliates. All rights reserved.
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# This code is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 only, as
# published by the Free Software Foundation.
#
# This code is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# version 2 for more details (a copy is included in the LICENSE file that
# accompanied this code).
#
# You should have received a copy of the GNU General Public License version
# 2 along with this work; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
# or visit www.oracle.com if you need additional information or have any
# questions.
#

# @test
# @bug 4990825
# @run shell jstatGcNewCapacityOutput1.sh
# @summary Test that output of 'jstat -gcnewcapacity 0' has expected line counts

. ${TESTSRC-.}/../../jvmstat/testlibrary/utils.sh

setup
verify_os

JSTAT="${TESTJAVA}/bin/jstat"

${JSTAT} ${COMMON_JSTAT_FLAGS} -gcnewcapacity 0 2>&1 | awk -f ${TESTSRC}/gcNewCapacityOutput1.awk
RC=$?
if [ $RC -ne 0 ]; then
  exit $RC
fi

${JSTAT} ${COMMON_JSTAT_FLAGS} -J-XX:+UseParallelGC -gcnewcapacity 0 2>&1 | awk -f ${TESTSRC}/gcNewCapacityOutput1.awk
