/*
 * Copyright (c) 2005, 2025, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef OS_LINUX_GLOBALS_LINUX_HPP
#define OS_LINUX_GLOBALS_LINUX_HPP

//
// Declare Linux specific flags. They are not available on other platforms.
//
#define RUNTIME_OS_FLAGS(develop,                                       \
                         develop_pd,                                    \
                         product,                                       \
                         product_pd,                                    \
                         range,                                         \
                         constraint)                                    \
                                                                        \
  product(bool, UseTransparentHugePages, false,                         \
          "Use MADV_HUGEPAGE for large pages")                          \
                                                                        \
  product(bool, LoadExecStackDllInVMThread, true,                       \
          "Load DLLs with executable-stack attribute in the VM Thread") \
                                                                        \
  product(bool, UseContainerSupport, true,                              \
          "Enable detection and runtime container configuration support") \
                                                                        \
  product(bool, AdjustStackSizeForTLS, false,                           \
          "Increase the thread stack size to include space for glibc "  \
          "static thread-local storage (TLS) if true")                  \
                                                                        \
  product(bool, DumpPrivateMappingsInCore, true, DIAGNOSTIC,            \
          "If true, sets bit 2 of /proc/PID/coredump_filter, thus "     \
          "resulting in file-backed private mappings of the process to "\
          "be dumped into the corefile.")                               \
                                                                        \
  product(bool, DumpSharedMappingsInCore, true, DIAGNOSTIC,             \
          "If true, sets bit 3 of /proc/PID/coredump_filter, thus "     \
          "resulting in file-backed shared mappings of the process to " \
          "be dumped into the corefile.")                               \
                                                                        \
  product(bool, UseCpuAllocPath, false, DIAGNOSTIC,                     \
          "Use CPU_ALLOC code path in os::active_processor_count ")     \
                                                                        \
  product(bool, DumpPerfMapAtExit, false, DIAGNOSTIC,                   \
          "Write map file for Linux perf tool at exit")                 \
                                                                        \
  product(intx, TimerSlack, -1, EXPERIMENTAL,                           \
          "Overrides the timer slack value to the given number of "     \
          "nanoseconds. Lower value provides more accurate "            \
          "high-precision timers, at the expense of (possibly) worse "  \
          "power efficiency. In current Linux, 0 means using the "      \
          "system-wide default, which would disable the override, but " \
          "VM would still print the current timer slack values. Use -1 "\
          "to disable both the override and the printouts."             \
          "See prctl(PR_SET_TIMERSLACK) for more info.")                \
                                                                        \
  product(bool, THPStackMitigation, true, DIAGNOSTIC,                   \
          "If THPs are unconditionally enabled on the system (mode "    \
          "\"always\"), the JVM will prevent THP from forming in "      \
          "thread stacks. When disabled, the absence of this mitigation"\
          "allows THPs to form in thread stacks.")                      \
                                                                        \
  develop(bool, DelayThreadStartALot, false,                            \
          "Artificially delay thread starts randomly for testing.")     \
                                                                        \
  product(bool, UseMadvPopulateWrite, true, DIAGNOSTIC,                 \
          "Use MADV_POPULATE_WRITE in os::pd_pretouch_memory.")         \
                                                                        \
  product(bool, PrintMemoryMapAtExit, false, DIAGNOSTIC,                \
          "Print an annotated memory map at exit")                      \
                                                                        \
// end of RUNTIME_OS_FLAGS

//
// Defines Linux-specific default values. The flags are available on all
// platforms, but they may have different default values on other platforms.
//
define_pd_global(size_t, PreTouchParallelChunkSize, 4 * M);
define_pd_global(bool, UseLargePages, false);
define_pd_global(bool, UseLargePagesIndividualAllocation, false);
define_pd_global(bool, UseThreadPriorities, true) ;

#endif // OS_LINUX_GLOBALS_LINUX_HPP
