/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ramfile_ext_csharp.cxx
 * @author Maxwell175
 * @date 2026-07-31
 */

#include "ramfile_ext_csharp.h"

#ifdef HAVE_CSHARP

static vector_uchar
to_bytes(const std::string &data) {
  vector_uchar result;
  result.reserve(data.size());
  for (char c : data) {
    result.push_back((unsigned char)c);
  }
  return result;
}

/**
 * Extracts the next `length` bytes from the current position, advancing it.
 */
vector_uchar Extension<Ramfile>::
read_bytes(size_t length) {
  return to_bytes(_this->read(length));
}

/**
 * Returns the entire contents of the Ramfile as bytes.
 */
vector_uchar Extension<Ramfile>::
get_data_bytes() const {
  return to_bytes(_this->get_data());
}

#endif  // HAVE_CSHARP
