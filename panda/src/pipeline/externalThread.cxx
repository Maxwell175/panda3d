/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file externalThread.cxx
 * @author drose
 * @date 2006-01-30
 */

#include "externalThread.h"

TypeHandle ExternalThread::_type_handle;

/**
 * Creates the Thread object for one external OS thread.
 */
ExternalThread::
ExternalThread(const std::string &name, const std::string &sync_name) :
  Thread(name, sync_name)
{
  _started = true;
#ifdef THREADED_PIPELINE
  // Already running on the external thread, so take occupancy now.
  acquire_stage_occupancy();
#endif
}

/**
 *
 */
void ExternalThread::
thread_main() {
}
