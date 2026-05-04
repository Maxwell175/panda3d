/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file managedAsyncTask.h
 * @author Maxwell175
 * @date 2026-05-01
 */

#ifndef MANAGEDASYNCTASK_H
#define MANAGEDASYNCTASK_H

#include "pandabase.h"

#ifdef HAVE_CSHARP

#include "asyncTask.h"

/**
 * An AsyncTask whose behaviour is driven by C function pointers, designed
 * to bridge C# delegates into a Panda3D task chain.  Functionally similar
 * to GenericAsyncTask but stripped to the minimum the C# async/await
 * layer needs: a run callback, a free-user-data callback, and an opaque
 * user-data pointer (typically a pinned GCHandle on the managed side).
 *
 * Constructed via the PUBLISHED `make` factory — interrogate marshals
 * the opaque pointers as C# IntPtr, which is the safe representation for
 * [UnmanagedCallersOnly] function pointers produced by the managed
 * trampolines.
 */
class EXPCL_PANDA_EVENT ManagedAsyncTask : public AsyncTask {
PUBLISHED:
  /**
   * Builds a ManagedAsyncTask.  `run_fn` is invoked every epoch and must
   * return a DoneStatus value cast to int.  `free_fn` is called when the
   * task is destroyed; it is responsible for releasing `user_data`.
   * Either callback may be null.  Returns a ref-counted task.
   *
   * Pointers are declared as `unsigned long long` so interrogate maps
   * them to C# `ulong` — always 8 bytes, matching a native pointer on
   * 64-bit Linux/macOS/Windows targets (the only platforms panda3d C#
   * bindings currently support).  The managed side passes the address
   * of a [UnmanagedCallersOnly] function or a GCHandle value; we cast
   * back to a function pointer / opaque user-data here.
   */
  static PT(ManagedAsyncTask) make(const std::string &name,
                                   unsigned long long run_fn,
                                   unsigned long long free_fn,
                                   unsigned long long user_data);

  virtual ~ManagedAsyncTask();
  ALLOC_DELETED_CHAIN(ManagedAsyncTask);

protected:
  virtual bool is_runnable();
  virtual DoneStatus do_task();

private:
  typedef int (*RunFn)(void *user_data);
  typedef void (*FreeFn)(void *user_data);

  ManagedAsyncTask(const std::string &name, RunFn run, FreeFn free_fn,
                   void *user_data);

  RunFn _run;
  FreeFn _free;
  void *_user_data;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AsyncTask::init_type();
    register_type(_type_handle, "ManagedAsyncTask",
                  AsyncTask::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "managedAsyncTask.I"

#endif  // HAVE_CSHARP

#endif  // MANAGEDASYNCTASK_H
