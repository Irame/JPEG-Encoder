// JPEG-Encoder.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "Image.h"
#include <iostream>

ColorCoding curCoding;

std::ostream& operator<<(std::ostream& strm, const ColorChannel& colorChannel)
{
	strm << "ColorChannel: ";
	if (curCoding == RGB)
		strm << colorChannel.getColorName().rgbColorName;
	else
		strm << colorChannel.getColorName().yCbCrColorName;
	strm << std::endl;

	for (int x = 0; x < colorChannel.getWidth(); x++)
	{
		strm << "[ ";
		for (int y = 0; y < colorChannel.getHeight(); y++)
		{
			char buffer[265];
			sprintf_s(buffer, "%1.3f ", colorChannel[x][y]);
			strm << buffer;
		}
		strm << "]" << std::endl;
	}
	return strm;
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cout 
			<< "Error: No File given." << std::endl
			<< "Usage: JPEG-Encoder.exe <ppmFile>" << std::endl
			<< "Press enter to exit ..." << std::endl;
		std::cin.get();
		exit(1);
	}
	ImagePtr img = Image::readPPM(argv[1]);
	std::cout << "ColorChannel for " << img->getColorCoding() << std::endl;
	std::cout << img->getColorChannel(R) << std::endl;
	std::cout << img->getColorChannel(G) << std::endl;
	std::cout << img->getColorChannel(B);

	img->switchColorCoding(YCbCr);
	curCoding = YCbCr;
	std::cout << "==================================================" << std::endl;

	std::cout << "ColorChannel for " << img->getColorCoding() << std::endl;
	std::cout << img->getColorChannel(R) << std::endl;
	std::cout << img->getColorChannel(G) << std::endl;
	std::cout << img->getColorChannel(B) << std::endl;

	std::cout << "Press enter to exit ..." << std::endl;
	std::cin.get();
	exit(0);
}

