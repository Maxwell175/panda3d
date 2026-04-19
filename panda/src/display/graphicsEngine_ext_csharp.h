#ifndef GRAPHICSENGINE_EXT_CSHARP_H
#define GRAPHICSENGINE_EXT_CSHARP_H

#include "dtoolbase.h"

#ifdef HAVE_CSHARP

#include "extension.h"
#include "graphicsEngine.h"

template<>
class Extension<GraphicsEngine> : public ExtensionBase<GraphicsEngine> {
public:
  static GraphicsEngine *get_global_ptr();
};

#endif

#endif
