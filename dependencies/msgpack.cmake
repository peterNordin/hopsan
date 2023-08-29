set(local_msgpack_dir ${CMAKE_CURRENT_LIST_DIR}/msgpack-c)
find_package(msgpack CONFIG PATHS ${local_msgpack_dir} NO_DEFAULT_PATH) # Search for local version
if (NOT msgpack_FOUND)
  message(STATUS "Looking for msgpack in system")
  find_package(msgpack CONFIG) # Search in system
  if (msgpack_FOUND)
    message(STATUS "  Found msgpack")
    if (NOT TARGET msgpackc-cxx)
      add_library(msgpackc-cxx INTERFACE)
      target_link_libraries(msgpackc-cxx INTERFACE msgpackc)
    endif()
  else()
    message(WARNING "Could not find msgpack")
  endif()
else()
  message(STATUS "Found local msgpack")
endif()
