/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgCullTraverser.I
 * @author drose
 * @date 2002-03-14
 */

/**
 *
 */
PGCullTraverser::
PGCullTraverser(PGTop *top, CullTraverser *trav) :
  CullTraverser(*trav),
  _top(top)
{
  _sort_index = 0;
}

/**
 * Adds a region to the group this traversal is collecting into.  Does nothing if the PGTop has no
 * MouseWatcher, in which case there is no group to collect into.
 */
INLINE void PGCullTraverser::
add_region(MouseWatcherRegion *region) {
  if (_watcher_group != nullptr) {
    _watcher_group->add_region(region);
  }
}
