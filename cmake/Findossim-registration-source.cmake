if(TARGET ossim_registration_source::ossim-registration-source)
  set(ossim-registration-source_FOUND TRUE)
  return()
endif()

set(_OSSIM_REGISTRATION_SOURCE_HINTS)
if(DEFINED OSSIM_REGISTRATION_SOURCE_ROOT)
  list(APPEND _OSSIM_REGISTRATION_SOURCE_HINTS "${OSSIM_REGISTRATION_SOURCE_ROOT}")
endif()
if(DEFINED ENV{OSSIM_REGISTRATION_SOURCE_ROOT})
  list(APPEND _OSSIM_REGISTRATION_SOURCE_HINTS "$ENV{OSSIM_REGISTRATION_SOURCE_ROOT}")
endif()
list(APPEND _OSSIM_REGISTRATION_SOURCE_HINTS
  "${CMAKE_CURRENT_LIST_DIR}/../../ossim-registration-source")

find_path(OSSIM_REGISTRATION_SOURCE_INCLUDE_DIR
  NAMES ossim/registration/ossimFixedRegistrationSource.h
  HINTS ${_OSSIM_REGISTRATION_SOURCE_HINTS}
  PATH_SUFFIXES include)

find_library(OSSIM_REGISTRATION_SOURCE_LIBRARY
  NAMES ossim-registration-source
  HINTS ${_OSSIM_REGISTRATION_SOURCE_HINTS}
  PATH_SUFFIXES lib64 lib build)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ossim-registration-source
  REQUIRED_VARS
    OSSIM_REGISTRATION_SOURCE_INCLUDE_DIR
    OSSIM_REGISTRATION_SOURCE_LIBRARY)

if(ossim-registration-source_FOUND AND
   NOT TARGET ossim_registration_source::ossim-registration-source)
  get_filename_component(OSSIM_REGISTRATION_SOURCE_INCLUDE_DIR
    "${OSSIM_REGISTRATION_SOURCE_INCLUDE_DIR}" ABSOLUTE)
  get_filename_component(OSSIM_REGISTRATION_SOURCE_LIBRARY
    "${OSSIM_REGISTRATION_SOURCE_LIBRARY}" ABSOLUTE)
  get_filename_component(OSSIM_REGISTRATION_SOURCE_LIBRARY_DIR
    "${OSSIM_REGISTRATION_SOURCE_LIBRARY}" DIRECTORY)

  add_library(ossim_registration_source::ossim-registration-source UNKNOWN IMPORTED)
  set_target_properties(ossim_registration_source::ossim-registration-source PROPERTIES
    IMPORTED_LOCATION "${OSSIM_REGISTRATION_SOURCE_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${OSSIM_REGISTRATION_SOURCE_INCLUDE_DIR}")
endif()

mark_as_advanced(
  OSSIM_REGISTRATION_SOURCE_INCLUDE_DIR
  OSSIM_REGISTRATION_SOURCE_LIBRARY)
