# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.31

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\build_mingw"

# Include any dependencies generated for this target.
include tests/CMakeFiles/parse_number.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tests/CMakeFiles/parse_number.dir/compiler_depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/parse_number.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/parse_number.dir/flags.make

tests/CMakeFiles/parse_number.dir/codegen:
.PHONY : tests/CMakeFiles/parse_number.dir/codegen

tests/CMakeFiles/parse_number.dir/parse_number.c.obj: tests/CMakeFiles/parse_number.dir/flags.make
tests/CMakeFiles/parse_number.dir/parse_number.c.obj: D:/Documentos/Septimo\ semestre/Sistos/CHAT-SISTO/cJSON/tests/parse_number.c
tests/CMakeFiles/parse_number.dir/parse_number.c.obj: tests/CMakeFiles/parse_number.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir="D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\build_mingw\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object tests/CMakeFiles/parse_number.dir/parse_number.c.obj"
	cd /d "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\build_mingw\tests" && C:\MinGW64\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT tests/CMakeFiles/parse_number.dir/parse_number.c.obj -MF CMakeFiles\parse_number.dir\parse_number.c.obj.d -o CMakeFiles\parse_number.dir\parse_number.c.obj -c "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\tests\parse_number.c"

tests/CMakeFiles/parse_number.dir/parse_number.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/parse_number.dir/parse_number.c.i"
	cd /d "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\build_mingw\tests" && C:\MinGW64\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\tests\parse_number.c" > CMakeFiles\parse_number.dir\parse_number.c.i

tests/CMakeFiles/parse_number.dir/parse_number.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/parse_number.dir/parse_number.c.s"
	cd /d "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\build_mingw\tests" && C:\MinGW64\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\tests\parse_number.c" -o CMakeFiles\parse_number.dir\parse_number.c.s

# Object files for target parse_number
parse_number_OBJECTS = \
"CMakeFiles/parse_number.dir/parse_number.c.obj"

# External object files for target parse_number
parse_number_EXTERNAL_OBJECTS =

tests/parse_number.exe: tests/CMakeFiles/parse_number.dir/parse_number.c.obj
tests/parse_number.exe: tests/CMakeFiles/parse_number.dir/build.make
tests/parse_number.exe: libcjson.dll.a
tests/parse_number.exe: tests/libunity.a
tests/parse_number.exe: tests/CMakeFiles/parse_number.dir/linkLibs.rsp
tests/parse_number.exe: tests/CMakeFiles/parse_number.dir/objects1.rsp
tests/parse_number.exe: tests/CMakeFiles/parse_number.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir="D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\build_mingw\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable parse_number.exe"
	cd /d "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\build_mingw\tests" && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\parse_number.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/parse_number.dir/build: tests/parse_number.exe
.PHONY : tests/CMakeFiles/parse_number.dir/build

tests/CMakeFiles/parse_number.dir/clean:
	cd /d "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\build_mingw\tests" && $(CMAKE_COMMAND) -P CMakeFiles\parse_number.dir\cmake_clean.cmake
.PHONY : tests/CMakeFiles/parse_number.dir/clean

tests/CMakeFiles/parse_number.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON" "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\tests" "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\build_mingw" "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\build_mingw\tests" "D:\Documentos\Septimo semestre\Sistos\CHAT-SISTO\cJSON\build_mingw\tests\CMakeFiles\parse_number.dir\DependInfo.cmake" "--color=$(COLOR)"
.PHONY : tests/CMakeFiles/parse_number.dir/depend

