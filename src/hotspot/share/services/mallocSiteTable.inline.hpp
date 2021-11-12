/*
 * Copyright (c) 2021, Red Hat, Inc. All rights reserved.
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

#ifndef SHARE_SERVICES_MALLOCSITETABLE_INLINE_HPP
#define SHARE_SERVICES_MALLOCSITETABLE_INLINE_HPP

#include "utilities/macros.hpp"
#if INCLUDE_NMT

#include "services/mallocSiteTable.hpp"
#include "services/memTracker.hpp"

// Acquire shared lock.
// Return true if shared access is granted.
bool MallocSiteTable::AccessGuard::shared_access() {
  int res = Atomic::add(&_guard, 1, memory_order_relaxed);
  if (res < 0) {
    assert(MemTracker::tracking_level() < NMT_detail, "The transition should be visible");
    Atomic::dec(&_guard, memory_order_relaxed);
    return false;
  }
  assert(MemTracker::tracking_level() == NMT_detail, "The table is only valid in detail level");
  _access_state = SharedAccess;
  return true;
}

bool MallocSiteTable::access_stack(NativeCallStack& stack, size_t bucket_idx, size_t pos_idx) {
  AccessGuard guard;
  if (guard.shared_access()) {
    MallocSite* site = malloc_site(bucket_idx, pos_idx);
    if (site != NULL) {
      stack = *site->call_stack();
      return true;
    }
  }
  return false;
}

bool MallocSiteTable::allocation_at(const NativeCallStack& stack, size_t size,
  size_t* bucket_idx, size_t* pos_idx, MEMFLAGS flags) {
  AccessGuard guard;
  if (guard.shared_access()) {
    MallocSite* site = lookup_or_add(stack, bucket_idx, pos_idx, flags);
    if (site != NULL) site->allocate(size);
    return site != NULL;
  }
  return false;
}

bool MallocSiteTable::deallocation_at(size_t size, size_t bucket_idx, size_t pos_idx) {
  AccessGuard guard;
  if (guard.shared_access()) {
    MallocSite* site = malloc_site(bucket_idx, pos_idx);
    if (site != NULL) {
      site->deallocate(size);
      return true;
    }
  }
  return false;
}

#endif // INCLUDE_NMT
#endif // SHARE_SERVICES_MALLOCSITETABLE_INLINE_HPP
