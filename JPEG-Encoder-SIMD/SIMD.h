#pragma once

#include <immintrin.h>
#include <intrin.h>

// https://software.intel.com/sites/default/files/m/d/4/1/d/8/Image_Processing_-_whitepaper_-_100pct_CCEreviewed_update.pdf
// https://gist.github.com/rygorous/4172889

static void transposeFloatSSE(float *pSrc, float *pDst, unsigned int imageSize)
{
	if (imageSize % 16 != 0)
	{
		abort();
	}

	__m128 vfT0, vfP0, vfT2, vfP2;
	for (unsigned int i = 0; i<imageSize * 4; i += 16)
	{
		vfT0 = _mm_unpacklo_ps(_mm_load_ps(pSrc + i), _mm_load_ps(pSrc + i + 4)); // R0R1G0G1
		vfP0 = _mm_unpackhi_ps(_mm_load_ps(pSrc + i), _mm_load_ps(pSrc + i + 4)); // B0B1A0A1
		vfT2 = _mm_unpacklo_ps(_mm_load_ps(pSrc + i + 8), _mm_load_ps(pSrc + i + 12)); // R2R3G2G3
		vfP2 = _mm_unpackhi_ps(_mm_load_ps(pSrc + i + 8), _mm_load_ps(pSrc + i + 12)); // B2B3A2A3
		_mm_store_ps(pDst + i, _mm_movelh_ps(vfT0, vfT2)); // R0R1R2R3
		_mm_store_ps(pDst + i + 4, _mm_movehl_ps(vfT2, vfT0)); // G0G1G2G3
		_mm_store_ps(pDst + i + 8, _mm_movelh_ps(vfP0, vfP2)); // B0B1B2B3
		_mm_store_ps(pDst + i + 12, _mm_movehl_ps(vfP2, vfP0)); // A0A1A2A3
	}
}

static void transposeFloatAVX(float *pSrc, float *pDst, unsigned int imageSize)
{
	if (imageSize % 32 != 0)
	{
		abort();
	}

	__m256 ld0, ld1, ld2, ld3;
	__m256 pm0, pm1, pm2, pm3;
	__m256 up0, up1, up2, up3;
	for (unsigned int i = 0; i<imageSize * 4; i += 32)
	{
		ld0 = _mm256_load_ps(pSrc + i);
		ld1 = _mm256_load_ps(pSrc + i + 8);
		ld2 = _mm256_load_ps(pSrc + i + 16);
		ld3 = _mm256_load_ps(pSrc + i + 24);
		pm0 = _mm256_permute2f128_ps(ld0, ld2, 0x20); // R0G0B0A0 R4G4B4A4
		pm1 = _mm256_permute2f128_ps(ld1, ld3, 0x20); // R2G2B2A2 R6G6B6A6
		pm2 = _mm256_permute2f128_ps(ld0, ld2, 0x31); // R1G1B1A1 R5G5B5A5
		pm3 = _mm256_permute2f128_ps(ld1, ld3, 0x31); // R3G3B3A3 R7G7B7A7
		up0 = _mm256_unpacklo_ps(pm0, pm1); // R0R2G0G2 R4R6G4G6
		up1 = _mm256_unpackhi_ps(pm0, pm1); // B0B2A0A2 B4B6A4A6
		up2 = _mm256_unpacklo_ps(pm2, pm3); // R1R3G1G3 R5R7G5G7
		up3 = _mm256_unpackhi_ps(pm2, pm3); // B1B3A1A3 B5B7A5A7
		_mm256_store_ps(pDst + i, _mm256_unpacklo_ps(up0, up2)); // R0R1R2R3 R4R5R6R7
		_mm256_store_ps(pDst + i + 8, _mm256_unpackhi_ps(up0, up2)); // G0G1G2G3 G4G5G6G7 
		_mm256_store_ps(pDst + i + 16, _mm256_unpacklo_ps(up1, up3)); // B0B1B2B3 B4B5B6B7
		_mm256_store_ps(pDst + i + 24, _mm256_unpackhi_ps(up1, up3)); // A0A1A2A3 A4A5A6A7
	}
}

static void transposeFloatAVX_reverse(float *pSrc, float *pDst, unsigned int imageSize)
{
	__m256 ld0, ld1, ld2, ld3;
	__m256 up0, up1, up2, up3;
	__m256 upb0, upb1, upb2, upb3;
	for (unsigned int i = 0; i<imageSize * 4; i += 32)
	{
		ld0 = _mm256_load_ps(pSrc + i);
		ld1 = _mm256_load_ps(pSrc + i + 8);
		ld2 = _mm256_load_ps(pSrc + i + 16);
		ld3 = _mm256_load_ps(pSrc + i + 24);
		up0 = _mm256_unpacklo_ps(ld0, ld2); // R0B0R1B1 R4B4R5B5
		up1 = _mm256_unpackhi_ps(ld0, ld2); // R2B2R3B3 R6B6R7B7
		up2 = _mm256_unpacklo_ps(ld1, ld3); // G0A0G1A1 G4A4G5A5
		up3 = _mm256_unpackhi_ps(ld1, ld3); // G2A2G3A3 G6A6G7A7
		upb0 = _mm256_unpacklo_ps(up0, up2); // R0G0B0A0 R4G4B4A4
		upb1 = _mm256_unpackhi_ps(up0, up2); // R1G1B1A1 R5G5B5A5
		upb2 = _mm256_unpacklo_ps(up1, up3); // R2G2B2A2 R6G6B6A6
		upb3 = _mm256_unpackhi_ps(up1, up3); // R3G3B3A3 R7G7B7A7
											 // R0G0B0A0 R1G1B1A1
		_mm256_store_ps(pDst + i, _mm256_permute2f128_ps(upb0, upb1, 0x20));
		// R2G2B2A2 R3G3B3A3
		_mm256_store_ps(pDst + i + 8, _mm256_permute2f128_ps(upb2, upb3, 0x20));
		// R4G4B4A4 R5G5B5A5
		_mm256_store_ps(pDst + i + 16, _mm256_permute2f128_ps(upb0, upb1, 0x31));
		// R6G6B6A6 R7G7B7A7
		_mm256_store_ps(pDst + i + 24, _mm256_permute2f128_ps(upb2, upb3, 0x31));
	}
}

union Mat44 {
	float m[4][4];
	__m128 row[4];
};

union Vec4
{
	float v[4];
	__m128 vals;
};

// dual linear combination using AVX instructions on YMM regs
static inline __m256 twolincomb_AVX_8(__m256 A01, const Mat44 &B)
{
	__m256 result;
	result = _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0x00), _mm256_broadcast_ps(&B.row[0]));
	result = _mm256_add_ps(result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0x55), _mm256_broadcast_ps(&B.row[1])));
	result = _mm256_add_ps(result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0xaa), _mm256_broadcast_ps(&B.row[2])));
	result = _mm256_add_ps(result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0xff), _mm256_broadcast_ps(&B.row[3])));
	return result;
}

static void matmult_AVX_8(Mat44 &out, const Mat44 &A, const Mat44 &B)
{
	_mm256_zeroupper();
	__m256 A01 = _mm256_loadu_ps(&A.m[0][0]);
	__m256 A23 = _mm256_loadu_ps(&A.m[2][0]);

	__m256 out01x = twolincomb_AVX_8(A01, B);
	__m256 out23x = twolincomb_AVX_8(A23, B);

	_mm256_storeu_ps(&out.m[0][0], out01x);
	_mm256_storeu_ps(&out.m[2][0], out23x);
}

template<int shuffleNum>
static inline void calcOneRowRGBToYCbCr(float* refFloatPtr, __m128& row, __m256& V0, __m256& V1, __m256& V2, __m256& V3, __m256& vAdd)
{
	__m256 MRow = _mm256_broadcast_ps(&row);
	__m256 resultRow = _mm256_mul_ps(_mm256_shuffle_ps(MRow, MRow, 0x00), V0);
	resultRow = _mm256_add_ps(resultRow, _mm256_mul_ps(_mm256_shuffle_ps(MRow, MRow, 0x55), V1));
	resultRow = _mm256_add_ps(resultRow, _mm256_mul_ps(_mm256_shuffle_ps(MRow, MRow, 0xaa), V2));
	resultRow = _mm256_add_ps(resultRow, _mm256_mul_ps(_mm256_shuffle_ps(MRow, MRow, 0xff), V3));
	resultRow = _mm256_add_ps(resultRow, _mm256_shuffle_ps(vAdd, vAdd, shuffleNum));
	_mm256_storeu_ps(refFloatPtr, resultRow);
}

static void convertRGBToYCbCrAVXImpl(PixelData32T8 &ref)
{
	static Mat44 M = { 
         0.2990f,  0.5870f,  0.1140f, 0.0f,
		-0.1687f, -0.3312f,  0.5000f, 0.0f,
		 0.5000f, -0.4186f, -0.0813f, 0.0f,
		 0.0f,     0.0f,     0.0f,    1.0f };
	static Vec4 V = { 0.0f, 0.5f, 0.5f, 0.0f };

	float* refFloatPtr = (float*)&ref;
	
	__m256 V0 = _mm256_loadu_ps(refFloatPtr);
	__m256 V1 = _mm256_loadu_ps(refFloatPtr + 8);
	__m256 V2 = _mm256_loadu_ps(refFloatPtr + 16);
	__m256 V3 = _mm256_loadu_ps(refFloatPtr + 24);

	__m256 vAdd = _mm256_broadcast_ps(&V.vals);

	calcOneRowRGBToYCbCr<0x00>(refFloatPtr     , M.row[0], V0, V1, V2, V3, vAdd);
	calcOneRowRGBToYCbCr<0x55>(refFloatPtr + 8 , M.row[1], V0, V1, V2, V3, vAdd);
	calcOneRowRGBToYCbCr<0xaa>(refFloatPtr + 16, M.row[2], V0, V1, V2, V3, vAdd);
	calcOneRowRGBToYCbCr<0xff>(refFloatPtr + 24, M.row[3], V0, V1, V2, V3, vAdd);
}

static void convertYCbCrToRGBAVXImpl(PixelData32T8 &ref)
{
	static Mat44 M = { 
		1.0000f,  0.0000f,  1.4021f, 0.0f,
		1.0000f, -0.3442f, -0.7142f, 0.0f,
		1.0000f,  1.7720f,  0.0000f, 0.0f,
		0.0f,     0.0f,      0.0f,   1.0f };
	static Vec4 V = { 0.0f, -0.5f, -0.5f, 0.0f };

	float* refFloatPtr = (float*)&ref;
	
	__m256 V0 = _mm256_loadu_ps(refFloatPtr);
	__m256 V1 = _mm256_loadu_ps(refFloatPtr + 8);
	__m256 V2 = _mm256_loadu_ps(refFloatPtr + 16);
	__m256 V3 = _mm256_loadu_ps(refFloatPtr + 24);

	__m256 vAdd = _mm256_broadcast_ps(&V.vals);

	for (int i = 0; i < 4; i++) {
		__m256 MRow = _mm256_broadcast_ps(&M.row[i]);

		__m256 resultRow = _mm256_mul_ps(_mm256_shuffle_ps(MRow, MRow, 0x00), _mm256_add_ps(V0, _mm256_shuffle_ps(vAdd, vAdd, 0x00)));
		resultRow = _mm256_add_ps(resultRow, _mm256_mul_ps(_mm256_shuffle_ps(MRow, MRow, 0x55), _mm256_add_ps(V1, _mm256_shuffle_ps(vAdd, vAdd, 0x55))));
		resultRow = _mm256_add_ps(resultRow, _mm256_mul_ps(_mm256_shuffle_ps(MRow, MRow, 0xaa), _mm256_add_ps(V2, _mm256_shuffle_ps(vAdd, vAdd, 0xaa))));
		resultRow = _mm256_add_ps(resultRow, _mm256_mul_ps(_mm256_shuffle_ps(MRow, MRow, 0xff), _mm256_add_ps(V3, _mm256_shuffle_ps(vAdd, vAdd, 0xff))));

		_mm256_storeu_ps(refFloatPtr + i*8, resultRow);
	}
}

static void applySepiaFilterAVXImpl(PixelData32T8 &ref)
{
	static Mat44 M = {
		0.3930f,  0.7690f,  0.1890f, 0.0f,
		0.3490f,  0.6860f,  0.1680f, 0.0f,
		0.2720f,  0.5340f,  0.1310f, 0.0f,
		0.0f,     0.0f,     0.0f,    1.0f };

	float* refFloatPtr = (float*)&ref;

	__m256 V0 = _mm256_loadu_ps(refFloatPtr);
	__m256 V1 = _mm256_loadu_ps(refFloatPtr + 8);
	__m256 V2 = _mm256_loadu_ps(refFloatPtr + 16);
	__m256 V3 = _mm256_loadu_ps(refFloatPtr + 24);

	for (int i = 0; i < 4; i++) {
		__m256 MRow = _mm256_broadcast_ps(&M.row[i]);

		__m256 resultRow = _mm256_mul_ps(_mm256_shuffle_ps(MRow, MRow, 0x00), V0);
		resultRow = _mm256_add_ps(resultRow, _mm256_mul_ps(_mm256_shuffle_ps(MRow, MRow, 0x55), V1));
		resultRow = _mm256_add_ps(resultRow, _mm256_mul_ps(_mm256_shuffle_ps(MRow, MRow, 0xaa), V2));
		resultRow = _mm256_add_ps(resultRow, _mm256_mul_ps(_mm256_shuffle_ps(MRow, MRow, 0xff), V3));

		_mm256_storeu_ps(refFloatPtr + i * 8, resultRow);
	}
}