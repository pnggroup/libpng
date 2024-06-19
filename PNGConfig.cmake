include(CMakeFindDependencyMacro)

find_dependency(ZLIB REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/PNGTargets.cmake")

if(NOT TARGET PNG::PNG)
  if(TARGET PNG::png_shared)
    add_library(PNG::PNG INTERFACE IMPORTED)
    target_link_libraries(PNG::PNG INTERFACE PNG::png_shared)
  elseif(TARGET PNG::png_static)
    add_library(PNG::PNG INTERFACE IMPORTED)
    target_link_libraries(PNG::PNG INTERFACE PNG::png_static)
  endif()
endif()
