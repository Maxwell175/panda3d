#include "graphicsEngine_ext_csharp.h"

#ifdef HAVE_CSHARP

GraphicsEngine *Extension<GraphicsEngine>::
get_global_ptr() {
  return GraphicsEngine::get_global_ptr();
}

#endif
