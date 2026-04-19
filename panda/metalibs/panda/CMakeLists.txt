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

if(UNIX AND NOT APPLE AND NOT BUILD_SHARED_LIBS AND HAVE_HARFBUZZ AND HAVE_FREETYPE)
  set(_hb_file "")
  set(_ft_file "")
  if(TARGET harfbuzz::harfbuzz)
    set(_hb_file "$<TARGET_FILE:harfbuzz::harfbuzz>")
  elseif(DEFINED HARFBUZZ_LIBRARY)
    set(_hb_file "${HARFBUZZ_LIBRARY}")
  endif()
  if(TARGET freetype)
    set(_ft_file "$<TARGET_FILE:freetype>")
  elseif(TARGET Freetype::Freetype)
    set(_ft_file "$<TARGET_FILE:Freetype::Freetype>")
  elseif(DEFINED FREETYPE_LIBRARY)
    set(_ft_file "${FREETYPE_LIBRARY}")
  endif()
  if(_hb_file AND _ft_file)
    set_property(TARGET panda APPEND PROPERTY INTERFACE_LINK_LIBRARIES
      "-Wl,--start-group" "${_hb_file}" "${_ft_file}" "-Wl,--end-group"
    )
  endif()
  unset(_hb_file)
  unset(_ft_file)
endif()

install(TARGETS panda
  EXPORT Core COMPONENT Core
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT CoreDevel)
