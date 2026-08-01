/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmImage_ext_csharp.h
 * @author Maxwell175
 * @date 2026-08-01
 */

#ifndef PNMIMAGE_EXT_CSHARP_H
#define PNMIMAGE_EXT_CSHARP_H

#include "dtoolbase.h"

#ifdef HAVE_CSHARP

#include "extension.h"
#include "pnmImage.h"

/**
 * C# extension methods on PNMImage: hand back the base address of the
 * contiguous pixel / alpha arrays so managed code can wrap them in a zero-copy
 * span (xel = three unsigned shorts).  The address is only valid until the
 * image is resized or reallocated.  Returned as an integer since a void* return
 * isn't mapped to C#; cast to a pointer on the managed side.
 */
template<>
class Extension<PNMImage> : public ExtensionBase<PNMImage> {
public:
  uint64_t get_array_pointer();
  uint64_t get_alpha_array_pointer();
};

#endif  // HAVE_CSHARP

#endif  // PNMIMAGE_EXT_CSHARP_H
