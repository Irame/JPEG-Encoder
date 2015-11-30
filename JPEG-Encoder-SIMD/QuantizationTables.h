#pragma once
#include "PointerMatrix.h"

namespace JPEGQuantization 
{
	// Two examples of quantization tables are given in Tables K.1 and K.2. These are based on psychovisual thresholding and
    // are derived empirically using luminance and chrominance and 2:1 horizontal subsampling.These tables are provided as
    // examples only and are not necessarily suitable for any particular application.These quantization values have been used
    // with good results on 8 - bit per sample luminance and chrominance images of the format illustrated in Figure 13. Note that
    // these quantization values are appropriate for the DCT normalization defined in A.3.3.
    // If these quantization values are divided by 2, the resulting reconstructed image is usually nearly indistinguishable from the
    // source image.

	// Tables of JPEG Specifications
	static const __m256 luminance[8] {
		16, 11, 10, 16, 24,  40,  51,  61,
		12, 12, 14, 19, 26,  58,  60,  55,
		14, 13, 16, 24, 40,  57,  69,  56,
		14, 17, 22, 29, 51,  87,  80,  62,
		18, 22, 37, 56, 68,  109, 103, 77,
		24, 35, 55, 64, 81,  104, 113, 92,
		49, 64, 78, 87, 103, 121, 120, 101,
		72, 92, 95, 98, 112, 100, 103, 99 
	};

	static const __m256 chrominance[8] { 
		17, 18, 24, 47, 99, 99, 99, 99,
		18, 21, 26, 66, 99, 99, 99, 99,
		24, 26, 56, 99, 99, 99, 99, 99,
		47, 66, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99 
	};
}