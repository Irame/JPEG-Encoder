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


# Common VS Project Properties for MSVC, Clang, Intel
if ("${CMAKE_GENERATOR}" MATCHES "Visual Studio*")
	set(warnings "/W3 /EHsc")
	set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")
	
	setCXXFlag("/arch:AVX") # enable AVX
	setCXXFlag("/GL") # whole program optimization
	setCXXFlag("/MP") # Multi processor compilation
	setCXXFlag("/Ot") # Favor speed over size
	
	set(CMAKE_CONFIGURATION_TYPES Release Debug)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_definitions(-DMSVC) # define MSVC
	add_definitions(-D_USE_MATH_DEFINES) # define M_PI etc for <cmath>
	

endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	add_definitions(-DCLANG) # define CLANG
	
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	add_definitions(-DINTEL) # define INTEL
	
# Workaround for missing macros with VS2015 Update 1
# https://software.intel.com/en-us/articles/limits1120-error-identifier-builtin-nanf-is-undefined

	add_definitions(-D"__builtin_huge_val()=HUGE_VAL" 
					-D"__builtin_huge_valf()=HUGE_VALF" 
					-D__builtin_nan=nan 
					-D__builtin_nanf=nanf 
					-D__builtin_nans=nan 
					-D__builtin_nansf=nanf)
	
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") # GCC
	add_definitions(-DGCC)

	setCXXFlag("-mavx")		 # Use AVX instructions
	setCXXFlag("-std=c++1y") # Use latest C++ standard
endif()






include_directories(${CMAKE_CURRENT_SOURCE_DIR}/JPEG-Encoder-SIMD)
add_subdirectory(JPEG-Encoder-SIMD)


