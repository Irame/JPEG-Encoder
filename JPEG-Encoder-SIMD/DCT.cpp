#include "stdafx.h"
#include "DCT.h"
#include <vector>
#include "PointerMatrix.h"

void DCT::directDCT(const PointerMatrix& values)
{
	// use 8x8 blocks for DCT
	int N = 8;
	// alocate memory to store the results
	float memory[64]; 
	PointerMatrix M = PointerMatrix(memory);

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

	// copy calculated values into the provided values
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			values[i][j] = M[i][j];
		}
	}
}

void DCT::seperateDCT(const PointerMatrix& values)
{
	// use 8x8 blocks for DCT
	int N = 8;

	// allocate memory
	float aValues[64];
	float resultValues[64];
	float saveResult[64];
	float temp;

	PointerMatrix a = PointerMatrix(aValues);
	PointerMatrix result = PointerMatrix(resultValues);
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

	// copy results to input matrix
	for (i = 0; i < N; i++) 
	{
		for (j = 0; j < N; j++)
		{
			values[i][j] = result[i][j];
		}
	}
}