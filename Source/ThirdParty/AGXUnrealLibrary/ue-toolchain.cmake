# A CMake toolchain is a configuration of a set of CMake variables that make CMake generate build files in a particular
# way. We can, for example, set the compiler to use, default compile and link flags, and which standard library to link
# against. A toolchain is selected by passing `-DCMAKE_TOOLCHAIN_FILE=<path/to/toolchain.cmake>` the _first_ time the
# the CMake project is configured.
#
# This particular toolchain tries to configure the toolchain so that binaries compatible with Unreal Engine 4.22 is
# produced. The flags have been taken from https://answers.unrealengine.com/questions/674473/compiling-libraries-in-linux.html

# Unreal Engine 4.25 added BuildCMakeLib to the Unreal Automation Tool, UAT,
# which may be what we actually need. Read the release notes at
# https://docs.unrealengine.com/en-US/Support/Builds/ReleaseNotes/4_25/index.html

# Unreal Engine 4.22 uses clang-7. We only name the compiler, instead of giving a full path to the binaires downloaded
# by the Unreal Engine Setup.sh script. This makes it use the system binaries. We may want to try using the Unreal
# Engine specific binaries at some point.
set(CMAKE_C_COMPILER "clang-7" CACHE STRING "The C compiler to use.")
set(CMAKE_CXX_COMPILER "clang++-7" CACHE STRING "The C++ compiler to use.")

# Disable system include paths and add the Unreal Engine paths instead.
if(NOT CMAKE_CXX_FLAGS MATCHES "Linux/LibCxx/include")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -nostdinc++ -I$ENV{UE_THIRD_PARTY_DIR}/Linux/LibCxx/include/ -I$ENV{UE_THIRD_PARTY_DIR}/Linux/LibCxx/include/c++/v1" CACHE INTERNAL "")
endif()

# Disable linking with the system standard library and setup linker directories to the Unreal Engine paths instead.
set(UE_LINKER_FLAGS "-nodefaultlibs -L$ENV{UE_THIRD_PARTY_DIR}/Linux/LibCxx/lib/Linux/x86_64-unknown-linux-gnu/")
if (NOT CMAKE_EXE_LINKER_FLAGS MATCHES "-nodefaultlibs")
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${UE_LINKER_FLAGS}" CACHE INTERNAL "")
endif()
if (NOT CMAKE_MODULE_LINKER_FLAGS MATCHES "-nodefaultlibs")
  set(CMAKE_MODULE_LINKER_FLAGS  "${CMAKE_MODULE_LINKER_FLAGS} ${UE_LINKER_FLAGS}" CACHE INTERNAL "")
endif()
if (NOT CMAKE_SHARED_LINKER_FLAGS MATCHES "-nodefaultlibs")
  set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} ${UE_LINKER_FLAGS}" CACHE INTERNAL "")
endif()
# todo Should CMAKE_STATIC_LINKER_FLAGS be added here as well?

# Enable usage of the Unreal Engine standard libraries.
set(CMAKE_C_STANDARD_LIBRARIES  "-lm -lc -lgcc_s -lgcc -lpthread" CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD_LIBRARIES  "$ENV{UE_THIRD_PARTY_DIR}/Linux/LibCxx/lib/Linux/x86_64-unknown-linux-gnu/libc++.a  $ENV{UE_THIRD_PARTY_DIR}/Linux/LibCxx/lib/Linux/x86_64-unknown-linux-gnu/libc++abi.a  -lm -lc -lgcc_s -lgcc -lpthread" CACHE INTERNAL "")

# CMake fails to detect CMAKE_SIZEOF_VOID_P and it ends up being the empty
# string. Here we assume that we are building with 8-byte pointers, i.e., 64-bit
# mode.
set(CMAKE_SIZEOF_VOID_P 8 CACHE INTERNAL "")
