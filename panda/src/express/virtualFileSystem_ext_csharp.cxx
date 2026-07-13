/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileSystem_ext_csharp.cxx
 * @author Maxwell175
 * @date 2026-07-13
 */

#include "virtualFileSystem_ext_csharp.h"

#ifdef HAVE_CSHARP

/**
 * Copies the named file into out, decompressing (.pz) when auto_unwrap is set.
 */
bool Extension<VirtualFileSystem>::
read_file_to(const Filename &filename, std::ostream &out, bool auto_unwrap) const {
  std::istream *in = _this->open_read_file(filename, auto_unwrap);
  if (in == nullptr) {
    return false;
  }

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  in->read(buffer, buffer_size);
  size_t count = in->gcount();
  while (count != 0) {
    out.write(buffer, count);
    in->read(buffer, buffer_size);
    count = in->gcount();
  }

  bool failed = (in->fail() && !in->eof());
  VirtualFileSystem::close_read_file(in);

  return (!failed && !out.fail());
}

/**
 * Replaces the named file with everything readable from in, compressing (.pz)
 * when auto_wrap is set.
 */
bool Extension<VirtualFileSystem>::
write_file_from(const Filename &filename, std::istream &in, bool auto_wrap) {
  std::ostream *out = _this->open_write_file(filename, auto_wrap, true);
  if (out == nullptr) {
    return false;
  }

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  in.read(buffer, buffer_size);
  size_t count = in.gcount();
  while (count != 0) {
    out->write(buffer, count);
    in.read(buffer, buffer_size);
    count = in.gcount();
  }

  bool failed = (in.fail() && !in.eof()) || out->fail();
  VirtualFileSystem::close_write_file(out);

  return !failed;
}

#endif  // HAVE_CSHARP
