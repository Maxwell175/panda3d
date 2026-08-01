/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggNode_ext_csharp.h
 * @author Maxwell175
 * @date 2026-07-31
 */

#ifndef EGGNODE_EXT_CSHARP_H
#define EGGNODE_EXT_CSHARP_H

#include "dtoolbase.h"

#ifdef HAVE_CSHARP

#include "eggData.h"
#include "eggNode.h"

/**
 * C# equivalents of the parse_egg_data / parse_egg_node global helpers that
 * eggNode_ext.h publishes for Python.  They parse an egg-syntax string into an
 * egg tree; unlike the Python versions they return null on failure rather than
 * raising a Python exception.
 */
BEGIN_PUBLISH
PT(EggData) parse_egg_data(const std::string &egg_syntax);
PT(EggNode) parse_egg_node(const std::string &egg_syntax);
END_PUBLISH

#endif  // HAVE_CSHARP

#endif  // EGGNODE_EXT_CSHARP_H
