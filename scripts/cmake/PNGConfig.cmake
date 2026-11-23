# PNGConfig.cmake
# CMake config file compatible with the FindPNG module.

# Copyright (c) 2024-2026 Cosmin Truta
# Written by Benjamin Buch, 2024
#
# Use, modification and distribution are subject to
# the same licensing terms and conditions as libpng.
# Please see the copyright notice in png.h or visit
# http://libpng.org/pub/png/src/libpng-LICENSE.txt
#
# SPDX-License-Identifier: libpng-2.0

include(CMakeFindDependencyMacro)

# Avoid using REQUIRED here.
# find_dependency(ZLIB) forwards REQUIRED and QUIET from find_package(PNG),
# which allows an optional find_package(PNG) to fail softly.
find_dependency(ZLIB 1.2.8)

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
