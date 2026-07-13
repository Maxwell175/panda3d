/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFile_ext_csharp.h
 * @author Maxwell175
 * @date 2026-07-13
 */

#ifndef VIRTUALFILE_EXT_CSHARP_H
#define VIRTUALFILE_EXT_CSHARP_H

#include "dtoolbase.h"

#ifdef HAVE_CSHARP

#include "extension.h"
#include "virtualFile.h"

/**
 * C# counterpart of virtualFile_ext.h: copies the file to or from a managed
 * System.IO.Stream, decompressing (.pz) when auto_unwrap/auto_wrap is set.
 */
// EXPCL is required: the C# stub is compiled into libpandaexpress but called from
// libpanda, so the symbol must be exported.
template<>
class EXPCL_PANDA_EXPRESS Extension<VirtualFile> : public ExtensionBase<VirtualFile> {
public:
  bool read_file_to(std::ostream &out, bool auto_unwrap) const;
  bool write_file_from(std::istream &in, bool auto_wrap);
};

#endif  // HAVE_CSHARP

#endif  // VIRTUALFILE_EXT_CSHARP_H
