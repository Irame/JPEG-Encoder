#pragma once
#include "PointerMatrix.h"

namespace DCT
{
	void directDCT(const PointerMatrix& values);
	void seperateDCT(const PointerMatrix& values);

	mat8x8 kokDCT(const mat8x8& x);
	void kokSimple(const PointerMatrix& values);
};
