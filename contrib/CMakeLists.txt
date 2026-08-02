if(NOT BUILD_PANDA)
  message(FATAL_ERROR "Cannot build contrib without panda!  Please enable the BUILD_PANDA option.")
endif()

# Include source directories
add_subdirectory(src/ai)
add_subdirectory(src/contribbase)
add_subdirectory(src/rplight)

if(INTERROGATE_CSHARP_INTERFACE)
  add_csharp_module(panda3d.ai p3ai IMPORT panda3d.core)
  add_csharp_module(panda3d._rplight p3rplight IMPORT panda3d.core)
endif()

if(HAVE_PYTHON)
  add_python_module(panda3d.ai p3ai
    IMPORT panda3d.core COMPONENT ContribPython)

  add_python_module(panda3d._rplight p3rplight
    IMPORT panda3d.core COMPONENT ContribPython)
endif()

export_targets(Contrib COMPONENT ContribDevel)
