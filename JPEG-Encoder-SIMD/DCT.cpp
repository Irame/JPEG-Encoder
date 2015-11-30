#include "stdafx.h"
#include "DCT.h"
#include <vector>
#include "PointerMatrix.h"
#include "const_math.h"
#include "SIMD.h"

void DCT::directDCT(const PointerMatrix& values, PointerMatrix& result)
{
	// use 8x8 blocks for DCT
	int N = 8;
	PointerMatrix& M = result;

	// calculate values that don't change
	float sqrtTemp = float(1.0 / sqrt(2));
	float twoN = float(2.0 * N);
	float halfN = float(2.0 / N);
	
	for (int i = 0; i < N; i++) {
		// calculate values that only change with i
		float iPI = i * M_PIf;
		float ci = i == 0 ? sqrtTemp : 1;

		for (int j = 0; j < N; j++) {
			// calculate values that only change with j
			float cj = j == 0 ? sqrtTemp : 1;
			float jPI = j * M_PIf;

			float temp = 0;
			
			for (int x = 0; x < N; x++) {
				float twoX = float(2.0 * x);
				for (int y = 0; y < N; y++) {
					temp += values[x][y] * cos(((twoX + 1) * iPI) / twoN) * cos(((2 * y + 1) * jPI) / twoN);
				}
			}
			M[i][j] = halfN * ci * cj * temp;
		}
	}
	// https://en.wikipedia.org/wiki/JPEG#Discrete_cosine_transform
	// subtract 1024 from the DC coefficient, which is mathematically equivalent
	// to center the provided data around zero.
	M[0][0] -= 1024;  
}

void DCT::directIDCT(const PointerMatrix& values, PointerMatrix& result)
{
	// use 8x8 blocks for DCT
	constexpr int N = 8;

	// calculate values that don't change
	constexpr float sqrtTemp = float(1.0 / c_sqrt(2));
	constexpr float twoN = float(2.0 * N);
	constexpr float halfN = float(2.0 / N);

	values[0][0] += 1024;

	for (int x = 0; x < N; x++) {
		// calculate values that only change with i
		float xFactor = M_PIf * (2 * x + 1) / twoN;

		for (int y = 0; y < N; y++) {
			// calculate values that only change with j
			float yFactor = M_PIf * (2 * y + 1) / twoN;

			float temp = 0;
			for (int i = 0; i < N; i++) {
				float ci = i == 0 ? sqrtTemp : 1;
				float cosi = cos(xFactor * i);
				for (int j = 0; j < N; j++) {
					float cj = j == 0 ? sqrtTemp : 1;
					float cosj = cos(yFactor * j);
					temp += values[i][j] * ci * cj * cosi * cosj;
				}
			}
			result[x][y] = halfN * temp;
		}
	}
}

void DCT::seperateDCT(const PointerMatrix& values, PointerMatrix& result)
{
	// use 8x8 blocks for DCT
	int N = 8;

	// allocate memory
	float aValues[64];
	float saveResult[64];
	float temp;

	PointerMatrix a = PointerMatrix(aValues);
	PointerMatrix saveResults = PointerMatrix(saveResult);

	float C = 0;
	for (int k = 0; k < N; k++)
	{
		for (int n = 0; n < N; n++)
		{
			C = k == 0.0 ? 1 / sqrtf(2.0) : 1;

			a[k][n] = C * sqrtf(2.0 / N)*cosf((2.0 * n + 1) * ((k * M_PIf) / (2.0 * N)));
		}
	}

	size_t i, j, k;

	for (i = 0; i < N; i++) // A x X
	{
		for (j = 0; j < N; j++)
		{
			result[i][j] = 0.0;
			for (k = 0; k < N; k++)
			{
				result[i][j] += a[i][k] * values[k][j];
				saveResults[i][j] = result[i][j];
			}
		}
	}

	for (i = 0; i < N; i++) // (A x X) x aT
	{
		for (j = 0; j < N; j++)
		{
			result[i][j] = 0.0;
			for (k = 0; k < N; k++)
			{
				result[i][j] += saveResults[i][k] * a.atT(k, j);
			}
		}
	}

	// https://en.wikipedia.org/wiki/JPEG#Discrete_cosine_transform
	// subtract 1024 from the DC coefficient, which is mathematically equivalent
	// to center the provided data around zero.
	result[0][0] -= 1024;
}

mat8x8 DCT::kokDCT(const mat8x8& x)
{
	const size_t N = 64;
	const size_t N_2 = N / 2;

	mat8x8 X; // Output
	float p[N_2]; // p(n) für C(i)
	float q[N_2]; // q(n) für D'(i)
	float D_i[N_2]; // cache for D(i)

	// Calculate new sequences p(n) and q(n) 
	for (size_t n = 0; n <= N_2 - 1; n++)
	{
		p[n] = x[n] + x[N - 1 - n];
		q[n] = (x[n] - x[N - 1 - n]) * 2 * cosf((2 * M_PIf *(2 * n + 1)) / (4.0f*N));
	}

	
	// Calc C(0); D(0)
	D_i[0] = 0.0f;
	for (size_t n = 0; n <= N - 1; n++)
	{
		// C(0)=sum(x[n])
		X[0] += x[n];

		// calc D(i=0) with X(2i+1)
		// D(0)=sum(x[n]*cos((n*pi + pi/2) / N))
		D_i[0] += x[n] * cosf((n*M_PIf + M_PI_2f) / N);
	}
	X[1] = D_i[0];

	// Calc i=[1;N/2-1]
	size_t pos = 1;
	for (size_t i = 1; i <= N_2 - 1; i++)
	{
		float c = 0.0f;  // C(i)
		float d_ = 0.0f; // D'(i)
		for (size_t n = 0; n <= N_2 - 1; n++)
		{
			float cos = cosf(((n*M_PIf + M_PI_2f) / N) * 2 * i);
			c += p[n] * cos;
			d_ += q[n] * cos;
		}

		// D(i) = D'(i) - D(i-1)
		float d = d_ - D_i[i - 1];

		X[++pos] = c;
		X[++pos] = d;

		D_i[i] = d; // store D(i) for recursion
	}

	return X;
}


mat8x8 DCT::kokSimple(const mat8x8& x)
{
	size_t N = 64;
	mat8x8 X; // Output

	for (int k = 0; k < N; k++)
	{
		for (int n = 0; n < N; n++)
		{
			X[k] += x[n] * cos(M_PIf / (2 * N) * (2 * n + 1) * k);
		}
	}

	return X;
}

// allready defined in SIMD.h
//constexpr float C_(size_t k) { return k == 0 ? 1.0f : (float)c_cos(k * M_PIf / 16); }
//constexpr float S_(size_t k) { return k == 0 ? M_SQRT1_2f / 2.0f : 1.0f / (4.0f * C_(k)); }
//float C_(size_t k) { return k == 0 ? 1.0f : (float)cosf(k * M_PIf / 16); }
//float S_(size_t k) { return k == 0 ? M_SQRT1_2f / 2.0f : 1.0f / (4.0f * C_(k)); }
void DCT::araiDCT(const PointerMatrix& in, PointerMatrix& out)
{
	constexpr float s0 = S_(0);
	constexpr float s1 = S_(1);
	constexpr float s2 = S_(2);
	constexpr float s3 = S_(3);
	constexpr float s4 = S_(4);
	constexpr float s5 = S_(5);
	constexpr float s6 = S_(6);
	constexpr float s7 = S_(7);

	constexpr float a1 = C_(4);
	constexpr float a2 = C_(2) - C_(6);
	constexpr float a3 = C_(4);
	constexpr float a4 = C_(6) + C_(2);
	constexpr float a5 = C_(6);

	for (size_t row = 0; row < 8; row++) {
		float temp1[8];
		temp1[0] = in.at(row, 0) + in.at(row, 7);
		temp1[1] = in.at(row, 1) + in.at(row, 6);
		temp1[2] = in.at(row, 2) + in.at(row, 5);
		temp1[3] = in.at(row, 3) + in.at(row, 4);
		temp1[4] = in.at(row, 3) - in.at(row, 4);
		temp1[5] = in.at(row, 2) - in.at(row, 5);
		temp1[6] = in.at(row, 1) - in.at(row, 6);
		temp1[7] = in.at(row, 0) - in.at(row, 7);

		float temp2[8];
		temp2[0] = temp1[0] + temp1[3];
		temp2[1] = temp1[1] + temp1[2];
		temp2[2] = temp1[1] - temp1[2];
		temp2[3] = temp1[0] - temp1[3];
		temp2[4] = -temp1[4] - temp1[5];
		temp2[5] = temp1[5] + temp1[6];
		temp2[6] = temp1[6] + temp1[7];
		temp2[7] = temp1[7];

		float temp3[8];
		temp3[0] = temp2[0] + temp2[1];
		temp3[1] = temp2[0] - temp2[1];
		temp3[2] = temp2[2] + temp2[3];
		temp3[3] = temp2[3];
		temp3[4] = temp2[4];
		temp3[5] = temp2[5];
		temp3[6] = temp2[6];
		temp3[7] = temp2[7];


		float temp6plus4 = temp3[4] + temp3[6];
		temp3[2] *= a1;
		temp3[4] = -temp3[4] * a2 - temp6plus4 * a5;
		temp3[5] *= a3;
		temp3[6] = temp3[6] * a4 - temp6plus4 * a5;

		float temp = temp3[2];
		temp3[2] += temp3[3];
		temp3[3] -= temp;
		temp = temp3[5];
		temp3[5] += temp3[7];
		temp3[7] -= temp;

		float temp4[8];
		out.at(row, 0) = temp3[0];
		out.at(row, 4) = temp3[1];
		out.at(row, 2) = temp3[2];
		out.at(row, 6) = temp3[3];
		out.at(row, 5) = temp3[4] + temp3[7];
		out.at(row, 1) = temp3[5] + temp3[6];
		out.at(row, 7) = temp3[5] - temp3[6];
		out.at(row, 3) = temp3[7] - temp3[4];

		out.at(row, 0) *= s0;
		out.at(row, 4) *= s4;
		out.at(row, 2) *= s2;
		out.at(row, 6) *= s6;
		out.at(row, 5) *= s5;
		out.at(row, 1) *= s1;
		out.at(row, 7) *= s7;
		out.at(row, 3) *= s3;
	}

	for (size_t row = 0; row < 8; row++) {
		float temp1[8];
		temp1[0] = out.atT(row, 0) + out.atT(row, 7);
		temp1[1] = out.atT(row, 1) + out.atT(row, 6);
		temp1[2] = out.atT(row, 2) + out.atT(row, 5);
		temp1[3] = out.atT(row, 3) + out.atT(row, 4);
		temp1[4] = out.atT(row, 3) - out.atT(row, 4);
		temp1[5] = out.atT(row, 2) - out.atT(row, 5);
		temp1[6] = out.atT(row, 1) - out.atT(row, 6);
		temp1[7] = out.atT(row, 0) - out.atT(row, 7);

		float temp2[8];
		temp2[0] = temp1[0] + temp1[3];
		temp2[1] = temp1[1] + temp1[2];
		temp2[2] = temp1[1] - temp1[2];
		temp2[3] = temp1[0] - temp1[3];
		temp2[4] = -temp1[4] - temp1[5];
		temp2[5] = temp1[5] + temp1[6];
		temp2[6] = temp1[6] + temp1[7];
		temp2[7] = temp1[7];

		float temp3[8];
		temp3[0] = temp2[0] + temp2[1];
		temp3[1] = temp2[0] - temp2[1];
		temp3[2] = temp2[2] + temp2[3];
		temp3[3] = temp2[3];
		temp3[4] = temp2[4];
		temp3[5] = temp2[5];
		temp3[6] = temp2[6];
		temp3[7] = temp2[7];


		float temp6plus4 = temp3[4] + temp3[6];
		temp3[2] *= a1;
		temp3[4] = -temp3[4] * a2 - temp6plus4 * a5;
		temp3[5] *= a3;
		temp3[6] = temp3[6] * a4 - temp6plus4 * a5;

		float temp = temp3[2];
		temp3[2] += temp3[3];
		temp3[3] -= temp;
		temp = temp3[5];
		temp3[5] += temp3[7];
		temp3[7] -= temp;

		out.atT(row, 0) = temp3[0];
		out.atT(row, 4) = temp3[1];
		out.atT(row, 2) = temp3[2];
		out.atT(row, 6) = temp3[3];
		out.atT(row, 5) = temp3[4] + temp3[7];
		out.atT(row, 1) = temp3[5] + temp3[6];
		out.atT(row, 7) = temp3[5] - temp3[6];
		out.atT(row, 3) = temp3[7] - temp3[4];

		out.atT(row, 0) *= s0;
		out.atT(row, 4) *= s4;
		out.atT(row, 2) *= s2;
		out.atT(row, 6) *= s6;
		out.atT(row, 5) *= s5;
		out.atT(row, 1) *= s1;
		out.atT(row, 7) *= s7;
		out.atT(row, 3) *= s3;
	}


	// https://en.wikipedia.org/wiki/JPEG#Discrete_cosine_transform
	// subtract 1024 from the DC coefficient, which is mathematically equivalent
	// to center the provided data around zero.
	out[0][0] -= 1024;
}

void DCT::araiDCTAVX(const PointerMatrix& in, PointerMatrix& out)
{
	twoDimensionalDCTAVX(in, out);
}
