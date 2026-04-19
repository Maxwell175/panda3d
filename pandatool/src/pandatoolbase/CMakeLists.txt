set(P3PANDATOOLBASE_HEADERS
  animationConvert.h
  config_pandatoolbase.h
  distanceUnit.h
  pandatoolbase.h pandatoolsymbols.h
  pathReplace.h pathReplace.I
  pathStore.h
)

set(P3PANDATOOLBASE_SOURCES
  animationConvert.cxx
  config_pandatoolbase.cxx
  distanceUnit.cxx
  pandatoolbase.cxx
  pathReplace.cxx
  pathStore.cxx
)

composite_sources(p3pandatoolbase P3PANDATOOLBASE_SOURCES)
add_library(p3pandatoolbase STATIC ${P3PANDATOOLBASE_HEADERS} ${P3PANDATOOLBASE_SOURCES})
target_link_libraries(p3pandatoolbase panda)

# In static builds, graphicsPipeSelection.cxx (part of panda) directly
# references get_pipe_type_* symbols from display pipe modules. Any final
# executable that links panda must also link those modules so the linker
# can resolve these symbols. Adding them here propagates the dependency
# to all pandatool executables via p3pandatoolbase's public link chain.
if(NOT BUILD_SHARED_LIBS)
  if(HAVE_GL)
    target_link_libraries(p3pandatoolbase pandagl)
  endif()
  if(HAVE_DX9)
    target_link_libraries(p3pandatoolbase pandadx9)
  endif()
  if(HAVE_TINYDISPLAY)
    target_link_libraries(p3pandatoolbase p3tinydisplay)
  endif()
endif()

# ptloader - a module - may need to link in libraries in this package, so:
if(MODULE_TYPE STREQUAL "MODULE")
  set_target_properties(p3pandatoolbase PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    INTERFACE_POSITION_INDEPENDENT_CODE ON)
endif()

install(TARGETS p3pandatoolbase
  EXPORT ToolsDevel COMPONENT ToolsDevel
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT ToolsDevel)
install(FILES ${P3PANDATOOLBASE_HEADERS} COMPONENT ToolsDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
