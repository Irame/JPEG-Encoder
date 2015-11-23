#pragma once
#include "PointerMatrix.h"

namespace DCT
{
	void directDCT(const PointerMatrix& values, PointerMatrix& result);
	void seperateDCT(const PointerMatrix& values, PointerMatrix& result);

	mat8x8 kokDCT(const mat8x8& x);
	mat8x8 kokSimple(const mat8x8& x);

	mat8x8 araiDCT(const mat8x8& x);
};
