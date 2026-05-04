/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncFuture_ext_csharp.cxx
 * @author Maxwell175
 * @date 2026-05-01
 */

#include "asyncFuture_ext_csharp.h"

#ifdef HAVE_CSHARP

void Extension<AsyncFuture>::
set_result_none() {
  if (!_this->done()) {
    _this->set_result(nullptr);
  }
}

void Extension<AsyncFuture>::
set_result_object(TypedObject *result) {
  if (!_this->done()) {
    _this->set_result(result);
  }
}

TypedObject *Extension<AsyncFuture>::
get_result_ptr() const {
  if (!_this->done() || _this->cancelled()) {
    return nullptr;
  }
  return _this->get_result();
}

bool Extension<AsyncFuture>::
add_waiting_task_csharp(AsyncTask *task) {
  if (task == nullptr) {
    return false;
  }
  return _this->add_waiting_task(task);
}

PT(AsyncFuture) Extension<AsyncFuture>::
gather_csharp(AsyncFuture **futures, int count) {
  AsyncFuture::Futures vec;
  vec.reserve((size_t)count);
  for (int i = 0; i < count; ++i) {
    if (futures[i] != nullptr) {
      vec.push_back(futures[i]);
    }
  }
  return AsyncFuture::gather(std::move(vec));
}

#endif  // HAVE_CSHARP
