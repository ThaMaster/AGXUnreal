# A CMake toolchain is a configuration of a set of CMake variables that make
# CMake generate build files in a particular way. We can, for example, set the
# compiler to use, default compile and link flags, and which standard library to
# link against. A toolchain is selected by passing
# `-DCMAKE_TOOLCHAIN_FILE=<path/to/toolchain.cmake>` the _first_ time the CMake
# project is configured.
#
# This particular toolchain tries to configure the toolchain so that binaries
# compatible with Unreal Engine is produced. The flags have been taken from
# https://answers.unrealengine.com/questions/674473/compiling-libraries-in-linux.html
# where RCL, an Epic Games employee, describe how to build third-party libraries.

# Unreal Engine 4.25 added BuildCMakeLib to the Unreal Automation Tool, UAT,
# which may be what we actually need. Read the release notes at
# https://docs.unrealengine.com/en-US/Support/Builds/ReleaseNotes/4_25/index.html
# to learn more.

# We use the UE_ROOT environment variable to find the compiler and standard
# libraries to use. This variable must have been set before running CMake.

# For now this script contains a number of in-source configurations. The intent
# is that all of these will either be removed or made automatic. Search for
# 'CONFIGURATION POINT' to find them.



# Disable implicit NO_POLICY_SCOPE.
#
# CMake used to have `cmake_policy` settings set in a sub-scripts propagate
# into the caller. CMake 3.10 warns on this, By setting this policy to NEW we
# enable policy scoping and the caller regain control over its own policies.
cmake_policy(SET CMP0011 NEW)



# Enable the 'IN_LIST' operator.
#
# I don't know why I need to explicitly specify this all of a sudden. I didn't
# before and IN_LIST has existed for ages, at least since CMake 3.3.
cmake_policy(SET CMP0057 NEW)




# Make sure we know which Unreal Engine installation to use.
if("${UE_ROOT}" STREQUAL "")
  set(UE_ROOT "$ENV{UE_ROOT}")
endif()

if("${UE_ROOT}" STREQUAL "")
  message(FATAL_ERROR "UE_ROOT has not been set.")
endif()


#
# CONFIGURATION POINT: compiler
#
# We can choose to either use the system compiler or the one shipped with Unreal
# Engine. When using the the Unreal Engine compiler one can also set sysroot.
#
# Different Ubuntu distributions have different versions of Clang, so set the
# proper binary name each time the system compiler is to be used.
#
# The long-term solution is to always use the compiler shipped with Unreal Engine.
#
# Pick one:
set(CONFIG_COMPILER "UNREAL")
#set(CONFIG_COMPILER "UNREAL_WITH_SYSROOT")
#set(CONFIG_COMPILER "SYSTEM")

# When CONFIG_COMPILER is set to 'SYSTEM', this is the compiler that will be used.
set(CONFIG_COMPILER_SYSTEM_C "clang-9")
set(CONFIG_COMPILER_SYSTEM_CXX "clang++-9")

set(CONFIG_OPTIONS_COMPILER "UNREAL" "UNREAL_WITH_SYSROOT" "SYSTEM") #
if(NOT "${CONFIG_COMPILER}" IN_LIST CONFIG_OPTIONS_COMPILER)
  message(FATAL_ERROR "CONFIG_COMPILER must be one of ${CONFIG_OPTIONS_COMPILER}.")
endif()


#
# CONFIGURATION POINT: compiler flags
#
# We have a list of compiler flags that we want to pass when compiling C++
# source files but not when compiling other file types. Recent CMake (3.12) has
# support for making this automatic, with the COMPILE_LANGUAGE generator
# expression, but that isn't available on some of the older CMake version we
# must support. For now we provide a configuration point that should be set
# according to the language of the library being built.
#
# Pick one:
#set(CONFIG_COMPILER_FLAGS "C")
set(CONFIG_COMPILER_FLAGS "CXX")

set(CONFIG_OPTIONS_COMPILER_FLAGS "C" "CXX")
if(NOT "${CONFIG_COMPILER_FLAGS}" IN_LIST CONFIG_OPTIONS_COMPILER_FLAGS)
  message(FATAL_ERROR "CONFIG_COMPILER_FLAGS must be one of ${CONFIG_OPTIONS_COMPILER_FLAGS}.")
endif()


#
# CONFIGURATION POINT: Unreal Engine system libraries to CMake
#
# This informs CMake of the Unreal Engine system libraries, so that
# 'find_package' can find them. This is strongly related to sysroot and system
# libraries in linker search paths, I think.
#
# Pick one:
set(CONFIG_UE_LIBS_TO_CMAKE "ON")
#set(CONFIG_UE_LIBS_TO_CMAKE "OFF")

set(CONFIG_OPTIONS_UE_LIBS_TO_CMAKE "ON" "OFF")
if(NOT "${CONFIG_UE_LIBS_TO_CMAKE}" IN_LIST CONFIG_OPTIONS_UE_LIBS_TO_CMAKE)
  message(FATAL_ERROR "CONFIG_UE_LIBS_TO_CMAKE must be one of ${CONFIG_OPTIONS_UE_LIBS_TO_CMAKE}.")
endif()


#
# CONFIGURATION POINT: Include Unreal Engine system libraries in linker search paths
#
# Tell the linker to search for libraries in the Unreal Engine compiler
# directories. That's where we have 'm', 'c', 'rt', and such.
#
# Pick one:
set(CONFIG_UE_LIBS_TO_LINKER "ON")
#set(CONFIG_UE_LIBS_TO_LINKER "OFF")

set(CONFIG_OPTIONS_UE_LIBS_TO_LINKER "ON" "OFF")
if(NOT "${CONFIG_UE_LIBS_TO_LINKER}" IN_LIST CONFIG_OPTIONS_UE_LIBS_TO_LINKER)
  message(FATAL_ERROR "CONFIG_UE_LIBS_TO_LINKER must be one of ${CONFIG_OPTIONS_UE_LIBS_TO_LINKER}.")
endif()

# Construct paths to various directories within the Unreal Engine installation.
set(ue_libcxx_dir "${UE_ROOT}/Engine/Source/ThirdParty/Linux/LibCxx")
set(ue_libcxx_libdir "${ue_libcxx_dir}/lib/Linux/x86_64-unknown-linux-gnu")
set(ue_libcxx_incdir "${ue_libcxx_dir}/include")
set(ue_compiler_subdir "Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64")
file(GLOB ue_compiler_name
      LIST_DIRECTORIES true
      RELATIVE "${UE_ROOT}/${ue_compiler_subdir}"
      "${UE_ROOT}/${ue_compiler_subdir}/v*_clang-*-centos*")
set(ue_compiler_dir "${UE_ROOT}/${ue_compiler_subdir}/${ue_compiler_name}/x86_64-unknown-linux-gnu")



# Tell CMake about the Unreal Engine compiler, both the path to the binaries and
# the sysroot.
if("${CONFIG_COMPILER}" STREQUAL "UNREAL")
  set(CMAKE_C_COMPILER "${ue_compiler_dir}/bin/clang" CACHE STRING "The C compiler to use.")
  set(CMAKE_CXX_COMPILER "${ue_compiler_dir}/bin/clang++" CACHE STRING "The C++ compiler to use.")
elseif("${CONFIG_COMPILER}" STREQUAL "UNREAL_WITH_SYSROOT")
  set(CMAKE_C_COMPILER "${ue_compiler_dir}/bin/clang" CACHE STRING "The C compiler to use.")
  set(CMAKE_CXX_COMPILER "${ue_compiler_dir}/bin/clang++" CACHE STRING "The C++ compiler to use.")
  set(CMAKE_SYSROOT "${ue_compiler_dir}" CACHE STRING "The root directory of the compiler installation.")
elseif("${CONFIG_COMPILER}" STREQUAL "SYSTEM")
  # Tell CMake to use the system compiler. Make sure the version is correct,
  # whatever that means.
  set(CMAKE_C_COMPILER "${CONFIG_COMPILER_SYSTEM_C}" CACHE STRING "The C compiler to use.")
  set(CMAKE_CXX_COMPILER "${CONFIG_COMPILER_SYSTEM_CXX}" CACHE STRING "The C++ compiler to use.")
else()
  message(FATAL_ERROR "Unknown compiler selection '${CONFIG_COMPILER}'.")
endif()






# This part is buggy. I want to enable C++ flags for C++ sources only, but on
# CMake < 3.12 the COMPILE_LANGUAGE:CXX throws an error because for C-only
# projects the CXX language isn't loaded. I therefore added the
# CMAKE_CXX_COMPILER_LOADED check so that we only check for CXX when we know
# about the language. This also fails because while enabling CXX, which is what
# we're currently doing, CXX hasn't yet been loaded so the check is always
# false. One might think that an 'enable_language(CXX)' call would fixed that,
# but that's not legal to call here because we may be in the process or enabling
# C++ right now.  See https://gitlab.kitware.com/cmake/cmake/-/issues/17952
#
# Cannot use CMAKE_CXX_FLAGS because those flags are also passed to the linker
# and we don't want that. (Are they really?)
#
# I don't rember why we can't just pass the C++ flags to all compilers,
# i.e. also for C files. For now I just comment all of this out and provide a
# configuration point instead.
#
#
# Disable system include paths and add compiler flags to use the standard
# library header files shipped with Unreal Engine when building C++ source
# files.
#set(ue_compiler_flags  -nostdinc++ -I${ue_libcxx_incdir} -I${ue_libcxx_incdir}/c++/v1)
#add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${ue_compiler_flags}>")

if("${CONFIG_COMPILER_FLAGS}" STREQUAL "C")
  message(STATUS "Not a C++ project, not passing C++ compiler flags.")
elseif("${CONFIG_COMPILER_FLAGS}" STREQUAL "CXX")
  message(STATUS "Is a C++ project, passing C++ compiler flags.")
  set(ue_compiler_flags  -nostdinc++ -I${ue_libcxx_incdir} -I${ue_libcxx_incdir}/c++/v1)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${ue_compiler_flags}>")
else()
  message(FATAL_ERROR "Unknown compiler flags selection: '${CONFIG_COMPILER_FLAGS}'.")
endif()


if("${CONFIG_UE_LIBS_TO_CMAKE}" STREQUAL "ON")
  list(APPEND CMAKE_PREFIX_PATH
    "${ue_compiler_dir}"
    "${ue_compiler_dir}/usr/lib"
    "${ue_compiler_dir}/usr/lib64")
  list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH)
elseif("${CONFIG_UE_LIBS_TO_CMAKE}" STREQUAL "OFF")
  # Intentionally empty.
else()
  message(FATAL_ERROR "Unknown Unreal Engine libraries to CMake selection: '${CONFIG_UE_LIBS_TO_CMAKE}'.")
endif()



# Disable linking with the system standard library and setup linker directories
# to the Unreal Engine paths instead.
set(ue_linker_flags "-nodefaultlibs -L${ue_libcxx_libdir}")



if("${CONFIG_UE_LIBS_TO_LINKER}" STREQUAL "ON")
  set(ue_linker_flags "${ue_linker_flags} -L${ue_compiler_dir} -L${ue_compiler_dir}/usr/lib -L${ue_compiler_dir}/usr/lib64")
elseif("${CONFIG_UE_LIBS_TO_LINKER}" STREQUAL "OFF")
  # Intentionally empty.
else()
  message(FATAL_ERROR "Unknown Unreal Engine libraries to linker selection: '${CONFIG_UE_LIBS_TO_LINKER}'.")
endif()


if(NOT CMAKE_EXE_LINKER_FLAGS MATCHES "${ue_libcxx_libdir}")
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ue_linker_flags}" CACHE INTERNAL "")
endif()
if(NOT CMAKE_MODULE_LINKER_FLAGS MATCHES "${ue_libcxx_libdir}")
  set(CMAKE_MODULE_LINKER_FLAGS  "${CMAKE_MODULE_LINKER_FLAGS} ${ue_linker_flags}" CACHE INTERNAL "")
endif()
if(NOT CMAKE_SHARED_LINKER_FLAGS MATCHES "${ue_libcxx_libdir}")
  set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} ${ue_linker_flags}" CACHE INTERNAL "")
endif()

# Including static libraries here causes `-nodefaultlibs` to be passed to `ar`,
# which errors out because it dosn't have a `-n` parameter. Static libraries
# doesn't really have a proper link stage, so the linker flags doesn't really
# make sense.
# if(NOT CMAKE_STATIC_LINKER_FLAGS MATCHES "${ue_libcxx_libdir}")
#   set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} ${ue_linker_flags}" CACHE INTERNAL "")
# endif()

# Enable usage of the Unreal Engine standard libraries.
set(c_libraries "-lm -lc -lgcc_s -lgcc -lpthread")
set(CMAKE_C_STANDARD_LIBRARIES  "${c_libraries}" CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD_LIBRARIES  "${ue_libcxx_libdir}/libc++.a  ${ue_libcxx_libdir}/libc++abi.a  ${c_libraries}" CACHE INTERNAL "")

# I don't know why, but CMake fails to detect CMAKE_SIZEOF_VOID_P and it ends up
# being the empty string. Here we assume that we are building with 8-byte
# pointers, i.e., 64-bit mode.
set(CMAKE_SIZEOF_VOID_P 8 CACHE INTERNAL "")
