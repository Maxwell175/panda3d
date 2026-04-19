#include "graphicsEngine_ext_csharp.h"

#ifdef HAVE_CSHARP

#ifdef LINK_ALL_STATIC

#if defined(HAVE_TINYDISPLAY)
extern EXPCL_TINYDISPLAY void init_libtinydisplay();
#endif
#if defined(HAVE_DX9)
extern EXPCL_PANDADX void init_libpandadx9();
#endif
#if defined(HAVE_GL)
extern void init_libpandagl();
#endif

extern EXPCL_PANDA_CHAR void init_libchar();
extern EXPCL_PANDA_MOVIES void init_libmovies();
extern "C" EXPCL_PANDA_PNMIMAGETYPES void init_libpnmimagetypes();

#ifdef HAVE_EGG
extern void init_libpandaegg();
#endif

static bool _static_initialized = false;

static void ensure_static_init() {
  if (_static_initialized) return;
  _static_initialized = true;

#if defined(HAVE_GL)
  init_libpandagl();
#endif
#if defined(HAVE_DX9)
  init_libpandadx9();
#endif
#if defined(HAVE_TINYDISPLAY)
  init_libtinydisplay();
#endif

  init_libchar();
  init_libmovies();
  init_libpnmimagetypes();

#ifdef HAVE_EGG
  init_libpandaegg();
#endif
}

#endif

GraphicsEngine *Extension<GraphicsEngine>::
get_global_ptr() {
#ifdef LINK_ALL_STATIC
  ensure_static_init();
#endif
  return GraphicsEngine::get_global_ptr();
}

#endif
