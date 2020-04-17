#!/usr/bin/env python3


"""Unreal compatible package generator for AGX Dynamics.

This program creates an AGX Dynamics for Unreal package from a source build of
AGX Dynamics. The build is assumed to have been made according to the AGX
Dynamics for Unreal build instructions available in the AGXUnreal Development
Handbook.

The process consists primarily of file and directory copies, but the
setup_env.bash script is patched to make it file system location independent. A
number of file- and directory name filters are applied to limit the files
topied to those needed by AGX Dynamics for Unreal. This list may still be
incomplete.

"""

from os import environ, listdir, makedirs, readlink, symlink
from os.path import join, isdir, isfile, islink
from shutil import copy2

agx_source_dir = environ["AGX_DIR"]
agx_components_dir = environ["AGX_COMPONENT_DEV_PATH"]
agx_bin_dir = environ["AGX_BINARY_DIR"]
agx_build_dir = environ["AGX_BUILD_DIR"]
agx_deps_dir = environ["AGX_DEPENDENCIES_DIR"]
agxterrain_deps_dir = environ["AGXTERRAIN_DEPENDENCIES_DIR"]


def get_package_dir():
    return "agx_dynamics_for_unreal"


def get_package_subdir(subdir):
    return join(get_package_dir(), subdir)


def get_package_cfg_dir():
    return get_package_subdir(join("data", "cfg"))


def get_package_lib_dir():
    return get_package_subdir("lib")


def get_package_include_dir():
    return get_package_subdir("include")


def get_package_components_dir():
    return get_package_subdir("Components")


def get_package_dependencies_dir():
    return get_package_subdir("dependencies")


def create_package_folders():
    makedirs(get_package_dir())
    makedirs(get_package_cfg_dir())
    makedirs(get_package_lib_dir())
    makedirs(get_package_include_dir())
    makedirs(get_package_components_dir())
    makedirs(get_package_dependencies_dir())


# Would like to use shutils.copytree instead, but dirs_exist_ok isn't supported
# until Python 3.8 and Ubuntu 18.04 doesn't have that by default.
def copy_directory_contents(source, destination, skip_list=[]):
    if not isdir(source):
        raise NotADirectoryError()
    if not isdir(destination):
        raise NotADirectoryError()

    def should_skip(content):
        for pattern in skip_list:
            if pattern.match(content):
                return True
        return False
    contents = listdir(source)
    for content in contents:
        if should_skip(content):
            continue
        source_content = join(source, content)
        destination_content = join(destination, content)
        if islink(source_content):
            link_target = readlink(source_content)
            symlink(link_target, destination_content)
        elif isdir(source_content):
            makedirs(destination_content, exist_ok=True)
            copy_directory_contents(source_content, destination_content,
                                    skip_list)
        elif isfile(source_content):
            copy2(source_content, destination_content)


def copy_setup_env():
    import io
    with io.open(join(get_package_dir(), "setup_env.bash"), "w") \
            as new_setup_env:
        root = '$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)'
        new_setup_env.write('package_root="{}"\n'.format(root))
        replacers = [
            (agx_source_dir, "${package_root}"),
            # TODO: Don't hard-code dependency directory names.
            (agx_deps_dir, join("${package_root}", "dependencies",
                                "agx_dependencies_191204_ubuntu_18.04_64")),
            (agxterrain_deps_dir,
             join("${package_root}", "dependencies",
                  "agxTerrain_dependencies_20191204_ubuntu_18.04_64_unreal")),
            (agx_build_dir, "${package_root}")
        ]
        with io.open(join(agx_build_dir, "setup_env.bash"), "r") \
                as old_setup_env:
            for line in old_setup_env:
                for replacer in replacers:
                    line = line.replace(replacer[0], replacer[1])
                new_setup_env.write(line)


def copy_cfg():
    copy2(join(agx_source_dir, "data", "cfg", "default.cfg"),
          get_package_cfg_dir())


def copy_lib():
    copy_directory_contents(join(agx_build_dir, "lib"), get_package_lib_dir())


def copy_source_include():
    copy_directory_contents(join(agx_source_dir, "include"),
                            get_package_include_dir())


def copy_build_include():
    copy_directory_contents(join(agx_build_dir, "include"),
                            get_package_include_dir())


def copy_components():
    copy_directory_contents(join(agx_source_dir, "Components"),
                            get_package_components_dir())


def copy_dependencies():
    import re
    skip_list = [
        re.compile("^libosg.*"),  # OSG libraries.
        re.compile("^osg"),  # OSG headers.
        re.compile("^osg[A-Z]+")  # More OSG headers.
        # Why they don't put a suffix on their include files I will never know.
    ]
    copy_directory_contents(join(agx_build_dir, "dependencies"),
                            get_package_dependencies_dir(), skip_list)


def create_archive():
    # make_archive doesn't support symlinks in Python 3.6. Fallback to zip
    # executable.
    import subprocess
    subprocess.run(["zip", "--symlinks", "-q", "-r", get_package_dir()+".zip",
                    get_package_dir()])

    # mark_archive based implementation. Use this once we get symlink support.
    # import os
    # make_archive(get_package_dir(), "zip", get_package_dir())


create_package_folders()
copy_setup_env()
copy_cfg()
copy_lib()
copy_source_include()
copy_build_include()
copy_components()
copy_dependencies()
create_archive()
