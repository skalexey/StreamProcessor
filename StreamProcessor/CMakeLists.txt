# CMakeList.txt : CMake project for StreamProcessor, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project(StreamProcessor)

if(MSVC)
	# fix __cplusplus macro for MSVC
	add_compile_options(
		"/Zc:__cplusplus"
	)
else()
	set(CMAKE_CXX_STANDARD 17)
endif()

option(StreamProcessor_INHERIT_PARENT_SRC "Adds all parent's source files" OFF)
option(StreamProcessor_INHERIT_PARENT_LIBS "Adds all parent's libraries" ON)
option(CMAKE_VERBOSE "Additional logs will be shown" OFF)
option(StreamProcessor_CMAKE_VERBOSE "Additional logs will be shown" OFF)

if (CMAKE_VERBOSE)
	set(StreamProcessor_CMAKE_VERBOSE ON)
endif()

if (StreamProcessor_CMAKE_VERBOSE)
	message("StreamProcessor: StreamProcessor_INHERIT_PARENT_SRC is ${StreamProcessor_INHERIT_PARENT_SRC}")
	message("StreamProcessor: StreamProcessor_INHERIT_PARENT_LIBS is ${StreamProcessor_INHERIT_PARENT_LIBS}")
endif()

if (LOG_ON)
	add_compile_definitions("LOG_ON")
	message("StreamProcessor: LOG_ON is defined")
else()
	message("StreamProcessor: LOG_ON is not defined")
endif()
if (LOG_LEVEL_VERBOSE)
	add_compile_definitions("LOG_LEVEL_VERBOSE")
	message("StreamProcessor: LOG_LEVEL_VERBOSE is defined")
else()
	message("StreamProcessor: LOG_LEVEL_VERBOSE is not defined")
endif ()
if (LOG_LEVEL_DEBUG)
	add_compile_definitions("LOG_LEVEL_DEBUG")
	message("StreamProcessor: LOG_LEVEL_DEBUG is defined")
else()
	message("StreamProcessor: LOG_LEVEL_DEBUG is not defined")
endif()

# Inherit parent libs logic
set(PARENT_LIBS ${PARENT_LIBS} ${LIBS})
set(PARENT_LIBS ${PARENT_LIBS} PARENT_SCOPE)
if (StreamProcessor_INHERIT_PARENT_LIBS)
	set(LIBS ${PARENT_LIBS})
	if (StreamProcessor_CMAKE_VERBOSE)
		message("StreamProcessor PARENT_LIBS: ${PARENT_LIBS}")
	endif()
endif()
# End of Inherit parent libs logic

# set build folder name variable for any dependency
if (CMAKE_CXX_COMPILER EQUAL "g++")
	set(BUILD_DIR_NAME "Build-g++")
else()
	set(BUILD_DIR_NAME "Build-cmake")
endif()

# Set and add external includes
if (NOT "${INCLUDES}" STREQUAL "")
	message("StreamProcessor: Add extra includes: ${INCLUDES}")
else()
	set(INCLUDES $ENV{StreamProcessor_includes})
endif()

set (packet_DIR ../packet)
add_subdirectory(${packet_DIR} ${packet_DIR}/${BUILD_DIR_NAME})
set (CUSTOM_INCLUDES ${CUSTOM_INCLUDES} ${packet_INCLUDES})

# Set StreamProcessor_INCLUDES global variable
set(StreamProcessor_INCLUDES ${StreamProcessor_INCLUDES}
	${PROJECT_SOURCE_DIR}/include
	${CUSTOM_INCLUDES}
	${INCLUDES}
)
set(StreamProcessor_INCLUDES ${StreamProcessor_INCLUDES} PARENT_SCOPE)

# include all directories
if (NOT "${StreamProcessor_INCLUDES}" STREQUAL "")
	message("StreamProcessor_INCLUDES generated: ${StreamProcessor_INCLUDES}")
	include_directories("${StreamProcessor_INCLUDES}")
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
file(GLOB SRC "src/*.cpp")
file(GLOB HEADERS "include/*.h")
set(SRC ${SRC} ${INCLUDES_SRC})

# Inherit parent sources logic
set(PARENT_SRC ${PARENT_SRC} ${SRC})
set(PARENT_SRC ${PARENT_SRC} PARENT_SCOPE)
if (StreamProcessor_INHERIT_PARENT_SRC)
	set(SRC ${PARENT_SRC})
	if (StreamProcessor_CMAKE_VERBOSE)
		message("StreamProcessor PARENT_SRC: ${PARENT_SRC}")
	endif()
endif()
# End of Inherit parent sources logic

# add library
add_library (StreamProcessor ${SRC} ${HEADERS})
target_link_libraries(StreamProcessor packet)

# link libraries
# Link all librariles including inherited
set(cnt 1)
foreach(item ${LIBS})
	if (${cnt} EQUAL 1)
		if (StreamProcessor_CMAKE_VERBOSE)	
			message("Link library '${item}'")
		endif()
		set(lib_name ${item})
	elseif(${cnt} EQUAL 2)
		if (StreamProcessor_CMAKE_VERBOSE)
			message("Library '${lib_name}' path: ${item}")
		endif()
		set(lib_path ${item})
	elseif(${cnt} EQUAL 3)
		if (StreamProcessor_CMAKE_VERBOSE)
			message("Library '${lib_name}' binary path: ${item}")
		endif()
		#add_subdirectory("${lib_path}" "${item}")
		target_link_libraries(StreamProcessor ${lib_name})
	endif()
	MATH(EXPR cnt "${cnt}+1")
endforeach()
