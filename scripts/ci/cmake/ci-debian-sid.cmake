# Client maintainer: chuck.atkins@kitware.com

execute_process(
  COMMAND /usr/bin/dpkg-architecture -q DEB_HOST_GNU_TYPE
  OUTPUT_VARIABLE arch
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(ENV{CFLAGS} "-Wno-deprecated -Wno-deprecated-declarations")
set(ENV{CXXFLAGS} "-Wno-deprecated -Wno-deprecated-declarations")

set(dashboard_cache "
ADIOS2_USE_EXTERNAL_DEPENDENCIES:BOOL=ON
ADIOS2_USE_EXTERNAL_EVPATH:BOOL=OFF
ADIOS2_USE_BZip2:BOOL=ON
ADIOS2_USE_Blosc:BOOL=ON
ADIOS2_USE_Fortran:BOOL=ON
ADIOS2_USE_HDF5:BOOL=ON
ADIOS2_USE_MPI:BOOL=OFF
ADIOS2_USE_PNG:BOOL=ON
ADIOS2_USE_Python:BOOL=ON
ADIOS2_USE_SSC:BOOL=ON
ADIOS2_USE_SST:BOOL=ON
ADIOS2_USE_ZeroMQ:BOOL=ON
ADIOS2_LIBRARY_SUFFIX:STRING=_serial
ADIOS2_EXECUTABLE_SUFFIX:STRING=.serial
CMAKE_INSTALL_PREFIX:STRING=/usr
CMAKE_INSTALL_LIBDIR:STRING=lib/${arch}
CMAKE_INSTALL_CMAKEDIR:STRING=lib/${arch}/cmake/adios/serial

HDF5_C_COMPILER_EXECUTABLE:FILEPATH=/usr/bin/h5cc
")

set(CTEST_CMAKE_GENERATOR "Ninja")
list(APPEND CTEST_UPDATE_NOTES_FILES "${CMAKE_CURRENT_LIST_FILE}")
include(${CMAKE_CURRENT_LIST_DIR}/ci-common.cmake)
