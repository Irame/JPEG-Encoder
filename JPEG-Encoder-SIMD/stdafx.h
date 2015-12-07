// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#include <stdio.h>
#include <tchar.h>
#include <iostream>

#define NUM_BYTE_VALUES 256

#define M_PIf ((float)M_PI)
#define M_PI_2f ((float)M_PI_2)
#define M_SQRT2f ((float)M_SQRT2)
#define M_SQRT1_2f ((float)M_SQRT1_2)

typedef unsigned char byte;

struct BEushort // datatype that swaps byteorder to have the correct order for serialization
{
	BEushort() : value(0) {};

	BEushort(unsigned short v)
	{
		value = _byteswap_ushort(v);
	}

	operator unsigned short() const
	{
		return _byteswap_ushort(value);
	}

private:
	unsigned short value;
};


inline std::ostream& operator<<(std::ostream& os, const byte& byte) {
	os << static_cast<int>(byte);
	return os;
}

// memcpy_s is a non standard VS extension; Alias memcpy_s to memcpy on GCC
#ifdef GCC
	#define memcpy_s(_dst, _dstSize, _src, _srcSize) memcpy(_dst, _src, _dstSize)
#endif


// TODO: reference additional headers your program requires here
