// JPEG-Encoder.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "Image.h"
#include <iostream>

int main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cout 
			<< "Error: No File given." << std::endl
			<< "Usage: JPEG-Encoder.exe <ppmFile>" << std::endl
			<< "Press any key to exit ..." << std::endl;
		std::cin.get();
		exit(1);
	}
	ImagePtr img = Image::readPPM(argv[1]);
	exit(0);
}

