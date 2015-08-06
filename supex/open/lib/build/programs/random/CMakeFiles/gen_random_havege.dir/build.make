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
include programs/random/CMakeFiles/gen_random_havege.dir/depend.make

# Include the progress variables for this target.
include programs/random/CMakeFiles/gen_random_havege.dir/progress.make

# Include the compile flags for this target's objects.
include programs/random/CMakeFiles/gen_random_havege.dir/flags.make

programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o: programs/random/CMakeFiles/gen_random_havege.dir/flags.make
programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o: /home/cheng/company/supex/open/lib/polarssl-1.2.8/programs/random/gen_random_havege.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/cheng/company/supex/open/lib/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o"
	cd /home/cheng/company/supex/open/lib/build/programs/random && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o   -c /home/cheng/company/supex/open/lib/polarssl-1.2.8/programs/random/gen_random_havege.c

programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/gen_random_havege.dir/gen_random_havege.c.i"
	cd /home/cheng/company/supex/open/lib/build/programs/random && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/cheng/company/supex/open/lib/polarssl-1.2.8/programs/random/gen_random_havege.c > CMakeFiles/gen_random_havege.dir/gen_random_havege.c.i

programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/gen_random_havege.dir/gen_random_havege.c.s"
	cd /home/cheng/company/supex/open/lib/build/programs/random && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/cheng/company/supex/open/lib/polarssl-1.2.8/programs/random/gen_random_havege.c -o CMakeFiles/gen_random_havege.dir/gen_random_havege.c.s

programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o.requires:
.PHONY : programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o.requires

programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o.provides: programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o.requires
	$(MAKE) -f programs/random/CMakeFiles/gen_random_havege.dir/build.make programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o.provides.build
.PHONY : programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o.provides

programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o.provides.build: programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o

# Object files for target gen_random_havege
gen_random_havege_OBJECTS = \
"CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o"

# External object files for target gen_random_havege
gen_random_havege_EXTERNAL_OBJECTS =

programs/random/gen_random_havege: programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o
programs/random/gen_random_havege: programs/random/CMakeFiles/gen_random_havege.dir/build.make
programs/random/gen_random_havege: library/libpolarssl.so
programs/random/gen_random_havege: programs/random/CMakeFiles/gen_random_havege.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable gen_random_havege"
	cd /home/cheng/company/supex/open/lib/build/programs/random && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gen_random_havege.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
programs/random/CMakeFiles/gen_random_havege.dir/build: programs/random/gen_random_havege
.PHONY : programs/random/CMakeFiles/gen_random_havege.dir/build

programs/random/CMakeFiles/gen_random_havege.dir/requires: programs/random/CMakeFiles/gen_random_havege.dir/gen_random_havege.c.o.requires
.PHONY : programs/random/CMakeFiles/gen_random_havege.dir/requires

programs/random/CMakeFiles/gen_random_havege.dir/clean:
	cd /home/cheng/company/supex/open/lib/build/programs/random && $(CMAKE_COMMAND) -P CMakeFiles/gen_random_havege.dir/cmake_clean.cmake
.PHONY : programs/random/CMakeFiles/gen_random_havege.dir/clean

programs/random/CMakeFiles/gen_random_havege.dir/depend:
	cd /home/cheng/company/supex/open/lib/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cheng/company/supex/open/lib/polarssl-1.2.8 /home/cheng/company/supex/open/lib/polarssl-1.2.8/programs/random /home/cheng/company/supex/open/lib/build /home/cheng/company/supex/open/lib/build/programs/random /home/cheng/company/supex/open/lib/build/programs/random/CMakeFiles/gen_random_havege.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : programs/random/CMakeFiles/gen_random_havege.dir/depend

