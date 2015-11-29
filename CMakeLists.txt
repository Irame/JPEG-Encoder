project(JPEG-Encoder)
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

# BOOST
set(BOOST_ROOT boost/)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)


include_directories(${Boost_INCLUDE_DIRS})



find_package(Boost COMPONENTS filesystem system thread date_time chrono locale program_options)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(warnings "/W4 /EHsc")
	set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_USE_MATH_DEFINES") # define M_PI etc in <cmath>
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GL") # whole program optimization
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") # Multi processor compilation
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ot") # Favor speed over size
	
	set(CMAKE_CONFIGURATION_TYPES Release Debug)
endif()




include_directories(${CMAKE_CURRENT_SOURCE_DIR}/JPEG-Encoder-SIMD)
add_subdirectory(JPEG-Encoder-SIMD)


