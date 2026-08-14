/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_navigation.h
 * @author ashwini
 * @date 2020-060-21
 */

#ifndef CONFIG_NAVIGATION_H
#define CONFIG_NAVIGATION_H

#include "pandabase.h"
#include "nodePath.h"
#include "notifyCategoryProxy.h"
#include "configVariableDouble.h"
#include "configVariableString.h"
#include "configVariableInt.h"
#include "lmatrix.h"
#include "coordinateSystem.h"

NotifyCategoryDecl(navigation, EXPCL_NAVIGATION, EXPTP_NAVIGATION);

extern EXPCL_NAVIGATION void init_libnavigation();

typedef pvector<NodePath> NodePaths;

/**
 * Recast and Detour are hard-wired to y-up-right; Panda's coordinate system is a runtime choice.  These
 * are the only bridge between the two.
 *
 * Everything stored in this module -- tiles, triangles, bounds, the tile-grid origin -- is in y-up, and
 * conversion happens only where a value crosses the published API.  So a mesh does not depend on the
 * setting at all; it only ever describes the caller's space.
 *
 * Functions rather than cached matrices, so CS_default resolves per call.
 */
INLINE LMatrix4 nav_to_recast_mat() {
  return LMatrix4::convert_mat(CS_default, CS_yup_right);
}

INLINE LMatrix4 nav_from_recast_mat() {
  return LMatrix4::convert_mat(CS_yup_right, CS_default);
}

#endif
