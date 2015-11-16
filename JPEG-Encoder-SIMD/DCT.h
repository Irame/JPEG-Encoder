#pragma once
#include "PointerMatrix.h"

namespace DCT
{
	void directDCT(const PointerMatrix& values);
	void dct_ii(const PointerMatrix values);
	void seperateDCT(const PointerMatrix& values);

	void kokDCT(const PointerMatrix& values);
};

