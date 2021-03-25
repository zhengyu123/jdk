/*
 * Copyright (c) 2017, 2020, Red Hat, Inc. All rights reserved.
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

#include "precompiled.hpp"

#include "gc/shared/stringdedup/stringDedup.hpp"
#include "gc/shared/workgroup.hpp"
#include "gc/shenandoah/shenandoahCollectionSet.inline.hpp"
#include "gc/shenandoah/shenandoahHeap.inline.hpp"
#include "gc/shenandoah/shenandoahMarkingContext.inline.hpp"
#include "gc/shenandoah/shenandoahPhaseTimings.hpp"
#include "gc/shenandoah/shenandoahStringDedup.hpp"
#include "gc/shenandoah/shenandoahUtils.hpp"
#include "runtime/thread.hpp"

/* Enqueue candidates for deduplication.
 * The method should only be called by GC worker threads during marking phases.
 */
bool ShenandoahStringDedup::check_string_candidate(oop java_string) {
  assert(Thread::current()->is_Worker_thread(),
        "Only from a GC worker thread");

  if (StringDedup::is_below_threshold_age(java_string->age())) {
    const markWord mark = java_string->mark();
    // Having/had displaced header, too risk to deal with them, skip
    if (mark == markWord::INFLATING() || mark.has_displaced_mark_helper()) {
      return false;
    }

    // Increase string age and enqueue it when it rearches age threshold
    markWord new_mark = mark.incr_age();
    if (mark == java_string->cas_set_mark(new_mark, mark)) {
      return StringDedup::is_threshold_age(new_mark.age());
    }
  }
  return false;
}
