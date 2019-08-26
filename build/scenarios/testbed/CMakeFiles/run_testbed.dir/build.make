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
CMAKE_SOURCE_DIR = /home/vad/artery

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/vad/artery/build

# Utility rule file for run_testbed.

# Include the progress variables for this target.
include scenarios/testbed/CMakeFiles/run_testbed.dir/progress.make

scenarios/testbed/CMakeFiles/run_testbed:
	cd /home/vad/artery/scenarios/testbed && /home/vad/omnetpp-5.5.1/bin/opp_run_release -n /home/vad/artery/src/artery:/home/vad/artery/src/traci:/home/vad/artery/extern/veins/examples/veins:/home/vad/artery/extern/veins/src/veins:/home/vad/artery/extern/inet/src:/home/vad/artery/extern/inet/examples:/home/vad/artery/extern/inet/tutorials:/home/vad/artery/extern/inet/showcases -l /home/vad/artery/build/src/artery/envmod/libartery_envmod.so -l /home/vad/artery/build/scenarios/highway-police/libartery_police.so -l /home/vad/artery/extern/inet/out/gcc-release/src/libINET.so -l /home/vad/artery/build/src/artery/testbed/libartery_testbed.so -l /home/vad/artery/build/src/artery/envmod/libartery_envmod.so -l /home/vad/artery/build/src/artery/storyboard/libartery_storyboard.so -l /home/vad/artery/extern/inet/out/gcc-release/src/libINET.so -l /home/vad/artery/extern/veins/out/gcc-release/src/libveins.so -l /home/vad/artery/build/src/artery/libartery_core.so omnetpp.ini

run_testbed: scenarios/testbed/CMakeFiles/run_testbed
run_testbed: scenarios/testbed/CMakeFiles/run_testbed.dir/build.make

.PHONY : run_testbed

# Rule to build all files generated by this target.
scenarios/testbed/CMakeFiles/run_testbed.dir/build: run_testbed

.PHONY : scenarios/testbed/CMakeFiles/run_testbed.dir/build

scenarios/testbed/CMakeFiles/run_testbed.dir/clean:
	cd /home/vad/artery/build/scenarios/testbed && $(CMAKE_COMMAND) -P CMakeFiles/run_testbed.dir/cmake_clean.cmake
.PHONY : scenarios/testbed/CMakeFiles/run_testbed.dir/clean

scenarios/testbed/CMakeFiles/run_testbed.dir/depend:
	cd /home/vad/artery/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/vad/artery /home/vad/artery/scenarios/testbed /home/vad/artery/build /home/vad/artery/build/scenarios/testbed /home/vad/artery/build/scenarios/testbed/CMakeFiles/run_testbed.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : scenarios/testbed/CMakeFiles/run_testbed.dir/depend

