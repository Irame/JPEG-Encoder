// JPEG-Encoder.cpp : Definiert den Einstiegspunkt f�r die Konsolenanwendung.
//

#include "stdafx.h"
#include "Pixel.h"
#include <iostream>
#include "Image.h"

int main()
{
	Image img = Image();
	img.readPPM("C:\\Users\\Stefan\\Desktop\\test.ppm");
}

