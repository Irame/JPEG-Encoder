#include "stdafx.h"
#include "Image.h"
#include <io.h>
#include <fstream>
#include <sstream>
using namespace std;

Image::Image()
{
}


Image::~Image()
{
}

void Image::getData(Pixel * data[])
{
	data = this->data;
}

ColorCoding Image::getColorCoding()
{
	return colorCoding;
}

void Image::setStep(unsigned int stepX, unsigned int stepY)
{
	this->stepX = stepX;
	this->stepY = stepY;
}

void Image::readPPM(std::string path)
{
	enum State {
		None, Size, Pixel
	};
	State state = State::None;
	std::ifstream fileStream = std::ifstream(path);
	string line;
	int maxValue;
	while (getline(fileStream, line)) {
		if (line == "P3")
			continue;
		if (line.front() == '#')
			continue;
		if (state == State::None) {
			std::stringstream stringStream(line);
			string size;
			for (int i = 0; getline(stringStream, size, ' '); i++) {
				if (i == 0)
					width = stoi(size);
				else {
					heigth = stoi(size);
					//break;
				}
			}
			state = Size;
		}
		if (state == State::Size) {
			maxValue = stoi(line);
			state = State::Pixel;
		}
		if (state == State::Pixel) {

		}
	}


}
