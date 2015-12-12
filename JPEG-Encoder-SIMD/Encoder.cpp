#include "stdafx.h"
#include "Encoder.h"
#include "SIMD.h"

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
	bitPattern[colorChannelName][CoefficientType::DC].reserve(blocks[colorChannelName].size());
	categories[colorChannelName][CoefficientType::DC].reserve(blocks[colorChannelName].size());

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
			category = static_cast<byte>(log2f(diff) + 1);
		}

		//align pattern first bit to the most left bit
		pattern = pattern << (16 - category);

		bitPattern[colorChannelName][CoefficientType::DC].push_back(BEushort(pattern));
		categories[colorChannelName][CoefficientType::DC].push_back(category);
		lastDC = dcValue;
	}
}

void Encoder::calculateACValues(const OffsetArray& zigZag, const ColorChannelName colorChannelName)
{
	bitPattern[colorChannelName][CoefficientType::AC].reserve(blocks[colorChannelName].size() * 63);
	categories[colorChannelName][CoefficientType::AC].reserve(blocks[colorChannelName].size() * 63);

	for (const PointerMatrix& matrix : blocks[colorChannelName])
	{
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
					bitPattern[colorChannelName][CoefficientType::AC].push_back(0);
					categories[colorChannelName][CoefficientType::AC].push_back(0xF0);//15 << 4 | 0
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
				byte category = static_cast<byte>(log2f(acValue) + 1);

				//align pattern first bit to the most left bit
				pattern <<= (16 - category);

				bitPattern[colorChannelName][CoefficientType::AC].push_back(BEushort(pattern));
				categories[colorChannelName][CoefficientType::AC].push_back(zeros << 4 | category);
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
			bitPattern[colorChannelName][CoefficientType::AC].push_back(0);
			categories[colorChannelName][CoefficientType::AC].push_back(0);
			zeros = 0;
		}
	}
}

void Encoder::createHuffmanTable(const CoefficientType type, const ColorChannelName colorChannelName)
{
	if (colorChannelName == YCbCrColorName::Cb || colorChannelName == YCbCrColorName::Cr)
	{
		if (huffmanTables[YCbCrColorName::Cb][type] != nullptr)
		{
			return;
		}
		// if color channel is cb or cr we have to combine them to one because normally there is only one huffman table for both cb and cr channels
		std::vector<byte> cat = std::vector<byte>(categories[YCbCrColorName::Cb][type]);
		cat.insert(cat.end(), categories[YCbCrColorName::Cr][type].begin(), categories[YCbCrColorName::Cr][type].end());
		huffmanTables[YCbCrColorName::Cb][type] = HuffmanTable<byte>::create(16, cat);
	}
	else
	{
		if (huffmanTables[colorChannelName][type] != nullptr)
		{
			return;
		}
		huffmanTables[colorChannelName][type] = HuffmanTable<byte>::create(16, categories[colorChannelName][type]);
	}
}

HuffmanTablePtr<byte> Encoder::getHuffmanTable(CoefficientType type, ColorChannelName colorChannelName)
{
	//createHuffmanTable(type, colorChannelName);
	//if (colorChannelName == YCbCrColorName::Cb || colorChannelName == YCbCrColorName::Cr)
	//{
	//	return huffmanTables[YCbCrColorName::Cb][type];
	//}
	//else
	//{
	//	return huffmanTables[colorChannelName][type];
	//}

	std::vector<byte> allSymbols{ 0, 1, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6 };
	return HuffmanTable<byte>::create(16, allSymbols);
}