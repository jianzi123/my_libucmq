# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cheng/company/supex/open/lib/polarssl-1.2.8

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cheng/company/supex/open/lib/build

# Utility rule file for apidoc.

# Include the progress variables for this target.
include CMakeFiles/apidoc.dir/progress.make

CMakeFiles/apidoc:
	cd /home/cheng/company/supex/open/lib/polarssl-1.2.8 && doxygen doxygen/polarssl.doxyfile

apidoc: CMakeFiles/apidoc
apidoc: CMakeFiles/apidoc.dir/build.make
.PHONY : apidoc

# Rule to build all files generated by this target.
CMakeFiles/apidoc.dir/build: apidoc
.PHONY : CMakeFiles/apidoc.dir/build

CMakeFiles/apidoc.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/apidoc.dir/cmake_clean.cmake
.PHONY : CMakeFiles/apidoc.dir/clean

CMakeFiles/apidoc.dir/depend:
	cd /home/cheng/company/supex/open/lib/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cheng/company/supex/open/lib/polarssl-1.2.8 /home/cheng/company/supex/open/lib/polarssl-1.2.8 /home/cheng/company/supex/open/lib/build /home/cheng/company/supex/open/lib/build /home/cheng/company/supex/open/lib/build/CMakeFiles/apidoc.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/apidoc.dir/depend

