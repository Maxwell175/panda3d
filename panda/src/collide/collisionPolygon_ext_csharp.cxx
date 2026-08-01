/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionPolygon_ext_csharp.cxx
 * @author Maxwell175
 * @date 2026-07-31
 */

#include "collisionPolygon_ext_csharp.h"

#ifdef HAVE_CSHARP

/**
 * Rebuilds the polygon from the given vertices (three or more, coplanar).
 */
void Extension<CollisionPolygon>::
setup_points(const pvector<LPoint3> &points) {
  if (points.size() < 3) {
    return;
  }
  _this->setup_points(&points[0], &points[0] + points.size());
}

#endif  // HAVE_CSHARP
