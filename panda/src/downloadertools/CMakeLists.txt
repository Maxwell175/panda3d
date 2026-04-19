if(NOT BUILD_TOOLS)
  return()
endif()

if(HAVE_OPENSSL)
  add_executable(pencrypt pencrypt.cxx)
  target_link_libraries(pencrypt panda)

  add_executable(pdecrypt pdecrypt.cxx)
  target_link_libraries(pdecrypt panda)

  install(TARGETS pencrypt pdecrypt
    EXPORT Tools COMPONENT Tools
    DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(HAVE_ZLIB)
  add_executable(pzip pzip.cxx)
  target_link_libraries(pzip panda)

  if(NOT CMAKE_CROSSCOMPILING)
    add_executable(host_pzip ALIAS pzip)
  else()
    find_program(HOST_PATH_PZIP pzip)

    add_executable(host_pzip IMPORTED GLOBAL)
    if(HOST_PATH_PZIP)
      set_target_properties(host_pzip PROPERTIES
        IMPORTED_LOCATION "${HOST_PATH_PZIP}")
    endif()

  endif()

  add_executable(punzip punzip.cxx)
  target_link_libraries(punzip panda)

  install(TARGETS pzip punzip
    EXPORT Tools COMPONENT Tools
    DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

add_executable(multify multify.cxx)
target_link_libraries(multify panda)
install(TARGETS multify EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

# Same static-linking fix as in p3pandatoolbase: resolve get_pipe_type_*
# symbols needed by graphicsPipeSelection.cxx in static builds.
if(NOT BUILD_SHARED_LIBS)
  set(_display_pipe_libs "")
  if(HAVE_GL)
    list(APPEND _display_pipe_libs pandagl)
  endif()
  if(HAVE_DX9)
    list(APPEND _display_pipe_libs pandadx9)
  endif()
  if(HAVE_TINYDISPLAY)
    list(APPEND _display_pipe_libs p3tinydisplay)
  endif()
  foreach(_tool pencrypt pdecrypt pzip punzip multify)
    if(TARGET ${_tool})
      target_link_libraries(${_tool} ${_display_pipe_libs})
    endif()
  endforeach()
  unset(_display_pipe_libs)
  unset(_tool)
endif()
