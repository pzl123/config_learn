# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/zlgmcu/project/config_learn

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zlgmcu/project/config_learn/build

# Include any dependencies generated for this target.
include third_party/CMakeFiles/cJSONxlib.dir/depend.make

# Include the progress variables for this target.
include third_party/CMakeFiles/cJSONxlib.dir/progress.make

# Include the compile flags for this target's objects.
include third_party/CMakeFiles/cJSONxlib.dir/flags.make

third_party/CMakeFiles/cJSONxlib.dir/cJSONx/src/cJSONx.c.o: third_party/CMakeFiles/cJSONxlib.dir/flags.make
third_party/CMakeFiles/cJSONxlib.dir/cJSONx/src/cJSONx.c.o: ../third_party/cJSONx/src/cJSONx.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zlgmcu/project/config_learn/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object third_party/CMakeFiles/cJSONxlib.dir/cJSONx/src/cJSONx.c.o"
	cd /home/zlgmcu/project/config_learn/build/third_party && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/cJSONxlib.dir/cJSONx/src/cJSONx.c.o   -c /home/zlgmcu/project/config_learn/third_party/cJSONx/src/cJSONx.c

third_party/CMakeFiles/cJSONxlib.dir/cJSONx/src/cJSONx.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cJSONxlib.dir/cJSONx/src/cJSONx.c.i"
	cd /home/zlgmcu/project/config_learn/build/third_party && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/zlgmcu/project/config_learn/third_party/cJSONx/src/cJSONx.c > CMakeFiles/cJSONxlib.dir/cJSONx/src/cJSONx.c.i

third_party/CMakeFiles/cJSONxlib.dir/cJSONx/src/cJSONx.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cJSONxlib.dir/cJSONx/src/cJSONx.c.s"
	cd /home/zlgmcu/project/config_learn/build/third_party && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/zlgmcu/project/config_learn/third_party/cJSONx/src/cJSONx.c -o CMakeFiles/cJSONxlib.dir/cJSONx/src/cJSONx.c.s

# Object files for target cJSONxlib
cJSONxlib_OBJECTS = \
"CMakeFiles/cJSONxlib.dir/cJSONx/src/cJSONx.c.o"

# External object files for target cJSONxlib
cJSONxlib_EXTERNAL_OBJECTS =

../lib/libcJSONxlib.so: third_party/CMakeFiles/cJSONxlib.dir/cJSONx/src/cJSONx.c.o
../lib/libcJSONxlib.so: third_party/CMakeFiles/cJSONxlib.dir/build.make
../lib/libcJSONxlib.so: third_party/CMakeFiles/cJSONxlib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/zlgmcu/project/config_learn/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C shared library ../../lib/libcJSONxlib.so"
	cd /home/zlgmcu/project/config_learn/build/third_party && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cJSONxlib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
third_party/CMakeFiles/cJSONxlib.dir/build: ../lib/libcJSONxlib.so

.PHONY : third_party/CMakeFiles/cJSONxlib.dir/build

third_party/CMakeFiles/cJSONxlib.dir/clean:
	cd /home/zlgmcu/project/config_learn/build/third_party && $(CMAKE_COMMAND) -P CMakeFiles/cJSONxlib.dir/cmake_clean.cmake
.PHONY : third_party/CMakeFiles/cJSONxlib.dir/clean

third_party/CMakeFiles/cJSONxlib.dir/depend:
	cd /home/zlgmcu/project/config_learn/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zlgmcu/project/config_learn /home/zlgmcu/project/config_learn/third_party /home/zlgmcu/project/config_learn/build /home/zlgmcu/project/config_learn/build/third_party /home/zlgmcu/project/config_learn/build/third_party/CMakeFiles/cJSONxlib.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : third_party/CMakeFiles/cJSONxlib.dir/depend

