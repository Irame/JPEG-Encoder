#pragma once
#include "stdafx.h"

enum SegmentType {
	StartOfImage = 0xd8, // SOI
	EndOfImage = 0xd9, // EOI
	APP0 = 0xe0,
	StartOfFrame0 = 0xc0, // SOF0
	DefineHuffmannTable = 0xc4, // DHT
	DefineQuantizationTable = 0xdb, // DQT
	StartOfScan = 0xda, // SOS
};

struct HeaderBase {
	void Serialize() {
	}
};

struct HeaderSegmentMarker {
	const byte headerBegin = 0xff;
	const byte headerType;

	HeaderSegmentMarker(SegmentType headerType): headerType (headerType) {}
};

struct StartOfImage : HeaderBase {
	const HeaderSegmentMarker marker;

	StartOfImage() : marker(SegmentType::StartOfImage) {}
};

struct EndOfImage : HeaderBase {
	const HeaderSegmentMarker marker;

	EndOfImage() : marker(SegmentType::EndOfImage) {}
};

struct APP0 : HeaderBase {
	const HeaderSegmentMarker marker;
	const short length = 0x10; // has to be >= 16
	const byte jfif0[5] { 0x4a, 0x46, 0x49, 0x46, 0x00 };
	const byte majorVersion = 0x01;
	const byte minorVersion = 0x01;
	const byte pixelSize = 0x00;
	const short xDensity = 0x0048;
	const short yDensity = 0x0048;
	const byte xResolutionPreview = 0x00;
	const byte yResolutionPreview = 0x00;

	APP0() : marker(SegmentType::APP0) {}
};

struct StartOfFrame0 : HeaderBase { // SOF0
	const HeaderSegmentMarker marker;
	const short length = 0x11; // 8 + Anzahl Komponenten*3
	const byte accuracy = 0x08;
	short yResolution; // has to be > 0
	short xResolution; // has to be > 0
	const byte componentCount = 0x03;
	byte Y[3]{ 0x01 }; //id, subsampling factor (0-3 bit vertical, 4-7 bit horizontal), number quantisationtable
	byte Cb[3]{ 0x02 };
	byte Cr[3]{ 0x03 };
	
	StartOfFrame0() : marker(SegmentType::StartOfFrame0){}
};

struct DefineHuffmannTable : HeaderBase {
	const HeaderSegmentMarker marker;
	short length;
	byte htInformation; //0-3 bit number of ht, bit 4 (0 = DC, 1 = AC), 5-7 have to be 0
	byte symbolCount[16];
	byte* table;
	
	DefineHuffmannTable() : marker(SegmentType::DefineHuffmannTable){}

	// Eine DHT kann mehrere HTs enthalten, jeweils mit eigenem Informationsbyte
	// Maximale Tiefe des Huffman - Baums ist auf 16 beschr�nkt
	// Ein Huffman - Code, der nur aus �1� Bits besteht ist nicht erlaubt
};

struct DefineQuantizationTable : HeaderBase {
	const HeaderSegmentMarker marker;
	short length;
	byte info; // 0-3 bits number of QT (0-3), 4-7 accuracy of QT (0 = 8 bit, otherwise 16 bit)
	byte* coefficients; // count = 64* (precision+1), zigzag sorted
	
	DefineQuantizationTable() : marker(SegmentType::DefineQuantizationTable) {}
	
	// Ein DQT-Segment kann mehrere QT enthalten
    // Verwenden Sie nur 8 Bit QTs
    // QT f�r Farbkanal Y ist z.B.Nummer 0
    // QT f�r Farbkanal Cb / Cr ist z.B.Nummer 1
};

struct StartOfScan : HeaderBase {
	const HeaderSegmentMarker marker;
	const short length = 0x0c;
	const byte components = 0x03;
	byte Y[2]{ 0x01 }; // second byte = used HT (0-3 bit = AC HT, 4-7 = DC HT)
	byte Cb[2]{ 0x02 };
	byte Cr[2]{ 0x03 };
	const byte irrelevant[3]{ 0x00, 0x3f, 0x00 };

	StartOfScan() : marker(SegmentType::StartOfScan){}

	// Kommt innerhalb eines Segmentes irgendwann das Byte 0xff vor, muss die Bytefolge 0xff 0x00 ausgegeben werden
	// Dadurch ist ein Decoder sp�ter in der Lage, besch�digte Dateien zu dekodieren und sich auf Segmentgrenzen zu synchronisieren
	// => Es gibt keinen Segmentmarker 0xff 0x00
};