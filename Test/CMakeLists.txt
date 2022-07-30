# CMakeList.txt : CMake project for Test, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project(Test)

if(MSVC)
	# fix __cplusplus macro for MSVC
	add_compile_options(
		"/Zc:__cplusplus"
		"/std:c++latest"
	)
else()
	set(CMAKE_CXX_STANDARD 20)
endif()

option(Test_INHERIT_PARENT_SRC "Adds all parent's source files" OFF)
option(Test_INHERIT_PARENT_LIBS "Adds all parent's libraries" ON)
option(CMAKE_VERBOSE "Additional logs will be shown" OFF)
option(Test_CMAKE_VERBOSE "Additional logs will be shown" OFF)

if (CMAKE_VERBOSE)
	set(Test_CMAKE_VERBOSE ON)
endif()

if (Test_CMAKE_VERBOSE)
	message("Test: Test_INHERIT_PARENT_SRC is ${Test_INHERIT_PARENT_SRC}")
	message("Test: Test_INHERIT_PARENT_LIBS is ${Test_INHERIT_PARENT_LIBS}")
endif()

if (LOG_ON)
	add_compile_definitions("LOG_ON")
	message("Test: LOG_ON is defined")
else()
	message("Test: LOG_ON is not defined")
endif()
if (LOG_LEVEL_VERBOSE)
	add_compile_definitions("LOG_LEVEL_VERBOSE")
	message("Test: LOG_LEVEL_VERBOSE is defined")
else()
	message("Test: LOG_LEVEL_VERBOSE is not defined")
endif ()
if (LOG_LEVEL_DEBUG)
	add_compile_definitions("LOG_LEVEL_DEBUG")
	message("Test: LOG_LEVEL_DEBUG is defined")
else()
	message("Test: LOG_LEVEL_DEBUG is not defined")
endif()

# Inherit parent libs logic
set(PARENT_LIBS ${PARENT_LIBS} ${LIBS})
set(PARENT_LIBS ${PARENT_LIBS} PARENT_SCOPE)
if (Test_INHERIT_PARENT_LIBS)
	set(LIBS ${PARENT_LIBS})
	if (Test_CMAKE_VERBOSE)
		message("Test PARENT_LIBS: ${PARENT_LIBS}")
	endif()
endif()
# End of Inherit parent libs logic

# set build folder name variable for any dependency
if (CMAKE_CXX_COMPILER EQUAL "g++")
	set(BUILD_DIR_NAME "Build-g++")
else()
	set(BUILD_DIR_NAME "Build-cmake")
endif()

set (STREAM_PROCESSOR_DIR ../StreamProcessor)
#add_subdirectory(${STREAM_PROCESSOR_DIR} ${STREAM_PROCESSOR_DIR}/${BUILD_DIR_NAME})
set (CUSTOM_INCLUDES ${CUSTOM_INCLUDES} ${StreamProcessor_INCLUDES})

# Set and add external includes
if (NOT "${INCLUDES}" STREQUAL "")
	message("Test: Add extra includes: ${INCLUDES}")
else()
	set(INCLUDES $ENV{Test_includes})
endif()

# Set Test_INCLUDES global variable
set(Test_INCLUDES ${Test_INCLUDES}
	${CUSTOM_INCLUDES}
	${INCLUDES}
)
set(Test_INCLUDES ${Test_INCLUDES} PARENT_SCOPE)

# include all directories
if (NOT "${Test_INCLUDES}" STREQUAL "")
	message("Test_INCLUDES generated: ${Test_INCLUDES}")
	include_directories("${Test_INCLUDES}")
endif()

# Fetch CPP from external includes
if (NOT "${INCLUDES}" STREQUAL "")
	message("Project: External INCLUDES variable: ${INCLUDES}")
	foreach(items ${INCLUDES})
		file(GLOB_RECURSE INCLUDES_SRC "${items}/*.cpp")
	endforeach()
	message("Project: External INCLUDES sources: ${INCLUDES_SRC}")
endif()

# set up sources
file(GLOB SRC "*.cpp")
file(GLOB HEADERS "*.h")
set(SRC ${SRC} ${INCLUDES_SRC})

# Inherit parent sources logic
set(PARENT_SRC ${PARENT_SRC} ${SRC})
set(PARENT_SRC ${PARENT_SRC} PARENT_SCOPE)
if (Test_INHERIT_PARENT_SRC)
	set(SRC ${PARENT_SRC})
	if (Test_CMAKE_VERBOSE)
		message("Test PARENT_SRC: ${PARENT_SRC}")
	endif()
endif()
# End of Inherit parent sources logic

# add executable
add_executable (Test ${SRC} ${HEADERS})
target_link_libraries(Test StreamProcessor)

# link libraries
# Link all librariles including inherited
set(cnt 1)
foreach(item ${LIBS})
	if (${cnt} EQUAL 1)
		if (Test_CMAKE_VERBOSE)	
			message("Link library '${item}'")
		endif()
		set(lib_name ${item})
	elseif(${cnt} EQUAL 2)
		if (Test_CMAKE_VERBOSE)
			message("Library '${lib_name}' path: ${item}")
		endif()
		set(lib_path ${item})
	elseif(${cnt} EQUAL 3)
		if (Test_CMAKE_VERBOSE)
			message("Library '${lib_name}' binary path: ${item}")
		endif()
		#add_subdirectory("${lib_path}" "${item}")
		target_link_libraries(Test ${lib_name})
	endif()
	MATH(EXPR cnt "${cnt}+1")
endforeach()

configure_file(test.jpg ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)