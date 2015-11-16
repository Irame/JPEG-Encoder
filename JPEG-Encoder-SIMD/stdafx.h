// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define _USE_MATH_DEFINES
#include <cmath>

#include <stdio.h>
#include <tchar.h>
#include <iostream>

#define NUM_BYTE_VALUES 256

#define M_PIf ((float)M_PI)

typedef unsigned char byte;

inline std::ostream& operator<<(std::ostream& os, const byte& byte) {
	os << static_cast<int>(byte);
	return os;
}


// TODO: reference additional headers your program requires here
