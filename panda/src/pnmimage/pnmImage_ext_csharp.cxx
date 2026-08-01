/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmImage_ext_csharp.cxx
 * @author Maxwell175
 * @date 2026-08-01
 */

#include "pnmImage_ext_csharp.h"

#ifdef HAVE_CSHARP

/**
 * Base address of the contiguous pixel array (row-major, x_size * y_size xels).
 */
uint64_t Extension<PNMImage>::
get_array_pointer() {
  return (uint64_t)(uintptr_t)_this->get_array();
}

/**
 * Base address of the contiguous alpha array (x_size * y_size xelvals), or 0
 * if the image has no alpha channel.
 */
uint64_t Extension<PNMImage>::
get_alpha_array_pointer() {
  return (uint64_t)(uintptr_t)_this->get_alpha_array();
}

#endif  // HAVE_CSHARP
