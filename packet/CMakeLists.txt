# CMakeList.txt : CMake project for packet, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project(packet)

if(MSVC)
	# fix __cplusplus macro for MSVC
	add_compile_options(
		"/Zc:__cplusplus"
		"/std:c++latest"
	)
else()
	set(CMAKE_CXX_STANDARD 20)
endif()

option(packet_INHERIT_PARENT_SRC "Adds all parent's source files" OFF)
option(packet_INHERIT_PARENT_LIBS "Adds all parent's libraries" ON)
option(CMAKE_VERBOSE "Additional logs will be shown" OFF)
option(packet_CMAKE_VERBOSE "Additional logs will be shown" OFF)

if (CMAKE_VERBOSE)
	set(packet_CMAKE_VERBOSE ON)
endif()

if (packet_CMAKE_VERBOSE)
	message("packet: packet_INHERIT_PARENT_SRC is ${packet_INHERIT_PARENT_SRC}")
	message("packet: packet_INHERIT_PARENT_LIBS is ${packet_INHERIT_PARENT_LIBS}")
endif()

if (LOG_ON)
	add_compile_definitions("LOG_ON")
	message("packet: LOG_ON is defined")
else()
	message("packet: LOG_ON is not defined")
endif()
if (LOG_LEVEL_VERBOSE)
	add_compile_definitions("LOG_LEVEL_VERBOSE")
	message("packet: LOG_LEVEL_VERBOSE is defined")
else()
	message("packet: LOG_LEVEL_VERBOSE is not defined")
endif ()
if (LOG_LEVEL_DEBUG)
	add_compile_definitions("LOG_LEVEL_DEBUG")
	message("packet: LOG_LEVEL_DEBUG is defined")
else()
	message("packet: LOG_LEVEL_DEBUG is not defined")
endif()

# Inherit parent libs logic
set(PARENT_LIBS ${PARENT_LIBS} ${LIBS})
set(PARENT_LIBS ${PARENT_LIBS} PARENT_SCOPE)
if (packet_INHERIT_PARENT_LIBS)
	set(LIBS ${PARENT_LIBS})
	if (packet_CMAKE_VERBOSE)
		message("packet PARENT_LIBS: ${PARENT_LIBS}")
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
	message("packet: Add extra includes: ${INCLUDES}")
else()
	set(INCLUDES $ENV{packet_includes})
endif()

# Set packet_INCLUDES global variable
set(packet_INCLUDES ${packet_INCLUDES}
	${PROJECT_SOURCE_DIR}/include
	${CUSTOM_INCLUDES}
	${INCLUDES}
)
set(packet_INCLUDES ${packet_INCLUDES} PARENT_SCOPE)

# include all directories
if (NOT "${packet_INCLUDES}" STREQUAL "")
	message("packet_INCLUDES generated: ${packet_INCLUDES}")
	include_directories("${packet_INCLUDES}")
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
if (packet_INHERIT_PARENT_SRC)
	set(SRC ${PARENT_SRC})
	if (packet_CMAKE_VERBOSE)
		message("packet PARENT_SRC: ${PARENT_SRC}")
	endif()
endif()
# End of Inherit parent sources logic

# add library
add_library (packet ${SRC} ${HEADERS})

# link libraries
# Link all librariles including inherited
set(cnt 1)
foreach(item ${LIBS})
	if (${cnt} EQUAL 1)
		if (packet_CMAKE_VERBOSE)	
			message("Link library '${item}'")
		endif()
		set(lib_name ${item})
	elseif(${cnt} EQUAL 2)
		if (packet_CMAKE_VERBOSE)
			message("Library '${lib_name}' path: ${item}")
		endif()
		set(lib_path ${item})
	elseif(${cnt} EQUAL 3)
		if (packet_CMAKE_VERBOSE)
			message("Library '${lib_name}' binary path: ${item}")
		endif()
		#add_subdirectory("${lib_path}" "${item}")
		target_link_libraries(packet ${lib_name})
	endif()
	MATH(EXPR cnt "${cnt}+1")
endforeach()
