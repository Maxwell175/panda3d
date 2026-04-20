set(PANDA_LINK_TARGETS
  p3chan p3char p3collide p3cull p3device p3dgraph p3display
  p3event p3gobj p3grutil p3gsgbase p3linmath p3mathutil
  p3movies p3parametrics p3pgraph p3pgraphnodes p3pgui p3pipeline
  p3pnmimage p3text p3pnmimagetypes p3pstatclient p3putil p3recorder p3tform
)

if(WANT_NATIVE_NET)
  list(APPEND PANDA_LINK_TARGETS p3nativenet)
  if(HAVE_NET)
    list(APPEND PANDA_LINK_TARGETS p3net)
  endif()
endif()

if(HAVE_AUDIO)
  list(APPEND PANDA_LINK_TARGETS p3audio)
endif()

if(HAVE_FREETYPE)
  list(APPEND PANDA_LINK_TARGETS p3pnmtext)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "CoreDevel")
add_metalib(panda INIT init_libpanda panda.h COMPONENTS ${PANDA_LINK_TARGETS})
unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)

if(UNIX AND NOT APPLE AND NOT BUILD_SHARED_LIBS)
  set(_group_libs "")

  if(HAVE_HARFBUZZ AND HAVE_FREETYPE)
    if(TARGET harfbuzz::harfbuzz)
      list(APPEND _group_libs "$<TARGET_FILE:harfbuzz::harfbuzz>")
    elseif(DEFINED HARFBUZZ_LIBRARY)
      list(APPEND _group_libs "${HARFBUZZ_LIBRARY}")
    endif()
    if(TARGET freetype)
      list(APPEND _group_libs "$<TARGET_FILE:freetype>")
    elseif(TARGET Freetype::Freetype)
      list(APPEND _group_libs "$<TARGET_FILE:Freetype::Freetype>")
    elseif(DEFINED FREETYPE_LIBRARY)
      list(APPEND _group_libs "${FREETYPE_LIBRARY}")
    endif()
  endif()

  if(_group_libs)
    set_property(TARGET panda APPEND PROPERTY INTERFACE_LINK_LIBRARIES
      "-Wl,--start-group" ${_group_libs} "-Wl,--end-group"
    )
  endif()
  unset(_group_libs)
endif()

install(TARGETS panda
  EXPORT Core COMPONENT Core
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT CoreDevel)
