#pragma once

#ifdef AVX512
#include "SIMD512.h"
#else
#include "SIMD256.h"
#endif