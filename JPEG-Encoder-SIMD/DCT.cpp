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

	float sqrtTemp = 1 / sqrt(2);
	float twoN = 2 * N;
	float halfN = 2 / N;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			float ci = i == 0 ? sqrtTemp : 1;
			float cj = j == 0 ? sqrtTemp : 1;

			float temp = 0;
			float iPI = i*M_PIf;
			float jPI = j*M_PIf;
			
			for (int x = 0; x < N; x++) {
				for (int y = 0; y < N; y++) {
					temp += values[x][y] * cos(((2 * x + 1)*iPI) / twoN)*cos(((2 * y + 1)*jPI) / twoN);
				}
			}
			M[i][j] = halfN * ci * cj * temp;
		}
	}
	// copy calculated values into the provided values
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			values[i][j] = M[i][j];
		}
	}
}
