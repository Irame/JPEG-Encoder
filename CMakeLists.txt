project(JPEG-Encoder)
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

macro(setCXXFlag flag)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
endmacro()

# BOOST
set(BOOST_ROOT boost/)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)


include_directories(${Boost_INCLUDE_DIRS})



find_package(Boost COMPONENTS filesystem system thread date_time chrono locale program_options)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_definitions(-DMSVC) # define MSVC
	add_definitions(-D_USE_MATH_DEFINES) # define M_PI etc for <cmath>
	
	set(warnings "/W3 /EHsc")
	set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")
	
	setCXXFlag("/arch:AVX") # enable AVS
	setCXXFlag("/GL") # whole program optimization
	setCXXFlag("/MP") # Multi processor compilation
	setCXXFlag("/Ot") # Favor speed over size
	
	set(CMAKE_CONFIGURATION_TYPES Release Debug)
endif()
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") # GCC
	add_definitions(-DGCC)

	setCXXFlag("-mavx")
	setCXXFlag("-std=c++1y")
endif()






include_directories(${CMAKE_CURRENT_SOURCE_DIR}/JPEG-Encoder-SIMD)
add_subdirectory(JPEG-Encoder-SIMD)


