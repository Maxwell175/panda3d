# Filename: FindDetour.cmake
# Authors: Maxwell175 (27 Feb 2022)
#
# Usage:
#   find_package(Detour [REQUIRED] [QUIET])
#
# Once done this will define:
#   DETOUR_FOUND       - system has Detour
#   DETOUR_INCLUDE_DIR - the include directory containing recastnavigation/
#   DETOUR_LIBRARY     - the path to the detour library
#

find_path(DETOUR_INCLUDE_DIR NAMES "recastnavigation/DetourCommon.h")

find_library(DETOUR_LIBRARY NAMES "Detour")
find_library(DETOUR_TILE_LIBRARY NAMES "DetourTileCache")
find_library(DETOUR_CROWD_LIBRARY NAMES "DetourCrowd")

mark_as_advanced(DETOUR_INCLUDE_DIR DETOUR_LIBRARY DETOUR_TILE_LIBRARY DETOUR_CROWD_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Detour DEFAULT_MSG DETOUR_INCLUDE_DIR DETOUR_LIBRARY)

if(DETOUR_INCLUDE_DIR)
  add_library(Detour::DetourMain UNKNOWN IMPORTED GLOBAL)

  set_target_properties(Detour::DetourMain PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${DETOUR_INCLUDE_DIR}"
          IMPORTED_LOCATION "${DETOUR_LIBRARY}")

  # DetourTileCache and DetourCrowd both call into Detour itself -- dtTileCache
  # reaches dtCreateNavMeshData, for one.  Declare that so the link line puts
  # Detour after them: these are static archives, and a member is only pulled in
  # to satisfy a symbol that is already undefined when the archive is scanned.
  # Listed the other way round the link still succeeds, because a shared library
  # may keep undefined symbols, and only fails when the navmesh is first built.
  add_library(Detour::DetourTile UNKNOWN IMPORTED GLOBAL)

  set_target_properties(Detour::DetourTile PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${DETOUR_INCLUDE_DIR}"
          INTERFACE_LINK_LIBRARIES Detour::DetourMain
          IMPORTED_LOCATION "${DETOUR_TILE_LIBRARY}")

  add_library(Detour::DetourCrowd UNKNOWN IMPORTED GLOBAL)

  set_target_properties(Detour::DetourCrowd PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${DETOUR_INCLUDE_DIR}"
          INTERFACE_LINK_LIBRARIES Detour::DetourMain
          IMPORTED_LOCATION "${DETOUR_CROWD_LIBRARY}")

  add_library(Detour::Detour INTERFACE IMPORTED)
  set_property(TARGET Detour::Detour PROPERTY
      INTERFACE_LINK_LIBRARIES Detour::DetourTile Detour::DetourCrowd Detour::DetourMain)
endif()
