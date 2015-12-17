#include "stdafx.h"
#include "Encoder.h"
#include "SIMD.h"

Encoder::Encoder(const Image& image) : Image(image)
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

void Encoder::multiplyColorChannelBy(ColorChannelName channelName, float val)
{
	multiplyAVX(channels->getChannel(channelName), val, blocksPerChannel[channelName] * 8);
}

void Encoder::reduceWidthResolutionColorChannel(ColorChannelName channelName, int factor, ReductionMethod method)
{
	assert(channelSizes[channelName].width % factor == 0);

	// factor 1 => no reduction necessary
	if (factor == 1) return;

	// get channelName data and information
	float* channel = channels->getChannel(channelName);
	size_t channelDataSize = blocksPerChannel[channelName] * 8;

	// adjust the channelName information
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

	// get channelName data and information
	float* channel = channels->getChannel(channelName);
	size_t channelDataSize = blocksPerChannel[channelName] * 8;

	// save old channelName info
	Dimension2D oldchannelSize = channelSizes[channelName];

	// calc new channelName info
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
	reduceWidthResolutionColorChannel(Y, samplingScheme.reductionOptions[Y].widthFactor, samplingScheme.reductionOptions[Y].widthMethod);
	reduceWidthResolutionColorChannel(Cb, samplingScheme.reductionOptions[Cb].widthFactor, samplingScheme.reductionOptions[Cb].widthMethod);
	reduceWidthResolutionColorChannel(Cr, samplingScheme.reductionOptions[Cr].widthFactor, samplingScheme.reductionOptions[Cr].widthMethod);

	reduceHeightResolutionColorChannel(Y, samplingScheme.reductionOptions[Y].heightFactor, samplingScheme.reductionOptions[Y].heightMethod);
	reduceHeightResolutionColorChannel(Cb, samplingScheme.reductionOptions[Cb].heightFactor, samplingScheme.reductionOptions[Cb].heightMethod);
	reduceHeightResolutionColorChannel(Cr, samplingScheme.reductionOptions[Cr].heightFactor, samplingScheme.reductionOptions[Cr].heightMethod);
}

void Encoder::ensurePointerMatrix(const ColorChannelName channelName)
{
	if (blocks[channelName].size() == 0)
	{
		float* const channel = channels->getChannel(channelName);
		const size_t height = channelSizes[channelName].height;
		const size_t width = channelSizes[channelName].width;
		const size_t size = height * width;

		const Dimension2D& factors = samplingScheme.inverseFactor[channelName];

		blocks[channelName].reserve(size / 64);

		const size_t lineOffsets[8]{
			0,
			width,
			width * 2,
			width * 3,
			width * 4,
			width * 5,
			width * 6,
			width * 7
		};

		const size_t blockWidth = 8;
		const size_t blockHeight = 8;

		const size_t mcuHeight = blockHeight * factors.height;
		const size_t mcuWidth = blockWidth * factors.width;

		const size_t mcuRowSize = width * mcuHeight;
		const size_t blockRowSize = width * blockHeight;

		for (size_t mcuYpos = 0; mcuYpos < size; mcuYpos += mcuRowSize)
		{
			for (size_t mcuXpos = 0; mcuXpos < width; mcuXpos += mcuWidth)
			{
				for (size_t mcuRow = 0; mcuRow < mcuRowSize; mcuRow += blockRowSize)
				{
					for (size_t mcuCol = 0; mcuCol < mcuWidth; mcuCol += blockWidth)
					{
						float* position = channel + (mcuYpos + mcuXpos + mcuRow + mcuCol);
						blocks[channelName].emplace_back(
							position + lineOffsets[0],
							position + lineOffsets[1],
							position + lineOffsets[2],
							position + lineOffsets[3],
							position + lineOffsets[4],
							position + lineOffsets[5],
							position + lineOffsets[6],
							position + lineOffsets[7]
						);
					}
				}
			}
		}
	}
}

void Encoder::applyDCT(ColorChannelName channelName)
{
	ensurePointerMatrix(channelName);
	for (PointerMatrix& matrix : blocks[channelName])
	{
		twoDimensionalDCTandQuantisationAVX(matrix, qTables[channelName], matrix);
	}

	auto zigZag = createZigZagOffsetArray(channelSizes[channelName].width);
	calculateDCValues(zigZag, channelName);
	calculateACValues(zigZag, channelName);
}

void Encoder::calculateDCValues(const OffsetArray& zigZag, const ColorChannelName channelName)
{
	bitPatternDC[channelName].reserve(blocks[channelName].size());
	categoriesDC[channelName].reserve(blocks[channelName].size());

	short lastDC = 0;
	for (const PointerMatrix& matrix : blocks[channelName])
	{
		float* dcValuePointer = matrix[0] + zigZag[0];
		short dcValue = static_cast<short>(*dcValuePointer);
		short diff = dcValue - lastDC;
		unsigned short pattern = static_cast<unsigned short>(diff);

		// if value is negative we need to use the inverse of the positive value, see jpeg spec p.17
		if (diff < 0)
		{
			pattern = ~(static_cast<unsigned short>(-diff));
		}

		// calculates category/length of the bitpattern
		byte category = lookupBitCategory(diff);

		// align pattern first bit to the most left bit
		pattern = pattern << (16 - category);

		bitPatternDC[channelName].push_back(BEushort(pattern));
		categoriesDC[channelName].push_back(category);
		lastDC = dcValue;
	}
}

void Encoder::calculateACValues(const OffsetArray& zigZag, const ColorChannelName channelName)
{
	bitPatternAC[channelName].reserve(blocks[channelName].size());
	categoriesAC[channelName].reserve(blocks[channelName].size());


	for (const PointerMatrix& matrix : blocks[channelName])
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
					// 16 zero values are encoded as 0xF0 which is equivalent to the tuple (15,0)
					bitPattern.push_back(0);
					categories.push_back(0xF0); // 15 << 4 | 0
					zeros -= 16;
				}

				unsigned short pattern;
				// if 'acValue' is negative we need to use the inverse of the positive value, see jpeg spec p.17
				if (acValue < 0) {
					pattern = ~(static_cast<unsigned short>(-acValue));
				} else {
					pattern = static_cast<unsigned short>(acValue);
				}

				// calculates category/length of the bitpattern
				byte category = lookupBitCategory(acValue);

				// align pattern first bit to the most left bit
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

		bitPatternAC[channelName].push_back(bitPattern);
		categoriesAC[channelName].push_back(categories);
	}
}

byte Encoder::lookupBitCategory(short value) const
{
	assert(value != SHRT_MIN && value != SHRT_MAX);

	if (value == 0) return 0;
#ifdef GCC
	return 32 - __builtin_clz(static_cast<unsigned int>(abs(value)));
#else
	unsigned long result;
	_BitScanReverse(&result, static_cast<long>(abs(value)));
	return static_cast<byte>(result + 1);
#endif
}

HuffmanTablePtr<byte> Encoder::createHuffmanTable(const CoefficientType type, const ColorChannelName channelName)
{
	// Can't create huffmantable for Cr alone because Cr and Cb are combined in a single table. Use Cb instead. 
	assert(channelName != YCbCrColorName::Cr);

	// If color channelName is cb or cr we have to combine them to one because normally there is only one huffman table for both cb and cr channels
	std::vector<byte> cat;

	if (type == CoefficientType::AC)
	{
		// AC
		cat.reserve(categoriesAC[channelName].size() * 63);
		for (std::vector<byte>& v : categoriesAC[channelName])
		{
			cat.insert(cat.end(), v.begin(), v.end());
		}

		if (channelName == YCbCrColorName::Cb)
		{
			for (std::vector<byte>& v : categoriesAC[YCbCrColorName::Cr])
			{
				cat.insert(cat.end(), v.begin(), v.end());
			}
		}
	}
	else
	{
		// DC
		if (channelName == YCbCrColorName::Cb)
		{
			cat.insert(cat.end(), categoriesDC[channelName].begin(), categoriesDC[channelName].end());
			cat.insert(cat.end(), categoriesDC[YCbCrColorName::Cr].begin(), categoriesDC[YCbCrColorName::Cr].end());
		}
		else
		{
			cat = categoriesDC[channelName];
		}
	}

	return HuffmanTable<byte>::create(16, cat);
}

HuffmanTablePtr<byte> Encoder::getHuffmanTable(CoefficientType type, ColorChannelName channelName)
{
	// Use Cb channelName for both Cr and Cb
	if (channelName == YCbCrColorName::Cr)
	{
		channelName = YCbCrColorName::Cb;
	}

	// Lazily create huffman table
	if (!huffmanTables[channelName][type])
	{
		auto table = createHuffmanTable(type, channelName);
		huffmanTables[channelName][type] = table;
		return table;
	}
	else 
	{
		return huffmanTables[channelName][type];
	}
}

void Encoder::serialize(BitBuffer &bitBuffer)
{
	size_t yBlockSize = (channelSizes[YCbCrColorName::Y].height*channelSizes[YCbCrColorName::Y].width) / 64;
	//size_t cbBlockSize = (channelSizes[YCbCrColorName::Cb].height*channelSizes[YCbCrColorName::Cb].width) / 64;
	//size_t crBlockSize = (channelSizes[YCbCrColorName::Cr].height*channelSizes[YCbCrColorName::Cr].width) / 64;
	
	size_t yfactor = samplingScheme.inverseFactor[Y].height * samplingScheme.inverseFactor[Y].width;
	size_t cbfactor = samplingScheme.inverseFactor[Cb].height * samplingScheme.inverseFactor[Cb].width;
	size_t crfactor = samplingScheme.inverseFactor[Cr].height * samplingScheme.inverseFactor[Cr].width;

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

void Encoder::pushBlock(BitBuffer &bitBuffer, ColorChannelName channelName, size_t block)
{
	ColorChannelName huffmannColorChannel = channelName;
	if (huffmannColorChannel == YCbCrColorName::Cr)
	{
		huffmannColorChannel = YCbCrColorName::Cb;
	}

	byte category = categoriesDC[channelName][block];
	bitBuffer.pushBitsEscaped(*huffmanTables[huffmannColorChannel][CoefficientType::DC]->encode(category));
	bitBuffer.pushBitsEscaped(category, &bitPatternDC[channelName][block]);

	for (int i = 0; i < categoriesAC[channelName][block].size(); i++)
	{
		category = categoriesAC[channelName][block][i];
		bitBuffer.pushBitsEscaped(*huffmanTables[huffmannColorChannel][CoefficientType::AC]->encode(category));
		bitBuffer.pushBitsEscaped(0b1111 & category, &bitPatternAC[channelName][block][i]);
	}
}