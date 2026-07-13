/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileSystem_ext_csharp.h
 * @author Maxwell175
 * @date 2026-07-13
 */

#ifndef VIRTUALFILESYSTEM_EXT_CSHARP_H
#define VIRTUALFILESYSTEM_EXT_CSHARP_H

#include "dtoolbase.h"

#ifdef HAVE_CSHARP

#include "extension.h"
#include "virtualFileSystem.h"

/**
 * Filename-addressed counterparts of Extension<VirtualFile>::read_file_to() and
 * write_file_from().
 */
template<>
class EXPCL_PANDA_EXPRESS Extension<VirtualFileSystem> : public ExtensionBase<VirtualFileSystem> {
public:
  bool read_file_to(const Filename &filename, std::ostream &out, bool auto_unwrap) const;
  bool write_file_from(const Filename &filename, std::istream &in, bool auto_wrap);
};

#endif  // HAVE_CSHARP

#endif  // VIRTUALFILESYSTEM_EXT_CSHARP_H
