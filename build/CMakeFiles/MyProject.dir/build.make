# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /home/vmuser/project/cmake_test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/vmuser/project/cmake_test/build

# Include any dependencies generated for this target.
include CMakeFiles/MyProject.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/MyProject.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/MyProject.dir/flags.make

CMakeFiles/MyProject.dir/src/main.c.o: CMakeFiles/MyProject.dir/flags.make
CMakeFiles/MyProject.dir/src/main.c.o: ../src/main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/vmuser/project/cmake_test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/MyProject.dir/src/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/MyProject.dir/src/main.c.o   -c /home/vmuser/project/cmake_test/src/main.c

CMakeFiles/MyProject.dir/src/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/MyProject.dir/src/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/vmuser/project/cmake_test/src/main.c > CMakeFiles/MyProject.dir/src/main.c.i

CMakeFiles/MyProject.dir/src/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/MyProject.dir/src/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/vmuser/project/cmake_test/src/main.c -o CMakeFiles/MyProject.dir/src/main.c.s

CMakeFiles/MyProject.dir/src/main.c.o.requires:

.PHONY : CMakeFiles/MyProject.dir/src/main.c.o.requires

CMakeFiles/MyProject.dir/src/main.c.o.provides: CMakeFiles/MyProject.dir/src/main.c.o.requires
	$(MAKE) -f CMakeFiles/MyProject.dir/build.make CMakeFiles/MyProject.dir/src/main.c.o.provides.build
.PHONY : CMakeFiles/MyProject.dir/src/main.c.o.provides

CMakeFiles/MyProject.dir/src/main.c.o.provides.build: CMakeFiles/MyProject.dir/src/main.c.o


# Object files for target MyProject
MyProject_OBJECTS = \
"CMakeFiles/MyProject.dir/src/main.c.o"

# External object files for target MyProject
MyProject_EXTERNAL_OBJECTS =

../bin/MyProject: CMakeFiles/MyProject.dir/src/main.c.o
../bin/MyProject: CMakeFiles/MyProject.dir/build.make
../bin/MyProject: ../lib/libconfiglib.a
../bin/MyProject: CMakeFiles/MyProject.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/vmuser/project/cmake_test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ../bin/MyProject"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/MyProject.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/MyProject.dir/build: ../bin/MyProject

.PHONY : CMakeFiles/MyProject.dir/build

CMakeFiles/MyProject.dir/requires: CMakeFiles/MyProject.dir/src/main.c.o.requires

.PHONY : CMakeFiles/MyProject.dir/requires

CMakeFiles/MyProject.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/MyProject.dir/cmake_clean.cmake
.PHONY : CMakeFiles/MyProject.dir/clean

CMakeFiles/MyProject.dir/depend:
	cd /home/vmuser/project/cmake_test/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/vmuser/project/cmake_test /home/vmuser/project/cmake_test /home/vmuser/project/cmake_test/build /home/vmuser/project/cmake_test/build /home/vmuser/project/cmake_test/build/CMakeFiles/MyProject.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/MyProject.dir/depend

