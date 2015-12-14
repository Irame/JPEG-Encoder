set(CPP_FILES 
	BitBuffer.cpp
	DCT.cpp
	Encoder.cpp
	Image.cpp
	ImageLoader.cpp
	JPEG-Encoder-SIMD.cpp
	lodepng.cpp
	SamplingScheme.cpp
	stdafx.cpp
) # CPP_FILES
	
set(H_FILES	
	Benchmark.h
	BitBuffer.h
	ColorNames.h
	const_math.h
	DCT.h
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
	SortedLinkedList.h
	stdafx.h
	targetver.h
	ZigZag.h
) # H_FILES

add_executable(JPEG-Encoder-SIMD ${CPP_FILES} ${H_FILES})

SOURCE_GROUP("Source Files" FILES ${CPP_FILES})
SOURCE_GROUP("Header Files" FILES ${H_FILES})
