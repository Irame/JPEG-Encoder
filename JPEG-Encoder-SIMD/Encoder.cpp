#include "stdafx.h"
#include "Encoder.h"
#include "SIMD.h"
#include <algorithm>

Encoder::Encoder(Image image) : Image(image)
{}

void Encoder::convertToYCbCr()
{
	//#pragma omp parallel for
	assert(blocksPerChannel[0] == blocksPerChannel[1] && blocksPerChannel[0] == blocksPerChannel[2]);
	for (size_t i = 0; i < blocksPerChannel[0] * 8; i += 8)
	{
		convertRGBToYCbCrAVXImpl(channels->red(i), channels->green(i), channels->blue(i));
	}
}

void Encoder::convertToRGB()
{
	//#pragma omp parallel for
	assert(blocksPerChannel[0] == blocksPerChannel[1] && blocksPerChannel[0] == blocksPerChannel[2]);
	for (size_t i = 0; i < blocksPerChannel[0] * 8; i += 8)
	{
		convertYCbCrToRGBAVXImpl(channels->red(i), channels->green(i), channels->blue(i));
	}
}

void Encoder::multiplyColorChannelBy(ColorChannelName colorChannel, float val)
{
	multiplyAVX(channels->getChannel(colorChannel), val, blocksPerChannel[colorChannel] * 8);
}

void Encoder::reduceWidthResolutionColorChannel(ColorChannelName channelName, int factor, ReductionMethod method)
{
	assert(channelSizes[channelName].width % factor == 0);

	// factor 1 => no reduction necessary
	if (factor == 1) return;

	// get channel data and information
	float* channel = channels->getChannel(channelName);
	size_t channelDataSize = blocksPerChannel[channelName] * 8;

	// adjust the channel information
	blocksPerChannel[channelName] /= factor;
	channelSizes[channelName].width /= factor;

	//TODO: implement AVX code paths

	// use special AVX codepath for higher perfomance if the factor is 2
	if (method == Average && factor == 2)
	{
		for (size_t srcOffset = 0, dstOffset = 0; srcOffset < channelDataSize; )
		{
			halfWidthResolutionAverageAVX(&channel[srcOffset], &channel[srcOffset + 8], &channel[dstOffset]);
			srcOffset += 16;
			dstOffset += 8;
		}
	}
	else if (method == Subsampling && factor == 2)
	{
		for (size_t srcOffset = 0, dstOffset = 0; srcOffset < channelDataSize; )
		{
			halfWidthResolutionSubsamplingAVX(&channel[srcOffset], &channel[srcOffset + 8], &channel[dstOffset]);
			srcOffset += 16;
			dstOffset += 8;
		}
	}
	// otherwise use a general implementation
	else if (method == Subsampling)
	{
		for (size_t srcOffset = 0, dstOffset = 0; srcOffset < channelDataSize; srcOffset += factor)
		{
			channel[dstOffset++] = channel[srcOffset];
		}
	}
	else if (method == Average)
	{
		float sum = 0.0f;
		for (size_t srcOffset = 0, dstOffset = 0; srcOffset < channelDataSize; )
		{
			sum += channel[srcOffset++];
			if (srcOffset % factor == 0) {
				channel[dstOffset++] = sum / factor;
				sum = 0.0f;
			}
		}
	}
}


void Encoder::reduceHeightResolutionColorChannel(ColorChannelName channelName, int factor, ReductionMethod method)
{
	assert(channelSizes[channelName].height % factor == 0);

	// factor 1 => no reduction necessary
	if (factor == 1) return;

	// get channel data and information
	float* channel = channels->getChannel(channelName);
	size_t channelDataSize = blocksPerChannel[channelName] * 8;

	// save old channel info
	Dimension2D oldchannelSize = channelSizes[channelName];

	// calc new channel info
	//size_t newChannelDataSize = channelDataSize / factor;
	size_t newChannelHeight = oldchannelSize.height / factor;
	blocksPerChannel[channelName] /= factor;
	channelSizes[channelName].height = newChannelHeight;

	//TODO: implement AVX code paths

	// use special AVX codepath for higher perfomance if the factor is 2 and method is Average
	if (method == Average && factor == 2)
	{
		// Processes the image in columns of width 8 from top to bottom
		for (size_t x = 0; x < oldchannelSize.width; x += 8) {
			for (size_t srcOffset = x, destOffset = x; srcOffset < channelDataSize; srcOffset += oldchannelSize.width * 2, destOffset += oldchannelSize.width) {
				halfHeightResolutionAverageAVX(&channel[srcOffset], &channel[srcOffset + oldchannelSize.width], &channel[destOffset]);
			}
		}
	}
	// otherwise use a general implementation
	else if (method == Subsampling)
	{
		size_t srcOffset = 0, dstOffset = 0;
		while (srcOffset < channelDataSize)
		{
			memcpy(&channel[dstOffset], &channel[srcOffset], oldchannelSize.width * sizeof(float));

			srcOffset += oldchannelSize.width * factor;
			dstOffset += oldchannelSize.width;
		}
	}
	else if (method == Average)
	{
		float sum = 0.0f;
		size_t valCount = 0;
		for (size_t x = 0; x < oldchannelSize.width; x++) {
			for (size_t srcOffset = x, destOffset = x; srcOffset < channelDataSize; srcOffset += oldchannelSize.width) {
				sum += channel[srcOffset];
				valCount++;
				if (valCount == factor)
				{
					channel[destOffset] = sum / factor;
					destOffset += oldchannelSize.width;
					sum = 0.0f;
					valCount = 0;
				}
			}
		}
	}
}

void Encoder::reduceResolutionBySchema()
{
	reduceWidthResolutionColorChannel(Y, samplingScheme.yReductionOptions.widthFactor, samplingScheme.yReductionOptions.widthMethod);
	reduceWidthResolutionColorChannel(Cb, samplingScheme.cbReductionOptions.widthFactor, samplingScheme.cbReductionOptions.widthMethod);
	reduceWidthResolutionColorChannel(Cr, samplingScheme.crReductionOptions.widthFactor, samplingScheme.crReductionOptions.widthMethod);

	reduceHeightResolutionColorChannel(Y, samplingScheme.yReductionOptions.heightFactor, samplingScheme.yReductionOptions.heightMethod);
	reduceHeightResolutionColorChannel(Cb, samplingScheme.cbReductionOptions.heightFactor, samplingScheme.cbReductionOptions.heightMethod);
	reduceHeightResolutionColorChannel(Cr, samplingScheme.crReductionOptions.heightFactor, samplingScheme.crReductionOptions.heightMethod);
}

void Encoder::ensurePointerMatrix(const ColorChannelName colorChannelName)
{
	if (blocks[colorChannelName].size() == 0)
	{
		float* channel = channels->getChannel(colorChannelName);
		size_t height = channelSizes[colorChannelName].height;
		size_t width = channelSizes[colorChannelName].width;

		blocks[colorChannelName].reserve(height * width / 8 * 8);

		size_t line[8];
		line[0] = 0;
		line[1] = width;
		line[2] = width * 2;
		line[3] = width * 3;
		line[4] = width * 4;
		line[5] = width * 5;
		line[6] = width * 6;
		line[7] = width * 7;

		for (size_t y = 0; y < height; y += 8)
		{
			size_t heightY = height*y;
			for (size_t x = 0; x < width; x += 8)
			{
				float* position = channel + x + heightY;
				blocks[colorChannelName].emplace_back(
					position + line[0],
					position + line[1],
					position + line[2],
					position + line[3],
					position + line[4],
					position + line[5],
					position + line[6],
					position + line[7]
				);
			}
		}
	}
}

void Encoder::applyDCT(ColorChannelName colorChannelName)
{
	ensurePointerMatrix(colorChannelName);
	for (PointerMatrix& matrix : blocks[colorChannelName])
	{
		twoDimensionalDCTandQuantisationAVX(matrix, qTables[colorChannelName], matrix);
	}

	auto zigZag = createZigZagOffsetArray(channelSizes[colorChannelName].width);
	calculateDCValues(zigZag, colorChannelName);
	calculateACValues(zigZag, colorChannelName);
}

void Encoder::calculateDCValues(const OffsetArray& zigZag, const ColorChannelName colorChannelName)
{
	bitPatternDC[colorChannelName].reserve(blocks[colorChannelName].size());
	categoriesDC[colorChannelName].reserve(blocks[colorChannelName].size());

	short lastDC = 0;
	for (const PointerMatrix& matrix : blocks[colorChannelName])
	{
		float* dcValuePointer = matrix[0] + zigZag[0];
		short dcValue = static_cast<short>(*dcValuePointer);
		short diff = dcValue - lastDC;
		unsigned short pattern = static_cast<unsigned short>(diff);

		//if value is negative we need to use the inverse of the positive value, see jpeg spec p.17
		if (diff < 0)
		{
			pattern = ~(static_cast<unsigned short>(-diff));
		}

		//calculates category/length of the bitpattern
		byte category;
		if (diff == 0)
		{
			category = 0;
		}
		else
		{
			category = static_cast<byte>(log2f(abs(diff)) + 1);
		}

		//align pattern first bit to the most left bit
		pattern = pattern << (16 - category);

		bitPatternDC[colorChannelName].push_back(BEushort(pattern));
		categoriesDC[colorChannelName].push_back(category);
		lastDC = dcValue;
	}
}

void Encoder::calculateACValues(const OffsetArray& zigZag, const ColorChannelName colorChannelName)
{
	bitPatternAC[colorChannelName].reserve(blocks[colorChannelName].size());
	categoriesAC[colorChannelName].reserve(blocks[colorChannelName].size());


	for (const PointerMatrix& matrix : blocks[colorChannelName])
	{
		std::vector<BEushort> bitPattern;
		std::vector<byte> categories;
		bitPattern.reserve(63);
		categories.reserve(63);

		int zeros = 0;
		for (int i = 1; i < 64; i++)
		{
			float* acValuePointer = matrix[0] + zigZag[i];
			short acValue = static_cast<short>(*acValuePointer);

			if (acValue != 0)
			{
				while (zeros > 15)
				{
					//16 null values are encoded as 0xF0 which is equivalent to the tuple (15,0)
					bitPattern.push_back(0);
					categories.push_back(0xF0);//15 << 4 | 0
					zeros -= 16;
				}

				unsigned short pattern;
				//if 'acValue' is negative we need to use the inverse of the positive value, see jpeg spec p.17
				if (acValue < 0) {
					pattern = ~(static_cast<unsigned short>(-acValue));
				} else {
					pattern = static_cast<unsigned short>(acValue);
				}

				//calculates category/length of the bitpattern
				byte category = static_cast<byte>(log2f(abs(acValue)) + 1);

				//align pattern first bit to the most left bit
				pattern <<= (16 - category);

				bitPattern.push_back(BEushort(pattern));
				categories.push_back(zeros << 4 | category);
				zeros = 0;
			}
			else
			{
				++zeros;
			}
		}
		// when there are still zeros left to the end of the block it is encoded as a EOB (End of Block) as Tuple (0,0)
		if (zeros != 0)
		{
			bitPattern.push_back(0);
			categories.push_back(0);
			zeros = 0;
		}
		bitPatternAC[colorChannelName].push_back(bitPattern);
		categoriesAC[colorChannelName].push_back(categories);
	}
}

void Encoder::createHuffmanTable(const CoefficientType type, const ColorChannelName colorChannelName)
{
	ColorChannelName channel = colorChannelName;
	if (channel == YCbCrColorName::Cr)
	{
		channel = YCbCrColorName::Cb;
	}
	if (huffmanTables[channel][type] != nullptr)
	{
		return;
	}
	// if color channel is cb or cr we have to combine them to one because normally there is only one huffman table for both cb and cr channels
	std::vector<byte> cat;
	if (type == CoefficientType::AC)
	{
		cat.reserve(categoriesAC[channel].size() * 63);
		for (std::vector<byte>& v : categoriesAC[channel])
		{
			cat.insert(cat.end(), v.begin(), v.end());
		}
		if (channel == YCbCrColorName::Cb)
		{
			for (std::vector<byte>& v : categoriesAC[YCbCrColorName::Cr])
			{
				cat.insert(cat.end(), v.begin(), v.end());
			}
		}
		huffmanTables[channel][type] = HuffmanTable<byte>::create(16, cat);
	}
	else
	{
		if (channel == YCbCrColorName::Cb)
		{
			cat.insert(cat.end(), categoriesDC[channel].begin(), categoriesDC[channel].end());
			cat.insert(cat.end(), categoriesDC[YCbCrColorName::Cr].begin(), categoriesDC[YCbCrColorName::Cr].end());
		}
		else
		{
			cat = categoriesDC[channel];
		}
		huffmanTables[channel][type] = HuffmanTable<byte>::create(16, cat);
	}
}

HuffmanTablePtr<byte> Encoder::getHuffmanTable(CoefficientType type, ColorChannelName colorChannelName)
{
	createHuffmanTable(type, colorChannelName);
	if (colorChannelName == YCbCrColorName::Cr)
	{
		colorChannelName = YCbCrColorName::Cb;
	}
	return huffmanTables[colorChannelName][type];
}

void Encoder::serialize(BitBuffer &bitBuffer)
{
	size_t yBlockSize = (channelSizes[YCbCrColorName::Y].height*channelSizes[YCbCrColorName::Y].width) / 64;
	//size_t cbBlockSize = (channelSizes[YCbCrColorName::Cb].height*channelSizes[YCbCrColorName::Cb].width) / 64;
	//size_t crBlockSize = (channelSizes[YCbCrColorName::Cr].height*channelSizes[YCbCrColorName::Cr].width) / 64;
	
	size_t yfactor = samplingScheme.yReductionOptions.heightFactor * samplingScheme.yReductionOptions.widthFactor;
	size_t cbfactor = samplingScheme.cbReductionOptions.heightFactor * samplingScheme.cbReductionOptions.widthFactor;
	size_t crfactor = samplingScheme.crReductionOptions.heightFactor * samplingScheme.crReductionOptions.widthFactor;

	size_t maxValue = std::max(yfactor, std::max(cbfactor, crfactor));
	
	yfactor = (1.0 / yfactor)*maxValue;
	cbfactor = (1.0 / cbfactor)*maxValue;
	crfactor = (1.0 / crfactor)*maxValue;
	for (int i = 0; i < yBlockSize/yfactor; i++)
	{
		for (int count = 0; count < yfactor; count++)
		{
			pushBlock(bitBuffer, YCbCrColorName::Y, i*yfactor + count);
		}
		for (int count = 0; count < cbfactor; count++)
		{
			pushBlock(bitBuffer, YCbCrColorName::Cb, i*cbfactor + count);
		}
		for (int count = 0; count < crfactor; count++)
		{
			pushBlock(bitBuffer, YCbCrColorName::Cr, i*crfactor + count);
		}
	}
	bitBuffer.fillToByteBorder();
}

void Encoder::pushBlock(BitBuffer &bitBuffer, ColorChannelName colorChannelName, size_t block)
{
	ColorChannelName huffmannColorChannel = colorChannelName;
	if (huffmannColorChannel == YCbCrColorName::Cr)
	{
		huffmannColorChannel = YCbCrColorName::Cb;
	}

	byte category = categoriesDC[colorChannelName][block];
	bitBuffer.pushBitsEscaped(*huffmanTables[huffmannColorChannel][CoefficientType::DC]->encode(category));
	bitBuffer.pushBitsEscaped(category, &bitPatternDC[colorChannelName][block]);

	for (int i = 0; i < categoriesAC[colorChannelName][block].size(); i++)
	{
		category = categoriesAC[colorChannelName][block][i];
		bitBuffer.pushBitsEscaped(*huffmanTables[huffmannColorChannel][CoefficientType::AC]->encode(category));
		bitBuffer.pushBitsEscaped(0b1111 & category, &bitPatternAC[colorChannelName][block][i]);
	}
}