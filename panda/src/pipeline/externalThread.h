/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file externalThread.h
 * @author drose
 * @date 2006-01-30
 */

#ifndef EXTERNALTHREAD_H
#define EXTERNALTHREAD_H

#include "pandabase.h"
#include "thread.h"

/**
 * The Thread object for a thread Panda did not create.  One is minted per
 * external OS thread (lazily on first get_current_thread(), or via
 * Thread::bind_thread()), so each has its own Thread and epoch participant.
 */
class EXPCL_PANDA_PIPELINE ExternalThread : public Thread {
private:
  ExternalThread(const std::string &name, const std::string &sync_name);
  virtual void thread_main();

public:
  virtual bool is_auto_bound() const { return true; }

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    Thread::init_type();
    register_type(_type_handle, "ExternalThread",
                  Thread::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class Thread;
};

#endif
