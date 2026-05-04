/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file managedAsyncTask.cxx
 * @author Maxwell175
 * @date 2026-05-01
 */

#include "managedAsyncTask.h"

#ifdef HAVE_CSHARP

TypeHandle ManagedAsyncTask::_type_handle;

ManagedAsyncTask::
ManagedAsyncTask(const std::string &name, RunFn run, FreeFn free_fn,
                 void *user_data) :
  AsyncTask(name),
  _run(run),
  _free(free_fn),
  _user_data(user_data)
{
}

ManagedAsyncTask::
~ManagedAsyncTask() {
  if (_free != nullptr) {
    (*_free)(_user_data);
  }
}

PT(ManagedAsyncTask) ManagedAsyncTask::
make(const std::string &name, unsigned long long run_fn,
     unsigned long long free_fn, unsigned long long user_data) {
  return new ManagedAsyncTask(name,
                              (RunFn)(uintptr_t)run_fn,
                              (FreeFn)(uintptr_t)free_fn,
                              (void *)(uintptr_t)user_data);
}

bool ManagedAsyncTask::
is_runnable() {
  return _run != nullptr;
}

AsyncTask::DoneStatus ManagedAsyncTask::
do_task() {
  nassertr(_run != nullptr, DS_interrupt);
  return (DoneStatus)(*_run)(_user_data);
}

#endif  // HAVE_CSHARP
