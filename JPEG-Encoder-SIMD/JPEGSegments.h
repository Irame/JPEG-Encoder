#pragma once

// fixing alignment issues by setting the byte alignment to 1
#pragma pack(push, 1)

#include <algorithm>

#include "HuffmanCoding.h"
#include "HuffmanCodingByte.h"

#include "SamplingScheme.h"
#include "QuantizationTables.h"
#include "ZigZag.h"
#include "ColorNames.h"




namespace JPEGSegments
{
	enum class HuffmanTableType : byte
	{
		DC = 0, AC = 1
	};

	enum class SegmentType : byte {
		StartOfImage = 0xD8, // SOI
		EndOfImage = 0xD9, // EOI
		APP0 = 0xE0,
		StartOfFrame0 = 0xC0, // SOF0
		DefineHuffmannTable = 0xC4, // DHT
		DefineQuantizationTable = 0xDB, // DQT
		StartOfScan = 0xDA, // SOS
	};

	//enum class QuantizationTableType : byte {
	//	Luminance = 0,
	//	Chrominance = 1
	//};

	struct HeaderSegmentMarker {
		const byte headerBegin = 0xff;
		const SegmentType headerType;

		HeaderSegmentMarker(SegmentType headerType) : headerType(headerType) {}
	};

	// Start of JPEG segments definition

	struct StartOfImage {
		const HeaderSegmentMarker marker;

		StartOfImage() : marker(SegmentType::StartOfImage) {}
	};

	struct EndOfImage {
		const HeaderSegmentMarker marker;

		EndOfImage() : marker(SegmentType::EndOfImage) {}
	};

	struct APP0 {
		const HeaderSegmentMarker marker;
		const BEushort length = 0x10; // has to be >= 16
		const byte jfif0[5]{ 0x4a, 0x46, 0x49, 0x46, 0x00 };
		const byte majorVersion = 0x01;
		const byte minorVersion = 0x01;
		const byte pixelSize = 0x00;
		const BEushort xDensity = 0x0048;
		const BEushort yDensity = 0x0048;
		const byte xResolutionPreview = 0x00;
		const byte yResolutionPreview = 0x00;

		APP0() : marker(SegmentType::APP0) {}
	};

	struct StartOfFrame0 { // SOF0
		const HeaderSegmentMarker marker;
		const BEushort length = 0x11; // 8 + Anzahl Komponenten*3
		const byte accuracy = 0x08;
		const BEushort yResolution; // has to be > 0
		const BEushort xResolution; // has to be > 0
		const byte componentCount = 0x03;
		byte Y[3]{ 0x01 }; //id, subsampling factor (0-3 bit vertical, 4-7 bit horizontal), number quantisationtable
		byte Cb[3]{ 0x02 };
		byte Cr[3]{ 0x03 };

		StartOfFrame0() : marker(SegmentType::StartOfFrame0) {}
		StartOfFrame0(BEushort xResolution, BEushort yResolution, const SamplingDefinition& scheme) : 
			marker(SegmentType::StartOfFrame0), 
			yResolution(yResolution), 
			xResolution(xResolution) {
			Y[1] = static_cast<byte>((scheme.inverseFactor[YCbCrColorName::Y].width << 4) | (0b1111 & scheme.inverseFactor[YCbCrColorName::Y].height));
			Y[2] = static_cast<byte>(YCbCrColorName::Y);
			Cb[1] = static_cast<byte>((scheme.inverseFactor[YCbCrColorName::Cb].width << 4) | (0b1111 & scheme.inverseFactor[YCbCrColorName::Cb].height));
			Cb[2] = static_cast<byte>(YCbCrColorName::Cb);
			Cr[1] = static_cast<byte>((scheme.inverseFactor[YCbCrColorName::Cr].width << 4) | (0b1111 & scheme.inverseFactor[YCbCrColorName::Cr].height));
			Cr[2] = static_cast<byte>(YCbCrColorName::Cr);
		}
	};

	struct DefineHuffmannTable {
		const HeaderSegmentMarker marker;
		BEushort length;
		byte htInformation; //0-3 bit number of ht, bit 4 (0 = DC, 1 = AC), 5-7 have to be 0
		byte symbolCount[16];
		byte* table;

		DefineHuffmannTable(byte htNum, HuffmanTableType htType, const HuffmanTable<byte>& huffmanTable)
			: marker(SegmentType::DefineHuffmannTable),
			length(2 + 1 + 16 + static_cast<unsigned short>(huffmanTable.getSymbolCount())),
			htInformation((0b1111 & htNum) | (static_cast<byte>(htType) << 4)),
			table(new byte[huffmanTable.getSymbolCount()])
		{
			memset(symbolCount, 0, 16);

			std::vector<HuffmanTable<byte>::SymbolCodePair> sortableMapEntries;

			for (const auto& symbolCodePair : huffmanTable)
			{
				sortableMapEntries.push_back(symbolCodePair);
			}

			std::sort(sortableMapEntries.begin(), sortableMapEntries.end(), [](const HuffmanTable<byte>::SymbolCodePair a, const HuffmanTable<byte>::SymbolCodePair b)
			{
				if (a.code->getSize() == b.code->getSize())
					return *a.code < *b.code;
				return a.code->getSize() < b.code->getSize();
			});

			assert(huffmanTable.getSymbolCount() > 0);
			assert(huffmanTable.getSymbolCount() == sortableMapEntries.size());

			int i = 0;
			for (const auto symbolCodePair : sortableMapEntries)
			{
				symbolCount[symbolCodePair.code->getSize() - 1] += 1;
				table[i++] = symbolCodePair.symbol;
			}
		}

		~DefineHuffmannTable()
		{
			delete[] table;
		}

		// Eine DHT kann mehrere HTs enthalten, jeweils mit eigenem Informationsbyte
		// Maximale Tiefe des Huffman - Baums ist auf 16 beschränkt
		// Ein Huffman - Code, der nur aus ‚1‘ Bits besteht ist nicht erlaubt
	};

	struct DefineQuantizationTable {
		const HeaderSegmentMarker marker;
		BEushort length = 2 + 1 + 64; // precision is always 8 Bit so the length is already known
		byte info; // 0-3 bits number of QT (0-3), 4-7 accuracy of QT (0 = 8 bit, otherwise 16 bit)
		byte coefficients[64]; // count = 64* (precision+1), zigzag sorted

		DefineQuantizationTable(ColorChannelName colorChannel, const QTable& qTable)
			: marker(SegmentType::DefineQuantizationTable),
			info(static_cast<byte>(colorChannel))
		{
			auto zigZagQTable = reorderByZigZag(qTable.floats);
			for (int i = 0; i < 64; i++) 
			{
				coefficients[i] = static_cast<byte>(zigZagQTable[i]);
			}
		}

		// Ein DQT-Segment kann mehrere QT enthalten
		// Verwenden Sie nur 8 Bit QTs
		// QT für Farbkanal Y ist z.B.Nummer 0
		// QT für Farbkanal Cb / Cr ist z.B.Nummer 1
	};

	struct StartOfScan {
		const HeaderSegmentMarker marker;
		const BEushort length = 0x0c;
		const byte components = 0x03;
		byte Y[2]{ 0x01 }; // second byte = used HT (0-3 bit = AC HT, 4-7 = DC HT)
		byte Cb[2]{ 0x02 };
		byte Cr[2]{ 0x03 };
		const byte irrelevant[3]{ 0x00, 0x3f, 0x00 };

		StartOfScan(std::array<byte, 3> acHT, std::array<byte, 3> dcHT) 
			: marker(SegmentType::StartOfScan)
		{
			Y[1] = ((0b1111 & acHT[YCbCrColorName::Y]) << 4) | (0b1111 & dcHT[YCbCrColorName::Y]);
			Cb[1] = ((0b1111 & acHT[YCbCrColorName::Cb]) << 4) | (0b1111 & dcHT[YCbCrColorName::Cb]);
			Cr[1] = ((0b1111 & acHT[YCbCrColorName::Cr]) << 4) | (0b1111 & dcHT[YCbCrColorName::Cr]);
		}

		// Kommt innerhalb eines Segmentes irgendwann das Byte 0xff vor, muss die Bytefolge 0xff 0x00 ausgegeben werden
		// Dadurch ist ein Decoder später in der Lage, beschädigte Dateien zu dekodieren und sich auf Segmentgrenzen zu synchronisieren
		// => Es gibt keinen Segmentmarker 0xff 0x00
	};

	// JPEGSegments serialize functions
	template <typename T>
	static void Serialize(T &headerSegment, BitBuffer &buffer) {
		buffer.push(headerSegment);
	}

	template <>
	inline void Serialize(DefineHuffmannTable &headerSegment, BitBuffer &buffer) {
		buffer.pushBits(21 * 8, &headerSegment);
		buffer.pushBits((headerSegment.length - 19) * 8, headerSegment.table); //21-2 byte because the marker doesn't count
	}
};

#pragma pack(pop) // use old pack value
