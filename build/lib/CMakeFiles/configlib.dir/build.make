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
include lib/CMakeFiles/configlib.dir/depend.make

# Include the progress variables for this target.
include lib/CMakeFiles/configlib.dir/progress.make

# Include the compile flags for this target's objects.
include lib/CMakeFiles/configlib.dir/flags.make

lib/CMakeFiles/configlib.dir/__/src/config_fun.c.o: lib/CMakeFiles/configlib.dir/flags.make
lib/CMakeFiles/configlib.dir/__/src/config_fun.c.o: ../src/config_fun.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zlgmcu/project/config_learn/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object lib/CMakeFiles/configlib.dir/__/src/config_fun.c.o"
	cd /home/zlgmcu/project/config_learn/build/lib && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/configlib.dir/__/src/config_fun.c.o   -c /home/zlgmcu/project/config_learn/src/config_fun.c

lib/CMakeFiles/configlib.dir/__/src/config_fun.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/configlib.dir/__/src/config_fun.c.i"
	cd /home/zlgmcu/project/config_learn/build/lib && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/zlgmcu/project/config_learn/src/config_fun.c > CMakeFiles/configlib.dir/__/src/config_fun.c.i

lib/CMakeFiles/configlib.dir/__/src/config_fun.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/configlib.dir/__/src/config_fun.c.s"
	cd /home/zlgmcu/project/config_learn/build/lib && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/zlgmcu/project/config_learn/src/config_fun.c -o CMakeFiles/configlib.dir/__/src/config_fun.c.s

# Object files for target configlib
configlib_OBJECTS = \
"CMakeFiles/configlib.dir/__/src/config_fun.c.o"

# External object files for target configlib
configlib_EXTERNAL_OBJECTS =

../lib/libconfiglib.so: lib/CMakeFiles/configlib.dir/__/src/config_fun.c.o
../lib/libconfiglib.so: lib/CMakeFiles/configlib.dir/build.make
../lib/libconfiglib.so: lib/CMakeFiles/configlib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/zlgmcu/project/config_learn/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C shared library ../../lib/libconfiglib.so"
	cd /home/zlgmcu/project/config_learn/build/lib && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/configlib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lib/CMakeFiles/configlib.dir/build: ../lib/libconfiglib.so

.PHONY : lib/CMakeFiles/configlib.dir/build

lib/CMakeFiles/configlib.dir/clean:
	cd /home/zlgmcu/project/config_learn/build/lib && $(CMAKE_COMMAND) -P CMakeFiles/configlib.dir/cmake_clean.cmake
.PHONY : lib/CMakeFiles/configlib.dir/clean

lib/CMakeFiles/configlib.dir/depend:
	cd /home/zlgmcu/project/config_learn/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zlgmcu/project/config_learn /home/zlgmcu/project/config_learn/lib /home/zlgmcu/project/config_learn/build /home/zlgmcu/project/config_learn/build/lib /home/zlgmcu/project/config_learn/build/lib/CMakeFiles/configlib.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lib/CMakeFiles/configlib.dir/depend

