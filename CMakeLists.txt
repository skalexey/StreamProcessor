# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project (StreamProcessorSolution C CXX)

if(MSVC)
	# fix __cplusplus macro for MSVC
	add_compile_options(
		"/Zc:__cplusplus"
		"/std:c++latest"
	)
else()
	set(CMAKE_CXX_STANDARD 20)
endif()

option(INHERIT_PARENT_SRC "Adds all parent's source files" OFF)
option(INHERIT_PARENT_LIBS "Adds all parent's libraries" OFF)

message("StreamProcessorSolution: INHERIT_PARENT_SRC is ${INHERIT_PARENT_SRC}")
message("StreamProcessorSolution: INHERIT_PARENT_LIBS is ${INHERIT_PARENT_LIBS}")

if (LOG_ON)
	add_compile_definitions("LOG_ON")
	message("StreamProcessorSolution: LOG_ON is defined")
else()
	message("StreamProcessorSolution: LOG_ON is not defined")
endif ()
if (LOG_LEVEL_VERBOSE)
	add_compile_definitions("LOG_LEVEL_VERBOSE")
	message("StreamProcessorSolution: LOG_LEVEL_VERBOSE is defined")
else()
	message("StreamProcessorSolution: LOG_LEVEL_VERBOSE is not defined")
endif ()
if (LOG_LEVEL_DEBUG)
	add_compile_definitions("LOG_LEVEL_DEBUG")
	message("StreamProcessorSolution: LOG_LEVEL_DEBUG is defined")
else()
	message("StreamProcessorSolution: LOG_LEVEL_DEBUG is not defined")
endif()

set(StreamProcessorSolution_INCLUDES "${PROJECT_SOURCE_DIR}/include")
if (NOT "${CUSTOM_INCLUDES}" STREQUAL "")
	set(StreamProcessorSolution_INCLUDES ${StreamProcessorSolution_INCLUDES} ${CUSTOM_INCLUDES})
endif()

# Build directory name
if (CMAKE_CXX_COMPILER EQUAL "g++")
	set(BUILD_DIR_NAME "Build-g++")
else ()
	set(BUILD_DIR_NAME "Build-cmake")
endif ()

# Add the lib to PARENT_LIBS for including in subprojects (example)
#set(PARENT_LIBS ${PARENT_LIBS} utils "${UTILS_DIR}" "${UTILS_DIR}/${BUILD_DIR_NAME}")
#set(PARENT_LIBS ${PARENT_LIBS} PARENT_SCOPE)

# Set and add external includes
if (NOT "${INCLUDES}" STREQUAL "")
	message("StreamProcessorSolution: Add extra includes: ${INCLUDES}")
else()
	set(INCLUDES $ENV{StreamProcessorSolution_includes})
endif()

# Set StreamProcessorSolution_INCLUDES global variable
set(StreamProcessorSolution_INCLUDES ${StreamProcessorSolution_INCLUDES}
	${CUSTOM_INCLUDES}
	${INCLUDES}
)

set(StreamProcessorSolution_INCLUDES ${StreamProcessorSolution_INCLUDES} PARENT_SCOPE)
message("StreamProcessorSolution: StreamProcessorSolution_INCLUDES: ${StreamProcessorSolution_INCLUDES}")

include_directories(".")
include_directories("${StreamProcessorSolution_INCLUDES}")
# end of includes set up

# Note: for no-executable and no-lib project SRC still can be non-empty
# Fetch CPP from external includes
if (NOT "${INCLUDES}" STREQUAL "")
	message("StreamProcessorSolution: External INCLUDES variable: ${INCLUDES}")
	foreach(items ${INCLUDES})
		file(GLOB_RECURSE INCLUDES_SRC "${items}/*.cpp")
	endforeach()
	message("StreamProcessorSolution: External INCLUDES sources: ${INCLUDES_SRC}")
endif()

# set up sources
file(GLOB SRC "*.cpp")
file(GLOB HEADERS "*.h")
set(SRC ${SRC} ${INCLUDES_SRC})

# Inherit parent sources logic
set(PARENT_SRC ${PARENT_SRC} ${SRC})
set(PARENT_SRC ${PARENT_SRC} PARENT_SCOPE)

if (INHERIT_PARENT_SRC)
	set(SRC ${PARENT_SRC})
	message("'StreamProcessorSolution' project PARENT_SRC: ${PARENT_SRC}")
endif()
# End of Inherit parent sources logic

# Inherit parent libs logic
set(PARENT_LIBS ${PARENT_LIBS} ${LIBS})
set(PARENT_LIBS ${PARENT_LIBS} PARENT_SCOPE)

if (StreamProcessorSolution_INHERIT_PARENT_LIBS)
	set(LIBS ${PARENT_LIBS})
	message("'StreamProcessorSolution' StreamProcessorSolution PARENT_LIBS: ${PARENT_LIBS}")
endif()
# End of Inherit parent libs logic

add_subdirectory ("StreamProcessor")
add_subdirectory ("Test")
