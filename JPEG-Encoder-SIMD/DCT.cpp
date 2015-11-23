#include "stdafx.h"
#include "DCT.h"
#include <vector>
#include "PointerMatrix.h"

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
				result[i][j] += saveResults[i][k] * a.atTransposed(k, j);
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
