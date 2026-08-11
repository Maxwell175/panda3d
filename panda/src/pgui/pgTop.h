/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgTop.h
 * @author drose
 * @date 2002-03-13
 */

#ifndef PGTOP_H
#define PGTOP_H

#include "pandabase.h"

#include "pgMouseWatcherGroup.h"

#include "pandaNode.h"
#include "mouseWatcher.h"
#include "pointerTo.h"
#include "lightMutex.h"

class GraphicsStateGuardian;
class PGMouseWatcherGroup;

/**
 * The "top" node of the new Panda GUI system.  This node must be parented to
 * the 2-d scene graph, and all PG objects should be parented to this node or
 * somewhere below it.  PG objects not parented within this hierarchy will not
 * be clickable.
 *
 * This node begins the special traversal of the PG objects that registers
 * each node within the MouseWatcher and forces everything to render in a
 * depth-first, left-to-right order, appropriate for 2-d objects.
 */
class EXPCL_PANDA_PGUI PGTop : public PandaNode {
PUBLISHED:
  explicit PGTop(const std::string &name);
  virtual ~PGTop();

protected:
  INLINE PGTop(const PGTop &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

PUBLISHED:
  void set_mouse_watcher(MouseWatcher *watcher);
  INLINE MouseWatcher *get_mouse_watcher() const;
  INLINE MouseWatcherGroup *get_group() const;
  MAKE_PROPERTY(mouse_watcher, get_mouse_watcher, set_mouse_watcher);
  MAKE_PROPERTY(group, get_group);

  INLINE void set_start_sort(int start_sort);
  INLINE int get_start_sort() const;
  MAKE_PROPERTY(start_sort, get_start_sort, set_start_sort);

public:
  // These methods duplicate the functionality of MouseWatcherGroup.
  INLINE void add_region(MouseWatcherRegion *region);
  INLINE void clear_regions();

private:
  // Written by set_mouse_watcher() and the destructor on the app thread, and read and swapped by
  // cull_callback() on the cull thread.  Neither is pipelined -- cull_callback() runs against the live
  // node rather than a CycleData copy -- so with a real cull thread these are a plain data race, and a
  // PT() assignment torn across two threads corrupts a refcount rather than merely reading stale.
  //
  // The failure observed was the narrow one: set_mouse_watcher(nullptr) landing between
  // cull_callback()'s test of _watcher_group and its use of _watcher, leaving a null _watcher to be
  // dereferenced.  The nassertr() there catches it in a development build and is compiled out of a
  // release one, so it presented as a segfault inside MouseWatcher::replace_group() taking a lock at
  // offset 0x48 of nullptr.
  //
  // The lock is only ever held across pointer swaps, never across a call into MouseWatcher or a group
  // (both of which take locks of their own), so it introduces no lock ordering.
  mutable LightMutex _watcher_lock;
  PT(MouseWatcher) _watcher;
  PT(PGMouseWatcherGroup) _watcher_group;
  int _start_sort;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "PGTop",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PGMouseWatcherGroup;
};

#include "pgTop.I"

#endif
