#pragma once

// Taken from https://github.com/pkeir/ctfft/blob/master/const_math.hpp and slightly modified

#ifndef _CONST_MATH_HPP_
#define _CONST_MATH_HPP_

/*
C++11 constexpr versions of cmath functions needed for the FFT.
Copyright (C) 2012 Paul Keir
Distributed under the GNU General Public License. See license.txt for details.
*/

#include <limits> // nan

constexpr double tol = 0.001;

constexpr double c_abs(const double x) { return x < 0.0 ? -x : x; }

constexpr double square(const double x) { return x*x; }

constexpr double sqrt_helper(const double x, const double g) {
	return c_abs(g - x / g) < tol ? g : sqrt_helper(x, (g + x / g) / 2.0);
}

constexpr double c_sqrt(const double x) { return sqrt_helper(x, 1.0); }

constexpr double cube(const double x) { return x*x*x; }

// Based on the triple-angle formula: sin 3x = 3 sin x - 4 sin ^3 x
constexpr
double sin_helper(const double x) {
	return x < tol ? x : 3 * (sin_helper(x / 3.0)) - 4 * cube(sin_helper(x / 3.0));
}

constexpr
double c_sin(const double x) {
	return sin_helper(x < 0 ? -x + M_PI : x);
}

constexpr double c_cos(const double x) { return c_sin(M_PI_2 - x); }




#endif // _CONST_MATH_HPP_