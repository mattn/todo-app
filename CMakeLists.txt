cmake_minimum_required(VERSION 3.20)

#IF(WIN32)
#  IF("$ENV{OSTYPE}" STREQUAL "msys")
#    set_property(GLOBAL PROPERTY _IMPORT_PREFIX "C:/msys64/mingw64")
#  ENDIF("$ENV{OSTYPE}" STREQUAL "msys")
#ENDIF(WIN32)
IF(WIN32)
  include("C:/msys64/mingw64/lib/cmake/soci/SOCI.cmake")
ELSE(WIN32)
  include("cmake/soci/SOCI.cmake")
ENDIF(WIN32)

project(todo)

add_executable(todo main.cxx)
target_link_libraries(todo PRIVATE SOCI::soci_postgresql SOCI::soci_core)
