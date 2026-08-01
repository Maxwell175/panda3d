/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggNode_ext_csharp.cxx
 * @author Maxwell175
 * @date 2026-07-31
 */

#include "eggNode_ext_csharp.h"

#ifdef HAVE_CSHARP

#include <sstream>

/**
 * Parses an EggData from the raw egg syntax.  Returns null on failure.
 */
PT(EggData) parse_egg_data(const std::string &egg_syntax) {
  PT(EggData) data = new EggData;
  data->set_auto_resolve_externals(false);

  std::istringstream in(egg_syntax);
  if (!data->read(in)) {
    return nullptr;
  }
  return data;
}

/**
 * Parses a single egg node from the raw egg syntax.  Returns null on failure
 * or if the syntax does not describe exactly one node.
 */
PT(EggNode) parse_egg_node(const std::string &egg_syntax) {
  PT(EggData) data = parse_egg_data(egg_syntax);
  if (data == nullptr) {
    return nullptr;
  }
  if (data->size() != 1) {
    return nullptr;
  }
  return data->remove_child(data->get_first_child());
}

#endif  // HAVE_CSHARP
