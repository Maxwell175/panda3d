/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncFuture_ext_csharp.h
 * @author Maxwell175
 * @date 2026-05-01
 */

#ifndef ASYNCFUTURE_EXT_CSHARP_H
#define ASYNCFUTURE_EXT_CSHARP_H

#include "dtoolbase.h"

#ifdef HAVE_CSHARP

#include "extension.h"
#include "asyncFuture.h"

/**
 * C# extension methods on AsyncFuture.  Parallel to asyncFuture_ext.h for
 * Python: declared via `CSHARP_EXTENSION(...)` in asyncFuture.h, defined
 * here in a template specialization of Extension<AsyncFuture>.
 *
 * Exposes internals that are not PUBLISHED on the base class — set_result,
 * get_result, the waiting-task hook used by coroutine resumption, and the
 * array-form of gather() — so the Panda3D.Async managed library can drive
 * the future state machine directly.
 */
template<>
class Extension<AsyncFuture> : public ExtensionBase<AsyncFuture> {
public:
  void set_result_none();
  void set_result_object(TypedObject *result);
  TypedObject *get_result_ptr() const;
  bool add_waiting_task_csharp(AsyncTask *task);

  static PT(AsyncFuture) gather_csharp(AsyncFuture **futures, int count);
};

#endif  // HAVE_CSHARP

#endif  // ASYNCFUTURE_EXT_CSHARP_H
