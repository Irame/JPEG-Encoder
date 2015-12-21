#include "stdafx.h"
#include "Encoder.h"
#include "SIMD.h"
#include "JPEGSegments.h"

Encoder::Encoder(const Image& image, const QTableSet& qtables)
	: image(std::make_shared<Image>(image)), qTables(qtables)
{}

Encoder::Encoder(ImagePtr imagePtr, const QTableSet& qtables)
	: image(imagePtr), qTables(qtables)
{}

void Encoder::convertToYCbCr()
{
	assert(image->blocksPerChannel[0] == image->blocksPerChannel[1] && image->blocksPerChannel[0] == image->blocksPerChannel[2]);
#pragma omp parallel for
	for (int64_t i = 0; i < static_cast<int64_t>(image->blocksPerChannel[0]) * 8; i += 8)
	{
		convertRGBToYCbCrAVXImpl(image->channels->red(i), image->channels->green(i), image->channels->blue(i));
	}
}

void Encoder::convertToRGB()
{
	assert(image->blocksPerChannel[0] == image->blocksPerChannel[1] && image->blocksPerChannel[0] == image->blocksPerChannel[2]);
#pragma omp parallel for
	for (int64_t i = 0; i < static_cast<int64_t>(image->blocksPerChannel[0]) * 8; i += 8)
	{
		convertYCbCrToRGBAVXImpl(image->channels->red(i), image->channels->green(i), image->channels->blue(i));
	}
}

void Encoder::multiplyColorChannelBy(ColorChannelName channelName, float val)
{
	multiplyAVX(image->channels->getChannel(channelName), val, image->blocksPerChannel[channelName] * 8);
}

void Encoder::reduceWidthResolutionColorChannel(ColorChannelName channelName, int factor, ReductionMethod method)
{
	assert(image->channelSizes[channelName].width % factor == 0);

	// factor 1 => no reduction necessary
	if (factor == 1) return;

	// get channelName data and information
	float* channel = image->channels->getChannel(channelName);
	int64_t channelDataSize = static_cast<int64_t>(image->blocksPerChannel[channelName] * 8);

	// adjust the channelName information
	image->blocksPerChannel[channelName] /= factor;
	image->channelSizes[channelName].width /= factor;

	//TODO: implement AVX code paths

	// use special AVX codepath for higher perfomance if the factor is 2
	if (method == Average && factor == 2)
	{
		for (int64_t srcOffset = 0; srcOffset < channelDataSize; srcOffset += 16)
		{
			halfWidthResolutionAverageAVX(&channel[srcOffset], &channel[srcOffset + 8], &channel[srcOffset / 2]);
		}
	}
	else if (method == Subsampling && factor == 2)
	{
		for (int64_t srcOffset = 0; srcOffset < channelDataSize; srcOffset += 16)
		{
			halfWidthResolutionSubsamplingAVX(&channel[srcOffset], &channel[srcOffset + 8], &channel[srcOffset / 2]);
		}
	}
	// otherwise use a general implementation
	else if (method == Subsampling)
	{
		for (int64_t srcOffset = 0; srcOffset < channelDataSize; srcOffset += factor)
		{
			channel[srcOffset/factor] = channel[srcOffset];
		}
	}
	else if (method == Average)
	{
		float sum = 0.0f;
		for (int64_t srcOffset = 0, dstOffset = 0; srcOffset < channelDataSize; )
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
	assert(image->channelSizes[channelName].height % factor == 0);

	// factor 1 => no reduction necessary
	if (factor == 1) return;

	// get channelName data and information
	float* channel = image->channels->getChannel(channelName);
	int64_t channelDataSize = static_cast<int64_t>(image->blocksPerChannel[channelName] * 8);

	// save old channelName info
	Dimension2D oldchannelSize = image->channelSizes[channelName];

	// calc new channelName info
	//size_t newChannelDataSize = channelDataSize / factor;
	size_t newChannelHeight = oldchannelSize.height / factor;
	image->blocksPerChannel[channelName] /= factor;
	image->channelSizes[channelName].height = newChannelHeight;

	//TODO: implement AVX code paths

	// use special AVX codepath for higher perfomance if the factor is 2 and method is Average
	if (method == Average && factor == 2)
	{
		// Processes the image in columns of width 8 from top to bottom
		for (int64_t x = 0; x < static_cast<int64_t>(oldchannelSize.width); x += 8) {
			for (int64_t srcOffset = x, destOffset = x; srcOffset < channelDataSize; srcOffset += oldchannelSize.width * 2, destOffset += oldchannelSize.width) {
				halfHeightResolutionAverageAVX(&channel[srcOffset], &channel[srcOffset + oldchannelSize.width], &channel[destOffset]);
			}
		}
	}
	// otherwise use a general implementation
	else if (method == Subsampling)
	{
		for(int64_t srcOffset = 0; 
			srcOffset < channelDataSize; 
			srcOffset += oldchannelSize.width * factor)
		{
			memcpy(&channel[srcOffset/factor], &channel[srcOffset], oldchannelSize.width * sizeof(float));
		}
	}
	else if (method == Average)
	{
		float sum = 0.0f;
		size_t valCount = 0;
		for (int64_t x = 0; x < static_cast<int64_t>(oldchannelSize.width); x++) {
			for (int64_t srcOffset = x, destOffset = x; srcOffset < channelDataSize; srcOffset += oldchannelSize.width) {
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
	reduceWidthResolutionColorChannel(Y,  image->samplingScheme.reductionOptions[Y].widthFactor,  image->samplingScheme.reductionOptions[Y].widthMethod);
	reduceWidthResolutionColorChannel(Cb, image->samplingScheme.reductionOptions[Cb].widthFactor, image->samplingScheme.reductionOptions[Cb].widthMethod);
	reduceWidthResolutionColorChannel(Cr, image->samplingScheme.reductionOptions[Cr].widthFactor, image->samplingScheme.reductionOptions[Cr].widthMethod);

	reduceHeightResolutionColorChannel(Y,  image->samplingScheme.reductionOptions[Y].heightFactor,  image->samplingScheme.reductionOptions[Y].heightMethod);
	reduceHeightResolutionColorChannel(Cb, image->samplingScheme.reductionOptions[Cb].heightFactor, image->samplingScheme.reductionOptions[Cb].heightMethod);
	reduceHeightResolutionColorChannel(Cr, image->samplingScheme.reductionOptions[Cr].heightFactor, image->samplingScheme.reductionOptions[Cr].heightMethod);
}

std::vector<PointerMatrix> Encoder::createBlocks(const ColorChannelName channelName)
{
	std::vector<PointerMatrix> result;

	float* const channel = image->channels->getChannel(channelName);
	const size_t height = image->channelSizes[channelName].height;
	const size_t width = image->channelSizes[channelName].width;
	const int64_t size = height * width;

	const Dimension2D& factors = image->samplingScheme.inverseFactor[channelName];

	result.reserve(size / 64);

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

//#pragma omp parallel for
	for (int64_t mcuYpos = 0; mcuYpos < size; mcuYpos += mcuRowSize)
	{
		for (size_t mcuXpos = 0; mcuXpos < width; mcuXpos += mcuWidth)
		{
			for (size_t mcuRow = 0; mcuRow < mcuRowSize; mcuRow += blockRowSize)
			{
				for (size_t mcuCol = 0; mcuCol < mcuWidth; mcuCol += blockWidth)
				{
					float* position = channel + (mcuYpos + mcuXpos + mcuRow + mcuCol);
					result.emplace_back(
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

	return result;
}

void Encoder::applyDCT(ColorChannelName channelName)
{
	std::vector<PointerMatrix> blocks = createBlocks(channelName);
	
#pragma omp parallel for
	for (int64_t i = 0; i < static_cast<int64_t>(blocks.size()); i++)
	{
		twoDimensionalDCTandQuantisationAVX(blocks[i], qTables[channelName], blocks[i]);
	}

	auto zigZag = createZigZagOffsetArray(image->channelSizes[channelName].width);
	calculateDCValues(blocks, zigZag, channelName);
	calculateACValues(blocks, zigZag, channelName);
}

void Encoder::calculateDCValues(const std::vector<PointerMatrix>& blocks, const OffsetArray& zigZag, const ColorChannelName channelName)
{
	bitPatternDC[channelName].reserve(blocks.size());
	categoriesDC[channelName].reserve(blocks.size());

	short lastDC = 0;
	for (size_t i = 0; i < blocks.size(); i++)
	{
		float* dcValuePointer = blocks[i][0] + zigZag[0];
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

void Encoder::calculateACValues(const std::vector<PointerMatrix>& blocks, const OffsetArray& zigZag, const ColorChannelName channelName)
{
	bitPatternAC[channelName].resize(blocks.size());
	categoriesAC[channelName].resize(blocks.size());


#pragma omp parallel for
	for (int64_t blockIdx = 0; blockIdx < static_cast<int64_t>(blocks.size()); blockIdx++)
	{
		std::vector<BEushort> bitPattern;
		std::vector<byte> categories;
		bitPattern.reserve(63);
		categories.reserve(63);

		int zeros = 0;
		for (int i = 1; i < 64; i++)
		{
			float* acValuePointer = blocks[blockIdx][0] + zigZag[i];
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

		bitPatternAC[channelName][blockIdx] = bitPattern;
		categoriesAC[channelName][blockIdx] = categories;
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
		if (channelName == YCbCrColorName::Y)
		{
			cat.reserve(categoriesAC[YCbCrColorName::Y].size() * 63);
			for (std::vector<byte>& v : categoriesAC[YCbCrColorName::Y])
			{
				cat.insert(cat.end(), v.begin(), v.end());
			}
		}
		else
		{
			cat.reserve(categoriesAC[YCbCrColorName::Cr].size() * 63);
			for (std::vector<byte>& v : categoriesAC[YCbCrColorName::Cr])
			{
				cat.insert(cat.end(), v.begin(), v.end());
			}

			for (std::vector<byte>& v : categoriesAC[YCbCrColorName::Cb])
			{
				cat.insert(cat.end(), v.begin(), v.end());
			}
		}
	}
	else
	{
		// DC
		if (channelName == YCbCrColorName::Y)
		{
			cat = categoriesDC[YCbCrColorName::Y];
		}
		else
		{
			cat.insert(cat.end(), categoriesDC[YCbCrColorName::Cb].begin(), categoriesDC[YCbCrColorName::Cb].end());
			cat.insert(cat.end(), categoriesDC[YCbCrColorName::Cr].begin(), categoriesDC[YCbCrColorName::Cr].end());
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
	using namespace JPEGSegments;

	const Dimension2D& imageSize = image->getImageSize();
	const SamplingDefinition& scheme = image->getSamplingScheme();
	const HuffmanTablePtr<byte> huffmannDCY = getHuffmanTable(CoefficientType::DC, YCbCrColorName::Y);
	const HuffmanTablePtr<byte> huffmannDCCb = getHuffmanTable(CoefficientType::DC, YCbCrColorName::Cb);
	const HuffmanTablePtr<byte> huffmannACY = getHuffmanTable(CoefficientType::AC, YCbCrColorName::Y);
	const HuffmanTablePtr<byte> huffmannACCb = getHuffmanTable(CoefficientType::AC, YCbCrColorName::Cb);
	auto acHT = std::array<byte, 3>();
	auto dcHT = std::array<byte, 3>();

	StartOfImage startOfImage;
	APP0 app0;
	StartOfFrame0 startOfFrame0(static_cast<short>(imageSize.width), static_cast<short>(imageSize.height), scheme);
	DefineHuffmannTable defineHuffmannTableDCY(Y, HuffmanTableType::DC, *huffmannDCY);
	DefineHuffmannTable defineHuffmannTableDCCb(Cb, HuffmanTableType::DC, *huffmannDCCb);
	DefineHuffmannTable defineHuffmannTableACY(Y, HuffmanTableType::AC, *huffmannACY);
	DefineHuffmannTable defineHuffmannTableACCb(Cb, HuffmanTableType::AC, *huffmannACCb);
	DefineQuantizationTable defineQuantizationTableLuminance(Y, qTables[Y]);
	DefineQuantizationTable defineQuantizationTableChrominance(Cb, qTables[Cb]);
	DefineQuantizationTable defineQuantizationTableChrominance2(Cr, qTables[Cr]);

	acHT[YCbCrColorName::Y] = YCbCrColorName::Y;
	acHT[YCbCrColorName::Cb] = YCbCrColorName::Cb;
	acHT[YCbCrColorName::Cr] = YCbCrColorName::Cb;
	dcHT[YCbCrColorName::Y] = YCbCrColorName::Y;
	dcHT[YCbCrColorName::Cb] = YCbCrColorName::Cb;
	dcHT[YCbCrColorName::Cr] = YCbCrColorName::Cb;

	StartOfScan startOfScan(acHT, dcHT);
	EndOfImage endOfImage;

	Serialize(startOfImage, bitBuffer);
	Serialize(app0, bitBuffer);
	Serialize(startOfFrame0, bitBuffer);
	Serialize(defineHuffmannTableDCY, bitBuffer);
	Serialize(defineHuffmannTableDCCb, bitBuffer);
	Serialize(defineHuffmannTableACY, bitBuffer);
	Serialize(defineHuffmannTableACCb, bitBuffer);
	Serialize(defineQuantizationTableLuminance, bitBuffer);
	Serialize(defineQuantizationTableChrominance, bitBuffer);
	Serialize(defineQuantizationTableChrominance2, bitBuffer);
	Serialize(startOfScan, bitBuffer);

	serializeScanData(bitBuffer);

	Serialize(endOfImage, bitBuffer);
}

void Encoder::serializeScanData(BitBuffer & bitBuffer)
{
	size_t yBlockSize = (image->channelSizes[YCbCrColorName::Y].height*image->channelSizes[YCbCrColorName::Y].width) / 64;
	//size_t cbBlockSize = (channelSizes[YCbCrColorName::Cb].height*channelSizes[YCbCrColorName::Cb].width) / 64;
	//size_t crBlockSize = (channelSizes[YCbCrColorName::Cr].height*channelSizes[YCbCrColorName::Cr].width) / 64;

	size_t yfactor = image->samplingScheme.inverseFactor[Y].height  * image->samplingScheme.inverseFactor[Y].width;
	size_t cbfactor = image->samplingScheme.inverseFactor[Cb].height * image->samplingScheme.inverseFactor[Cb].width;
	size_t crfactor = image->samplingScheme.inverseFactor[Cr].height * image->samplingScheme.inverseFactor[Cr].width;

	for (int i = 0; i < yBlockSize / yfactor; i++)
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