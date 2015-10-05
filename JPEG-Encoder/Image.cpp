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
		None, Size, Pixels
	};
	State state = State::None;
	std::ifstream fileStream = std::ifstream(path);
	string line;
	float maxValue;
	int currentWidth = 0;
	int currentHeigth = 0;
	int j = 0;

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
			data = new Pixel*[width];
			for (int i = 0; i < width; i++) {
				data[i] = new Pixel[heigth];
			}

			state = Size;
		} else if (state == State::Size) {
			maxValue = stoi(line);
			state = State::Pixels;
		} else if (state == State::Pixels) {
			std::stringstream stringStream(line);
			string color;
			while (getline(stringStream, color, ' ')) {
				if (color.length() == 0)
					continue;

				if(j == 0)
					data[currentWidth][currentHeigth].setColorValue(Pixel::R, float(stoi(color) / maxValue));
				else if(j == 1)
					data[currentWidth][currentHeigth].setColorValue(Pixel::G, float(stoi(color) / maxValue));
				else if(j == 2)
					data[currentWidth][currentHeigth].setColorValue(Pixel::G, float(stoi(color) / maxValue));

				j++;
				if (j > 2) {
					j = 0;
					currentWidth++;
					if (currentWidth >= width) {
						currentWidth = 0;
						currentHeigth++;
					}
				}
			}
		}
	}


}
