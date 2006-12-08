project(libpng)

SET(PNG_VER_MAJOR 1)
SET(PNG_VER_MINOR 2)
SET(PNG_VER_PATCH 15)

# needed packages
find_package(ZLIB REQUIRED)


# command line options
option(PNG_SHARED "Build shared lib" YES)
option(PNG_TESTS  "Build pngtest" YES)
#TODO:
# PNG_NO_CONSOLE_IO
# PNG_NO_STDIO
# PNG_CONSOLE_IO_SUPPORTED
# PNGARG
# some others :)

set(png_asm_tmp "OFF")
if(NOT WIN32)
  find_program(uname_executable NAMES uname PATHS /bin /usr/bin /usr/local/bin)
  if(uname_executable)
    EXEC_PROGRAM(${uname_executable} ARGS --machine OUTPUT_VARIABLE uname_output)
    if("uname_output" MATCHES "^.*i[1-9]86.*$")
       set(png_asm_tmp "ON")
    else("uname_output" MATCHES "^.*i[1-9]86.*$")
       set(png_asm_tmp "OFF")
    endif("uname_output" MATCHES "^.*i[1-9]86.*$")
  endif(uname_executable)
endif(NOT WIN32)

option(PNG_MMX "Use MMX assembler code (x86 only)" ${png_asm_tmp})


# msvc does not append 'lib' - do it here to have consistent name
if(MSVC)
  set(PNG_LIB_NAME lib)
endif(MSVC)
#set(PNG_LIB_NAME ${PNG_LIB_NAME}png-${PNG_VER_MAJOR}.${PNG_VER_MINOR})
set(PNG_LIB_NAME ${PNG_LIB_NAME}png)

# to distinguish between debug and release lib
set(CMAKE_DEBUG_POSTFIX "d")

# append _static to static lib
if(NOT PNG_SHARED)
  set(PNG_LIB_NAME ${PNG_LIB_NAME}_static)
endif(NOT PNG_SHARED)


# our sources
set(libpng_sources
  png.h
  pngconf.h
  png.c
  pngerror.c
  pngget.c
  pngmem.c
  pngpread.c
  pngread.c
  pngrio.c
  pngrtran.c
  pngrutil.c
  pngset.c
  pngtrans.c
  pngwio.c
  pngwrite.c
  pngwtran.c
  pngwutil.c
)
set(pngtest_sources
        pngtest.c
)


# Some needed definitions
add_definitions(-DZLIB_DLL)

if(MSVC)
  add_definitions(-DPNG_USE_PNGVCRD -DPNG_NO_MODULEDEF -D_CRT_SECURE_NO_DEPRECATE)
  set(libpng_sources ${libpng_sources}
          pngvcrd.c
  )
else(MSVC)
  add_definitions(-DPNG_USE_PNGGCCRD -DPNG_USE_GLOBAL_ARRAYS)
  set(libpng_sources ${libpng_sources}
          pnggccrd.c
  )
endif(MSVC)

if(NOT PNG_MMX)
  add_definitions(-DPNG_NO_MMX_CODE)
endif(NOT PNG_MMX)

if(NOT WIN32)
  find_library(M_LIBRARY
      NAMES m
      PATHS /usr/lib /usr/local/lib
  )
endif(NOT WIN32)

# now build our target
include_directories(${CMAKE_SOURCE_DIR} ${ZLIB_INCLUDE_DIR})

if(PNG_SHARED)
  add_library(${PNG_LIB_NAME} SHARED ${libpng_sources})
else(PNG_SHARED)
  add_library(STATIC ${libpng_sources})
endif(PNG_SHARED)

target_link_libraries(${PNG_LIB_NAME} ${ZLIB_LIBRARY} ${M_LIBRARY})
if(PNG_SHARED AND WIN32)
  set_target_properties(${PNG_LIB_NAME} PROPERTIES DEFINE_SYMBOL PNG_BUILD_DLL)
endif(PNG_SHARED AND WIN32)

if(PNG_TESTS)
# does not work with msvc due to png_lib_ver issue
  add_executable(pngtest ${pngtest_sources})
  target_link_libraries(pngtest ${PNG_LIB_NAME})
#  add_test(pngtest ${CMAKE_SOURCE_DIR}/pngtest.png)
endif(PNG_TESTS)


# install
install_targets(/bin ${PNG_LIB_NAME})
install(FILES libpng.so libpng.a DESTINATION lib)
install(FILES png.h pngconf.h DESTINATION include)
install(FILES libpng.3 libpngpf.3 DESTINATION man/man3)
# what's with libpng.txt and all the extra files?


# uninstall
# do we need this?


# dist
# do we need this?

# to create msvc import lib for mingw compiled shared lib
# pexports libpng.dll > libpng.def
# lib /def:libpng.def /machine:x86

 	  	 

 	  	 
