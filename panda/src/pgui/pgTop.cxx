/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgTop.cxx
 * @author drose
 * @date 2002-03-13
 */

#include "pgTop.h"
#include "pgMouseWatcherGroup.h"
#include "pgCullTraverser.h"
#include "cullBinAttrib.h"

#include "omniBoundingVolume.h"

TypeHandle PGTop::_type_handle;

/**
 *
 */
PGTop::
PGTop(const std::string &name) :
  PandaNode(name)
{
  set_cull_callback();

  // We flag the PGTop as renderable, even though it technically doesn't have
  // anything to render, but we do need the traverser to visit it every frame.
  set_renderable();

  _start_sort = 0;

  // A PGTop node normally has an infinite bounding volume.  Screw culling.
  set_internal_bounds(new OmniBoundingVolume());
  set_final(true);

  // Also, screw state sorting.  By default, everything under PGTop will be
  // unsorted: rendered in scene graph order.  This is closer to what the user
  // wants anyway in a 2-d scene graph.

  set_attrib(CullBinAttrib::make("unsorted", 0));
}

/**
 *
 */
PGTop::
~PGTop() {
  set_mouse_watcher(nullptr);
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *PGTop::
make_copy() const {
  return new PGTop(*this);
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool PGTop::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // We create a new MouseWatcherGroup for the purposes of collecting a new
  // set of regions visible onscreen.
  //
  // This runs on the cull thread while set_mouse_watcher() may be running on the app thread, so both
  // members are taken in one critical section and everything after it works from the locals.  Reading
  // _watcher again further down is what used to crash: set_mouse_watcher(nullptr) between the test
  // and the use left a null to dereference.
  PT(MouseWatcher) watcher;
  PT(PGMouseWatcherGroup) old_watcher_group;
  PT(PGMouseWatcherGroup) new_watcher_group;
  {
    LightMutexHolder holder(_watcher_lock);
    watcher = _watcher;
    old_watcher_group = _watcher_group;
    if (old_watcher_group != nullptr) {
      new_watcher_group = new PGMouseWatcherGroup(this);
      _watcher_group = new_watcher_group;
    }
  }

  if (old_watcher_group != nullptr) {
    // Outside the lock: the group takes its own.
    old_watcher_group->clear_top(this);
  }

  // Now subsitute for the normal CullTraverser a special one of our own
  // choosing.  This just carries around a pointer back to the PGTop node, for
  // the convenience of PGItems to register themselves as they are drawn, along
  // with the group they are to be collected into.
  PGCullTraverser pg_trav(this, trav);
  pg_trav.local_object();
  pg_trav._sort_index = _start_sort;
  pg_trav._watcher_group = new_watcher_group;
  pg_trav.traverse_below(data);
  pg_trav.end_traverse();

  // Now tell the watcher about the new set of regions.  Strictly speaking, we
  // shouldn't do this until the frame that we're about to render has been
  // presented; otherwise, we may make regions active before they are actually
  // visible.  But no one has complained about this so far.
  //
  // The watcher can legitimately be null here even with a group in hand, if the app thread cleared it
  // during the traversal above; there is then nothing to tell, and the group we built is simply
  // dropped.
  if (new_watcher_group != nullptr && watcher != nullptr) {
    watcher->replace_group(old_watcher_group, new_watcher_group);
  }

  // We've taken care of the traversal, thank you.
  return false;
}

/**
 * Sets the MouseWatcher pointer that the PGTop object registers its PG items
 * with.  This must be set before the PG items are active.
 */
void PGTop::
set_mouse_watcher(MouseWatcher *watcher) {
  // The swap happens under the lock; the calls into the old and new watchers happen outside it, since
  // those take locks of their own and a cull in flight may already be inside them.
  PT(MouseWatcher) old_watcher;
  PT(PGMouseWatcherGroup) old_watcher_group;
  PT(PGMouseWatcherGroup) new_watcher_group;
  {
    LightMutexHolder holder(_watcher_lock);
    old_watcher = _watcher;
    old_watcher_group = _watcher_group;

    _watcher = watcher;
    _watcher_group = nullptr;

    if (watcher != nullptr) {
      new_watcher_group = new PGMouseWatcherGroup(this);
      _watcher_group = new_watcher_group;
    }
  }

  if (old_watcher_group != nullptr) {
    old_watcher_group->clear_top(this);
  }
  if (old_watcher != nullptr && old_watcher_group != nullptr) {
    old_watcher->remove_group(old_watcher_group);
  }
  if (watcher != nullptr) {
    watcher->add_group(new_watcher_group);
  }
}
