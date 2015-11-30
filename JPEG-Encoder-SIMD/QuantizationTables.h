#pragma once
#include "PointerMatrix.h"

union QTable {
	float floats[64];
	__m256 avx[8];
};

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
	static const QTable luminance {
		16, 11, 10, 16, 24,  40,  51,  61,
		12, 12, 14, 19, 26,  58,  60,  55,
		14, 13, 16, 24, 40,  57,  69,  56,
		14, 17, 22, 29, 51,  87,  80,  62,
		18, 22, 37, 56, 68,  109, 103, 77,
		24, 35, 55, 64, 81,  104, 113, 92,
		49, 64, 78, 87, 103, 121, 120, 101,
		72, 92, 95, 98, 112, 100, 103, 99 
	};

	static const QTable chrominance { 
		17, 18, 24, 47, 99, 99, 99, 99,
		18, 21, 26, 66, 99, 99, 99, 99,
		24, 26, 56, 99, 99, 99, 99, 99,
		47, 66, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99 
	};

	// Aufgabe 5c: Besorgen Sie sich Quantisierungstabellen von Digitalkameras oder Photohandys – je nachdem was
	// für Sie verfügbar ist.Wie kommen Sie an die Quantisierungstabellen ?
	//
	// http://www.impulseadventure.com/photo/jpeg-quantization.html

	namespace Photoshop {
		// Quality 12
		static const QTable luminance12 {
			1,	1,	1,	1,	1,	1,	1,	2,
			1,	1,	1,	1,	1,	1,	1,	2,
			1,	1,	1,	1,	1,	1,	2,	2,
			1,	1,	1,	1,	1,	2,	2,	3,
			1,	1,	1,	1,	2,	2,	3,	3,
			1,	1,	1,	2,	2,	3,	3,	3,
			1,	1,	2,	2,	3,	3,	3,	3,
			2,	2,	2,	3,	3,	3,	3,	3,
		};
		static const QTable chrominance12 {
			1,	1,	1,	2,	3,	3,	3,	3,
			1,	1,	1,	2,	3,	3,	3,	3,
			1,	1,	2,	3,	3,	3,	3,	3,
			2,	2,	3,	3,	3,	3,	3,	3,
			3,	3,	3,	3,	3,	3,	3,	3,
			3,	3,	3,	3,	3,	3,	3,	3,
			3,	3,	3,	3,	3,	3,	3,	3,
			3,	3,	3,	3,	3,	3,	3,	3,
		};

		// Quality 5
		static const QTable luminance5 {
			12,	8 ,	13,	21,	26,	32,	34,	17,
			8 ,	9 ,	12,	20,	27,	23,	12,	12,
			13,	12,	16,	26,	23,	12,	12,	12,
			21,	20,	26,	23,	12,	12,	12,	12,
			26,	27,	23,	12,	12,	12,	12,	12,
			32,	23,	12,	12,	12,	12,	12,	12,
			34,	12,	12,	12,	12,	12,	12,	12,
			17,	12,	12,	12,	12,	12,	12,	12,
		};
		static const QTable chrominance5 {
			13,	13,	17,	27,	20,	20,	17,	17,
			13,	14,	17,	14,	14,	12,	12,	12,
			17,	17,	14,	14,	12,	12,	12,	12,
			27,	14,	14,	12,	12,	12,	12,	12,
			20,	14,	12,	12,	12,	12,	12,	12,
			20,	12,	12,	12,	12,	12,	12,	12,
			17,	12,	12,	12,	12,	12,	12,	12,
			17,	12,	12,	12,	12,	12,	12,	12,
		};
	}
}