/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ramfile_ext_csharp.h
 * @author Maxwell175
 * @date 2026-07-31
 */

#ifndef RAMFILE_EXT_CSHARP_H
#define RAMFILE_EXT_CSHARP_H

#include "dtoolbase.h"

#ifdef HAVE_CSHARP

#include "extension.h"
#include "ramfile.h"
#include "vector_uchar.h"

/**
 * C# extension methods on Ramfile.  Parallel to ramfile_ext.h for Python, but
 * returns bytes as a vector_uchar (byte-safe, zero-copy span in C#) instead of
 * the base std::string overloads, which marshal to C# as lossy UTF-8 text.
 */
// EXPCL is required: the C# stub is compiled into libpandaexpress but called
// from libpanda, so the symbol must be exported.
template<>
class EXPCL_PANDA_EXPRESS Extension<Ramfile> : public ExtensionBase<Ramfile> {
public:
  vector_uchar read_bytes(size_t length);
  vector_uchar get_data_bytes() const;
};

#endif  // HAVE_CSHARP

#endif  // RAMFILE_EXT_CSHARP_H
