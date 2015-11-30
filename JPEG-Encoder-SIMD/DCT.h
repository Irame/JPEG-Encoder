#pragma once
#include "PointerMatrix.h"
#include "QuantizationTables.h"

namespace DCT
{
	void directDCT(const PointerMatrix& values, PointerMatrix& result);
	void directIDCT(const PointerMatrix & values, PointerMatrix & result);
	void seperateDCT(const PointerMatrix& values, PointerMatrix& result);

	mat8x8 kokDCT(const mat8x8& x);
	mat8x8 kokSimple(const mat8x8& x);

	void araiDCT(const PointerMatrix& in, PointerMatrix& out);
	void araiDCTAVX(const PointerMatrix& in, PointerMatrix& out);
	void DCT::araiDCTandQuantisationAVX(const PointerMatrix& in, const QTable& qTable, PointerMatrix& out);
};
