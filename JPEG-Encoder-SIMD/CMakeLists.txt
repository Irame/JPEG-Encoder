set(CPP_FILES 
	BitBuffer.cpp
	CLDCT.cpp
	DCT.cpp
	Encoder.cpp
	Image.cpp
	ImageLoader.cpp
	JPEG-Encoder-SIMD.cpp
	lodepng.cpp
	stdafx.cpp
) # CPP_FILES
	
set(H_FILES	
	Benchmark.h
	BitBuffer.h
	CLDCT.h
	ColorNames.h
	const_math.h
	DCT.h
	Dimension2D.h
	Encoder.h
	HuffmanCoding.h
	HuffmanCodingByte.h
	Image.h
	ImageLoader.h
	JPEGSegments.h
	LinkedList.h
	lodepng.h
	PointerMatrix.h
	QuantizationTables.h
	SamplingScheme.h
	SIMD.h
	SIMD256.h
	SIMD512.h
	SortedLinkedList.h
	stdafx.h
	targetver.h
	ZigZag.h
) # H_FILES

set(CL_FILES
	TwoDimDct.cl
) # CL_FILES

embed_resources(CL_RESOURCES ${CL_FILES})

add_executable(JPEG-Encoder-SIMD ${CPP_FILES} ${H_FILES} ${CL_FILES} ${CL_RESOURCES})

SOURCE_GROUP("Source Files" FILES ${CPP_FILES})
SOURCE_GROUP("Header Files" FILES ${H_FILES})
SOURCE_GROUP("OpenCL Files" FILES ${CL_FILES})
SOURCE_GROUP("OpenCL Recources" FILES ${CL_RESOURCES})
