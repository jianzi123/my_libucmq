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

# Include any dependencies generated for this target.
include programs/pkey/CMakeFiles/rsa_verify_pss.dir/depend.make

# Include the progress variables for this target.
include programs/pkey/CMakeFiles/rsa_verify_pss.dir/progress.make

# Include the compile flags for this target's objects.
include programs/pkey/CMakeFiles/rsa_verify_pss.dir/flags.make

programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o: programs/pkey/CMakeFiles/rsa_verify_pss.dir/flags.make
programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o: /home/cheng/company/supex/open/lib/polarssl-1.2.8/programs/pkey/rsa_verify_pss.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/cheng/company/supex/open/lib/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o"
	cd /home/cheng/company/supex/open/lib/build/programs/pkey && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o   -c /home/cheng/company/supex/open/lib/polarssl-1.2.8/programs/pkey/rsa_verify_pss.c

programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.i"
	cd /home/cheng/company/supex/open/lib/build/programs/pkey && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/cheng/company/supex/open/lib/polarssl-1.2.8/programs/pkey/rsa_verify_pss.c > CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.i

programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.s"
	cd /home/cheng/company/supex/open/lib/build/programs/pkey && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/cheng/company/supex/open/lib/polarssl-1.2.8/programs/pkey/rsa_verify_pss.c -o CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.s

programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o.requires:
.PHONY : programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o.requires

programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o.provides: programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o.requires
	$(MAKE) -f programs/pkey/CMakeFiles/rsa_verify_pss.dir/build.make programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o.provides.build
.PHONY : programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o.provides

programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o.provides.build: programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o

# Object files for target rsa_verify_pss
rsa_verify_pss_OBJECTS = \
"CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o"

# External object files for target rsa_verify_pss
rsa_verify_pss_EXTERNAL_OBJECTS =

programs/pkey/rsa_verify_pss: programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o
programs/pkey/rsa_verify_pss: programs/pkey/CMakeFiles/rsa_verify_pss.dir/build.make
programs/pkey/rsa_verify_pss: library/libpolarssl.so
programs/pkey/rsa_verify_pss: programs/pkey/CMakeFiles/rsa_verify_pss.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable rsa_verify_pss"
	cd /home/cheng/company/supex/open/lib/build/programs/pkey && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rsa_verify_pss.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
programs/pkey/CMakeFiles/rsa_verify_pss.dir/build: programs/pkey/rsa_verify_pss
.PHONY : programs/pkey/CMakeFiles/rsa_verify_pss.dir/build

programs/pkey/CMakeFiles/rsa_verify_pss.dir/requires: programs/pkey/CMakeFiles/rsa_verify_pss.dir/rsa_verify_pss.c.o.requires
.PHONY : programs/pkey/CMakeFiles/rsa_verify_pss.dir/requires

programs/pkey/CMakeFiles/rsa_verify_pss.dir/clean:
	cd /home/cheng/company/supex/open/lib/build/programs/pkey && $(CMAKE_COMMAND) -P CMakeFiles/rsa_verify_pss.dir/cmake_clean.cmake
.PHONY : programs/pkey/CMakeFiles/rsa_verify_pss.dir/clean

programs/pkey/CMakeFiles/rsa_verify_pss.dir/depend:
	cd /home/cheng/company/supex/open/lib/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cheng/company/supex/open/lib/polarssl-1.2.8 /home/cheng/company/supex/open/lib/polarssl-1.2.8/programs/pkey /home/cheng/company/supex/open/lib/build /home/cheng/company/supex/open/lib/build/programs/pkey /home/cheng/company/supex/open/lib/build/programs/pkey/CMakeFiles/rsa_verify_pss.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : programs/pkey/CMakeFiles/rsa_verify_pss.dir/depend

