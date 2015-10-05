// JPEG-Encoder.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "Pixel.h"
#include <iostream>


int main()
{
	Pixel pixel(0.3, 0.1, 0.5);
	std::cout << pixel.getColorValue(Pixel::RGBColorName::G) << std::endl;
}

