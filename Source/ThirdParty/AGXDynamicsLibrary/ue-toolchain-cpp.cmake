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
# where RCL, an Epic Games employee, describe how to build third-party libraries,
# and from the output of [ue4cli](https://docs.adamrehn.com/ue4cli/overview/introduction-to-ue4cli).

# Unreal Engine 4.25 added BuildCMakeLib to the Unreal Automation Tool, UAT,
# which may be what we actually need. Read the release notes at
# https://docs.unrealengine.com/en-US/Support/Builds/ReleaseNotes/4_25/index.html
# to learn more.
#
# I had a look it I couldn't see that it does the necessary configuration. To
# me it looks like it assumes that it is being run on a compatible host system,
# i.e., that the system compilers and libraries are Unreal Engine compatible.

# We use the UE_ROOT environment variable to find the compiler and standard
# libraries to use. This variable must have been set before running CMake.
# Altenatively, it can be passed on the CMake command line as
#   -DUE_ROOT=<PATH_TO_UNREAL_ENGINE>

# For now this script contains a number of in-source configurations. The intent
# is that all of these will either be removed or made automatic. Search for
# 'CONFIGURATION POINT' to find them.



# Things that need to be configured:
#
# CMake flags
#
# The behavior of CMake can be controlled using various flags. This affects
# things like where CMake will search for files. By default it searches in the
# system directories, but we may want to have it search in the Unreal Engine
# compiler directory as well.
#
# Compiler
#
# Unreal Engine has been built with a particular version of Clang and we wish to
# match this. We can build either using a system installed Clang or the one in
# the Unreal Engine installation.
#
# Compiler flags
#
# The flags that are passed when source files are built. This includes things
# like include paths, preprocessor defines, and ... what else? We may want to
# add the include directory for the Clang installation in the Unreal Engine
# installation.
#
# Linker flags
#
# The flags that are passed when a build target is being linked.This incldues
# things like dependency libraries and directories in which to search for them.
# We may want to add the `lib` directory for the Clang installation in the
# Unreal Engine installation.
#





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



#
# Make sure we know which Unreal Engine installation to use.
#

# Fall back to environment variable if not set in or pass to CMake explicitly.
if("${UE_ROOT}" STREQUAL "")
  set(UE_ROOT "$ENV{UE_ROOT}")
endif()

# By now we must have an Unreal Engine installation.
if("${UE_ROOT}" STREQUAL "")
  message(FATAL_ERROR "UE_ROOT has not been set.")
endif()




###
### CONFIGURATION POINTS
###


#
# CONFIGURATION POINT: compiler.
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
#set(CONFIG_COMPILER "UNREAL")
set(CONFIG_COMPILER "UNREAL_WITH_SYSROOT")
#set(CONFIG_COMPILER "SYSTEM")

# When CONFIG_COMPILER is set to 'SYSTEM', this is the compiler that will be used.
set(CONFIG_COMPILER_SYSTEM_C "clang-9")
set(CONFIG_COMPILER_SYSTEM_CXX "clang++-9")

set(CONFIG_OPTIONS_COMPILER "UNREAL" "UNREAL_WITH_SYSROOT" "SYSTEM")
if(NOT "${CONFIG_COMPILER}" IN_LIST CONFIG_OPTIONS_COMPILER)
  message(FATAL_ERROR "CONFIG_COMPILER must be one of ${CONFIG_OPTIONS_COMPILER}.")
endif()




#
# CONFIGURATION POINT: Unreal Engine C system libraries to CMake.
#
# This informs CMake, using CMAKE_PREFIX_PATH, of the C system libraries included
# with the Clang installation in the Unreal Engine installation, so that
# 'find_package' can find them. This is strongly related to sysroot and system
# libraries in linker search paths, I think, in that this part should be OFF
# when CONFIG_COMPILER is set to UNREAL_WITH_SYSROOT.
#
# I'm not sure if this should be ON or OFF when using a system Clang. Are these
# tightly coupled with a particular Clang version?
#
# The C system libraries are stored in various subdirectories of
#  ${UE_ROOT}/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v16_clang-9.0.1-centos7/x86_64-unknown-linux-gnu/
# We sometimes refer to this directory as `UE_CLANG`.
#
# Pick one:
#set(CONFIG_UE_C_LIBS_TO_CMAKE "ON")
set(CONFIG_UE_C_LIBS_TO_CMAKE "OFF")

set(CONFIG_OPTIONS_UE_C_LIBS_TO_CMAKE "ON" "OFF")
if(NOT "${CONFIG_UE_C_LIBS_TO_CMAKE}" IN_LIST CONFIG_OPTIONS_UE_C_LIBS_TO_CMAKE)
  message(FATAL_ERROR "CONFIG_UE_C_LIBS_TO_CMAKE must be one of ${CONFIG_OPTIONS_UE_C_LIBS_TO_CMAKE}.")
endif()




#
# CONFIGURATION POINT: Include Unreal Engine C system libraries in linker search paths.
#
# Tell the linker to search for libraries in the Unreal Engine compiler
# directories. That's where we have 'm', 'c', 'rt', and such.
#
# This will enable the flags printed by `ue4 ldflags libc++`.
# I worry that this isn't quite true. The `ue4 ldflags` only print C++ standard
# libraries, not the C libraries. So I think those will still be picked up from
# the host system if only passing the ue4cli flags.
#
# I think this relates to sysroot and CONFIG_UE_C_LIBS_TO_CMAKE.
#
# Pick one:
#set(CONFIG_UE_C_LIBS_TO_LINKER "ON")
set(CONFIG_UE_C_LIBS_TO_LINKER "OFF")

set(CONFIG_OPTIONS_UE_C_LIBS_TO_LINKER "ON" "OFF")
if(NOT "${CONFIG_UE_C_LIBS_TO_LINKER}" IN_LIST CONFIG_OPTIONS_UE_C_LIBS_TO_LINKER)
  message(FATAL_ERROR "CONFIG_UE_C_LIBS_TO_LINKER must be one of ${CONFIG_OPTIONS_UE_C_LIBS_TO_LINKER}.")
endif()



# Construct paths to various directories within the Unreal Engine installation.

# ue_compiler_list_dir:
#   Contains a list of compiler installations in subdirectories.
#   We assume, for now, that there is only one.
set(ue_compiler_list_dir "${UE_ROOT}/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64")

# ue_compiler_dir:
#   The root directory for a particular Clang installation.
#   Contains `bin`, `include`, `lib`, `lib64`, `share`, and `usr` for that compiler.
#   This is the path that will be set when as sysroot when CONFIG_COMPILER is UNREAL_WITH_SYSROOT.
#   Subdirectories of this is passed to CMAKE_PREFIX_PATH when CONFIG_UE_C_LIBS_TO_CMAKE in ON.
#   Sometimes referred to as `UE_CLANG` in text.
file(GLOB ue_compiler_name
      LIST_DIRECTORIES true
      RELATIVE "${ue_compiler_list_dir}"
      "${ue_compiler_list_dir}/v*_clang-*-centos*")
set(ue_compiler_dir "${ue_compiler_list_dir}/${ue_compiler_name}/x86_64-unknown-linux-gnu")

# ue_libcxx_dir:
#   The root directory of the Unreal Engine libc++ C++ standard library implementation.
set(ue_libcxx_dir "${UE_ROOT}/Engine/Source/ThirdParty/Linux/LibCxx")

# ue_libcxx_incdir:
#   The include directory for the Unreal Engine standard library implementation.
#   Pass this as one of the `-I` parameters to the compiler.
set(ue_libcxx_incdir "${ue_libcxx_dir}/include")

# ue_libcxx_libdir:
#   The library directory within the Unreal Engine standard library implementation.
#   Pass this as one of the `-L` parameters to the linker.
set(ue_libcxx_libdir "${ue_libcxx_dir}/lib/Linux/x86_64-unknown-linux-gnu")


###
### CMAKE
###
### This section sets variables controlling the behavior of CMake.
###


# This part tells CMake where to look for files with `find_package`,
# `find_file`, find_library` and the like.
#
# This first part is based completely on the output from ue4cli:
#    ➤ ue4 cmakeflags --multiline --nodefaults libc++
#    -DCMAKE_PREFIX_PATH=
#    -DCMAKE_INCLUDE_PATH=${UE_ROOT}/Engine/Source/ThirdParty/Linux/LibCxx/include;${UE_ROOT}/Engine/Source/ThirdParty/Linux/LibCxx/include/c++/v1
#    -DCMAKE_LIBRARY_PATH=${UE_ROOT}/Engine/Source/ThirdParty/Linux/LibCxx/lib/Linux/x86_64-unknown-linux-gnu
list(APPEND CMAKE_INCLUDE_PATH "${ue_libcxx_incdir}" "${ue_libcxx_incdir}/c++/v1")
list(APPEND CMAKE_LIBRARY_PATH "${ue_libcxx_libdir}")



#
# This bit is not necessary when using SYSROOT. I hope.
#
# This entire thing has me worried. ue4cli only mentions C++ paths in
# ${UE_ROOT}/Engine/Source/ThirdParty/Linux/LibCxx/
# It never talks about the C paths in
# ${UE_ROOT}/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v16_clang-9.0.1-centos7/x86_64-unknown-linux-gnu
# called `ue_compiler_dir` in the code below.
#
# Perhaps ue4cli assume either that we build from a C-compatible host system, or
# that sysroot is used.
#
# I would really like to only build with CONFIG_UE_C_LIBS_TO_CMAKE = OFF.
if("${CONFIG_UE_C_LIBS_TO_CMAKE}" STREQUAL "ON")
  list(APPEND CMAKE_PREFIX_PATH
    "${ue_compiler_dir}"
    "${ue_compiler_dir}/include"
    "${ue_compiler_dir}/lib/gcc/x86_64-unknown-linux-gnu/4.8.5"  # For libgcc.a.
    "${ue_compiler_dir}/lib64"  # For libgcc_s.so.
    "${ue_compiler_dir}/usr/lib"  # For libc.a, libm.a, and libpthread.a.
    "${ue_compiler_dir}/usr/lib64"  # For libc.a, libm.a, and libpthread.a again. The files are identical.
  )
  list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH)
elseif("${CONFIG_UE_C_LIBS_TO_CMAKE}" STREQUAL "OFF")
  # Intentionally empty.
else()
  message(FATAL_ERROR "Unknown Unreal Engine libraries to CMake selection: '${CONFIG_UE_C_LIBS_TO_CMAKE}'.")
endif()


###
### COMPILER
###
### This section sets variables controlling which compiler to use.
###


# Tell CMake about the Unreal Engine compiler, both the path to the binaries
# and, if requested, the sysroot.
if("${CONFIG_COMPILER}" STREQUAL "UNREAL")
  set(CMAKE_CXX_COMPILER "${ue_compiler_dir}/bin/clang++" CACHE STRING "The CXX compiler to use.")

  # CMake require there to be a C compiler even for C++ projects.
  set(CMAKE_C_COMPILER "${ue_compiler_dir}/bin/clang" CACHE STRING "The C compiler to use.")
elseif("${CONFIG_COMPILER}" STREQUAL "UNREAL_WITH_SYSROOT")
  # CHECK: Should I pass the full path to the compiler, i.e. include `${ue_compiler_dir}`, or is the
  #        path here relative to `CMAKE_SYSROOT`?
  set(CMAKE_CXX_COMPILER "${ue_compiler_dir}/bin/clang++" CACHE STRING "The CXX compiler to use.")
  set(CMAKE_SYSROOT "${ue_compiler_dir}" CACHE STRING "The root directory of the compiler installation.")

  # CMake require there to be a C compiler even for C++ projects.
  set(CMAKE_C_COMPILER "${ue_compiler_dir}/bin/clang" CACHE STRING "The C compiler to use.")
elseif("${CONFIG_COMPILER}" STREQUAL "SYSTEM")
  # Tell CMake to use the system compiler. Make sure the version is correct,
  # whatever that means.
  set(CMAKE_CXX_COMPILER "${CONFIG_COMPILER_SYSTEM_CXX}" CACHE STRING "The CXX compiler to use.")

    # CMake require there to be a C compiler even from C++ projects.
  set(CMAKE_C_COMPILER "${CONFIG_COMPILER_SYSTEM_C}" CACHE STRING "The C compiler to use.")
else()
  message(FATAL_ERROR "Unknown compiler selection '${CONFIG_COMPILER}'.")
endif()



###
### COMPILER FLAGS
###
### This section sets variables controlling flags passed to the compile step.
###

# We base the compiler flags on the output from `ue4 cxxflags libc++`:
# ➤ ue4 cxxflags --multiline --nodefaults libc++
#   -I${UE_LIBCXX}/include
#   -I${UE_LIBCXX}/include/c++/v1
#   -fPIC
#   -nostdinc++
set(ue_compiler_flags -fPIC -nostdinc++ -I${ue_libcxx_incdir} -I${ue_libcxx_incdir}/c++/v1)
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${ue_compiler_flags}>")



###
### LINKER FLAGS
###
### This section sets varaibles controlling flags passed to the link step.
###

# This variable is filled according to the configuration settings and then
# passed to the various CMAKE_*_LINKER_FLAGS variables.
set(ue_linker_flags "")

# We base the compiler flags on the output from `ue4 ldflags libc++`:
#   -nodefaultlibs
#   -L${UE_LIBCXX}
#   ${UE_LIBCXX}/libc++.a
#   ${UE_LIBCXX}/libc++abi.a
#   -lm
#   -lc
#   -lgcc_s
#   -lgcc
set(ue_linker_flags "-nodefaultlibs -L${ue_libcxx_libdir} ${ue_libcxx_libdir}/libc++.a ${ue_libcxx_libdir}/libc++abi.a -lm -lc -lgcc_s -lgcc")

if("${CONFIG_UE_C_LIBS_TO_LINKER}" STREQUAL "ON")
  # Enabling CONFIG_UE_C_LIBS_TO_CMAKE passes more directories to CMake. Should this as well?
  set(ue_linker_flags "${ue_linker_flags} -L${ue_compiler_dir} -L${ue_compiler_dir}/usr/lib -L${ue_compiler_dir}/usr/lib64")
elseif("${CONFIG_UE_C_LIBS_TO_LINKER}" STREQUAL "OFF")
  # Intentionally empty.
else()
  message(FATAL_ERROR "Unknown Unreal Engine libraries to linker selection: '${CONFIG_UE_C_LIBS_TO_LINKER}'.")
endif()



set(CMAKE_C_STANDARD_LIBRARIES  "-nodefaultlibs -lc -lgcc_s -lgcc" CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD_LIBRARIES  "${ue_linker_flags}" CACHE INTERNAL "")

###
### This block has been disabled because it require CMake 3.13 and I only have
### 3.10. Not sure which approach is better, `add_link_options` with a generator
### expression or `CMAKE_*_STANDARD_LIBRARIES` used above.
###
#  add_link_options("$<$<COMPILE_LANGUAGE:CXX>:${ue_linker_flags}>")
#  add_link_options("$<$<COMPILE_LANGUAGE:C>:-nodefaultlibs>")

###
### This block has been disabled because it send the linker flags to both C and
### C++ targets. Understandably, passing the C++ standard library to a C linker
### doesn't end well.
###
### Instead of CMAKE_*_LINKER_FLAGS we use add_linker_options with a generator
### expression to separate flags for C and C++ targets.
###
#  # The MATCHES check is to prevent adding these flags multiple times.
#  # Is that really needed? Can we do something with REMOVE_DUPLICATES instead if we really need to do something?
#  if(NOT CMAKE_EXE_LINKER_FLAGS MATCHES "-nodefaultlibs")
#    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ue_linker_flags}" CACHE INTERNAL "")
#  endif()
#  if(NOT CMAKE_MODULE_LINKER_FLAGS MATCHES "-nodefaultlibs")
#    set(CMAKE_MODULE_LINKER_FLAGS  "${CMAKE_MODULE_LINKER_FLAGS} ${ue_linker_flags}" CACHE INTERNAL "")
#  endif()
#  if(NOT CMAKE_SHARED_LINKER_FLAGS MATCHES "-nodefaultlibs")
#    set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} ${ue_linker_flags}" CACHE INTERNAL "")
#  endif()


# Including static libraries here causes `-nodefaultlibs` to be passed to `ar`,
# which errors out because it dosn't have a `-n` parameter. Static libraries
# doesn't really have a proper link stage, so the linker flags doesn't really
# make sense.
# if(NOT CMAKE_STATIC_LINKER_FLAGS MATCHES "-nodefaultlibs")
#   set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} ${ue_linker_flags}" CACHE INTERNAL "")
# endif()



# Enable usage of the Unreal Engine standard libraries.
#
# Not sure if these should go to CMAKE_C_STANDARD_LIBRARIES or via ue_linker_flags to CMAKE_*_LINKER_FLAGS.
# Disabled where while they are set to ue_linker_flags.
#set(c_libraries "-lm -lc -lgcc_s -lgcc -lpthread")
#set(CMAKE_C_STANDARD_LIBRARIES  "${c_libraries}" CACHE INTERNAL "")



###
### OTHER
###

# I don't know why, but CMake fails to detect CMAKE_SIZEOF_VOID_P and it ends up
# being the empty string. Here we assume that we are building with 8-byte
# pointers, i.e., 64-bit mode.
set(CMAKE_SIZEOF_VOID_P 8 CACHE INTERNAL "")

