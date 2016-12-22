project(JPEG-Encoder)
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

macro(setCXXFlag flag)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
endmacro()

macro(setCXXFlag_Debug flag)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${flag}")
endmacro()

macro(setCXXFlag_Release flag)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${flag}")
endmacro()

macro(setLinkerFlag flag)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${flag}")
endmacro()

macro(setLinkerFlag_Debug flag)
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${flag}")
endmacro()

macro(setLinkerFlag_Release flag)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${flag}")
endmacro()

# BOOST
set(BOOST_ROOT boost/)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# Intel OpenCL SDK for Applications
include_directories($ENV{INTELOCLSDKROOT}/include)
link_directories($ENV{INTELOCLSDKROOT}lib/x64)
setLinkerFlag("OpenCL.lib")

# extension to embed recources
include_directories(embed-resource)
add_subdirectory(embed-resource)

# Common VS Project Properties for MSVC, Clang, Intel
if ("${CMAKE_GENERATOR}" MATCHES "Visual Studio*")
	set(warnings "/W3 /EHsc")
	set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")
	
	setCXXFlag("/arch:AVX") # enable AVX
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_definitions(-D MSVC) # define MSVC
	add_definitions(-D _USE_MATH_DEFINES) # define M_PI etc for <cmath>
	
	setCXXFlag("/MP") 		  # Multi processor compilation
	
	setCXXFlag_Debug("/MTd")	   # use the multithread, static version of the run-time library in debug
	
	setCXXFlag_Release("/Ot") 	   # Favor speed over size
	setCXXFlag_Release("/Oi") 	   # Generate Intrinsic Functions	
	setCXXFlag_Release("/GL") 	   # whole program optimization
	setCXXFlag_Release("/MT")	   # use the multithread, static version of the run-time library
	setCXXFlag_Release("/openmp")  # enables OpenMP (set to '/openmp-' to disable it)
	
	setLinkerFlag_Release("/LTCG") # link time code generation
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	add_definitions(-D CLANG) # define CLANG
	add_definitions(-D _CRT_SECURE_NO_WARNINGS)
	
	setCXXFlag("-Wno-missing-braces")
	setCXXFlag("-Wno-unused-function")
	
	setCXXFlag_Debug("/MTd")	   # use the multithread, static version of the run-time library in debug
	
	setCXXFlag_Release("/MT")	   # use the multithread, static version of the run-time library
	setCXXFlag_Release("-openmp")  # enables OpenMP (set to '/openmp-' to disable it)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	add_definitions(-D INTEL) # define INTEL
#	add_definitions(-D AVX512) # enable AVX512 implementation
	
# Workaround for missing macros with VS2015 Update 1
# https://software.intel.com/en-us/articles/limits1120-error-identifier-builtin-nanf-is-undefined

	add_definitions(-D "__builtin_huge_val()=HUGE_VAL" 
					-D "__builtin_huge_valf()=HUGE_VALF" 
					-D __builtin_nan=nan 
					-D __builtin_nanf=nanf 
					-D __builtin_nans=nan 
					-D __builtin_nansf=nanf )
					
	setCXXFlag_Debug("/MTd")	   # use the multithread, static version of the run-time library in debug
	
	setCXXFlag_Release("/MT")	   # use the multithread, static version of the run-time library
	setCXXFlag_Release("/Qopenmp")  # enables OpenMP (set to '/openmp-' to disable it)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") # GCC
	add_definitions(-DGCC)

	setCXXFlag("-mavx")		 # Use AVX instructions
	setCXXFlag("-std=c++1y") # Use latest C++ standard
	
	setCXXFlag_Release("-fopenmp")  # enables OpenMP (set to '/openmp-' to disable it)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/JPEG-Encoder-SIMD)
add_subdirectory(JPEG-Encoder-SIMD)

if ("${CMAKE_GENERATOR}" MATCHES "Visual Studio*")
    get_filename_component(smartCmdArgJsonFile "JPEG-Encoder-SIMD.args.json" ABSOLUTE)
    MESSAGE(STATUS "Path to json file: " ${smartCmdArgJsonFile})

    set_property(TARGET JPEG-Encoder-SIMD PROPERTY VS_GLOBAL_SmartCmdArgJsonFile ${smartCmdArgJsonFile})
endif()
