/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionPolygon_ext_csharp.h
 * @author Maxwell175
 * @date 2026-07-31
 */

#ifndef COLLISIONPOLYGON_EXT_CSHARP_H
#define COLLISIONPOLYGON_EXT_CSHARP_H

#include "dtoolbase.h"

#ifdef HAVE_CSHARP

#include "extension.h"
#include "collisionPolygon.h"

/**
 * C# extension methods on CollisionPolygon.  Parallel to collisionPolygon_ext.h
 * for Python: declared via `CSHARP_EXTENSION(...)` in collisionPolygon.h,
 * defined here in a template specialization of Extension<CollisionPolygon>.
 *
 * Exposes the N-point setup_points -- the base class only publishes the fixed
 * 3- and 4-point constructors, so this is the only way to build a polygon from
 * an arbitrary list of vertices.
 */
template<>
class Extension<CollisionPolygon> : public ExtensionBase<CollisionPolygon> {
public:
  void setup_points(const pvector<LPoint3> &points);
};

#endif  // HAVE_CSHARP

#endif  // COLLISIONPOLYGON_EXT_CSHARP_H
