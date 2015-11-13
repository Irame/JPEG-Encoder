// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <iostream>

#define NUM_BYTE_VALUES 256

typedef unsigned char byte;

inline std::ostream& operator<<(std::ostream& os, const byte& byte) {
	os << static_cast<int>(byte);
	return os;
}


// TODO: reference additional headers your program requires here
