#pragma once
#include "stdafx.h"

#include <array>

#include "HuffmanCoding.h"

template<>
class HuffmanTable<byte>
{
private:
	typedef uint64_t InternalCodeType;
	typedef std::vector<std::pair<byte, BitBufferPtr>> InnerCodeMap;

public:
	// Special iterator to skip zero symbol entires in innerCodeMap
	struct const_iterator
	{
		typedef HuffmanTable<byte>::InnerCodeMap::const_iterator IteratorType;
		typedef HuffmanTable<byte>::InnerCodeMap::const_reference const_reference;

		IteratorType innerIterator;
		IteratorType endIterator;

	public:
		const_iterator(IteratorType innerIterator, IteratorType endIterator) 
			: innerIterator(innerIterator), endIterator(endIterator) { }

		const_reference operator*() { return *innerIterator; }
		bool operator==(const const_iterator& other) const { return innerIterator == other.innerIterator; }
		bool operator!=(const const_iterator& other) const { return innerIterator != other.innerIterator; }
		const_iterator& operator++() { 
			while ((++innerIterator) != endIterator && (*innerIterator).first == 0);
			return *this; 
		}
	};

public:
	HuffmanTable(size_t maxCodewordLength) : codeMap(256), maxCodewordLength(maxCodewordLength), codedSymbolCount(0) {};

	InnerCodeMap codeMap;
	size_t maxCodewordLength;
	size_t codedSymbolCount; // size of the HuffmanTable

public:
	size_t getSymbolCount() const;

	const_iterator begin() const;
	const_iterator end() const;

	static HuffmanTablePtr<byte> create(size_t codeWordLength, const std::vector<byte>& srcData);

	BitBufferPtr encode(const std::vector<byte>& srcData);
	std::vector<byte> decode(BitBufferPtr inputStream);

	friend std::ostream& operator<<(std::ostream& strm, const HuffmanTable<byte>& huffmanTable)
	{
		for (const auto& pair : huffmanTable)
		{
			std::cout << pair.first << ": " << *(pair.second) << std::endl;
		}

		return strm;
	}
};

inline size_t HuffmanTable<byte>::getSymbolCount() const
{
	return 0; // TODO
}

inline HuffmanTable<byte>::const_iterator HuffmanTable<byte>::begin() const
{
	return const_iterator(this->codeMap.begin(), this->codeMap.end());
}

inline HuffmanTable<byte>::const_iterator HuffmanTable<byte>::end() const
{
	return const_iterator(this->codeMap.end(), this->codeMap.end());
}

inline HuffmanTablePtr<byte> HuffmanTable<byte>::create(size_t codeWordLength, const std::vector<byte>& srcData)
{
	// code words can't be greater then the longest code storeable in 'InternalCodeType'
	if (codeWordLength > sizeof(InternalCodeType) * 8) return nullptr;

	HuffmanTablePtr<byte> huffmanTable = std::shared_ptr<HuffmanTable<byte>>(new HuffmanTable(codeWordLength));

	// if no input symbols are given an empty table is returned
	if (srcData.size() == 0) return huffmanTable;

	// count the number of occurences for each symbol in 'srcData'
	std::array<size_t, 256> symbolsCount{0};
	for (byte symbol : srcData)
	{
		++symbolsCount[static_cast<size_t>(symbol)];
	}

	// the lower bound of the code word length is ceil(log2(numberOfDifferentSymbols + 1)) (+1 => dummy symbol)
	if (codeWordLength < ceil(log2(symbolsCount.size() + 1))) return nullptr;

	// creating leaves for each symbol in symbolsCount
	std::vector<PackageMergeTreeNodePtr> origNodes;
	for (size_t i = 0; i < symbolsCount.size(); i++)
	{
		size_t count = symbolsCount[i];
		if (count != 0)
		{
			huffmanTable->codedSymbolCount++;
			origNodes.push_back(std::make_shared<PackageMergeTreeDataNode<byte>>(static_cast<byte>(i), count));
		}
	}

	// add a dummy leave with a frequency of 0 to eliminate 1* codes
	origNodes.push_back(std::make_shared<PackageMergeTreeDataNode<byte>>(0, 0));

	// execute the core part of the package merge algorithm
	std::vector<PackageMergeTreeNodePtr> curNodes = origNodes;
	for (size_t l = 1; l < codeWordLength; l++)
	{
		// sort 'curNodes' by frequency such that the lowest frequency is on top
		std::sort(curNodes.begin(), curNodes.end(), [](PackageMergeTreeNodePtr a, PackageMergeTreeNodePtr b)
		{
			return a->frequency < b->frequency;
		});

		// pack pairs of two nodes from 'curNodes', merge them with 'origNodes', and store them in 'nextNodes'
		std::vector<PackageMergeTreeNodePtr> nextNodes = origNodes;
		for (size_t i = 1; i < curNodes.size(); i += 2)
		{
			PackageMergeTreeNodePtr leftNode = curNodes[i - 1];
			PackageMergeTreeNodePtr rightNode = curNodes[i];
			nextNodes.push_back(std::make_shared<PackageMergeTreeNode>(leftNode->frequency + rightNode->frequency, leftNode, rightNode));
		}
		curNodes = nextNodes;
	}

	// sort 'curNodes' by frequency such that the lowest frequency is on top
	std::sort(curNodes.begin(), curNodes.end(), [](PackageMergeTreeNodePtr a, PackageMergeTreeNodePtr b)
	{
		return a->frequency < b->frequency;
	});

	// take 2*numberOfDifferentSymbols - 2 nodes from 'origNodes' and increment the codeLength of the leaves from each of these nodes by one
	for (size_t i = 0; i < 2 * origNodes.size() - 2; i++)
	{
		curNodes[i]->incCodeLength();
	}

	// get all leaves as 'PackageMergeTreeDataNodePtr'
	std::vector<PackageMergeTreeDataNodePtr<byte>> dataNodes(origNodes.size());
	transform(origNodes.begin(), origNodes.end(), dataNodes.begin(), std::static_pointer_cast<PackageMergeTreeDataNode<byte>, PackageMergeTreeNode>);

	// sort 'dataNodes' by codeLength such that the longest code length is on top
	std::sort(dataNodes.begin(), dataNodes.end(), [](PackageMergeTreeDataNodePtr<byte> a, PackageMergeTreeDataNodePtr<byte> b)
	{
		return a->codeLength > b->codeLength;
	});

	// get the greatest code length
	size_t lastCodeLength = dataNodes[0]->codeLength;

	// start with a 1* code to ensure a rightgrowing
	InternalCodeType code = ~InternalCodeType(0) >> (sizeof(InternalCodeType) * 8 - lastCodeLength);
	for (size_t i = 0; i < dataNodes.size(); i++)
	{
		// ignore our dummy leave
		if (dataNodes[i]->frequency == 0) continue;

		size_t curCodeLength = dataNodes[i]->codeLength;

		// if the codelength changes we have to go up in our tree
		if (curCodeLength != lastCodeLength) {
			code >>= lastCodeLength - curCodeLength;
		}

		// go one left in our tree (also eliminates 1* codes)
		code--;

		// create a bitbuffer with the current code and put it into the 'huffmanTable->codeMap'
		auto curBitBuffer = std::make_shared<BitBuffer>();
		InternalCodeType beCode = _byteswap_uint64(code << (sizeof(InternalCodeType) * 8 - curCodeLength));
		curBitBuffer->pushBits(curCodeLength, &beCode);
		huffmanTable->codeMap[dataNodes[i]->data] = { dataNodes[i]->data, curBitBuffer };

		lastCodeLength = curCodeLength;
	}

	return huffmanTable;
}


inline BitBufferPtr HuffmanTable<byte>::encode(const std::vector<byte>& srcData)
{
	BitBufferPtr result = std::make_shared<BitBuffer>();

	for (byte b : srcData)
	{
		result->pushBits(*(codeMap[b].second));
	}

	return result;
}

inline std::vector<byte> HuffmanTable<byte>::decode(BitBufferPtr inputStream)
{
	std::vector<byte> resultVector;
	BitBuffer currentBitstream = BitBuffer();

	for (int i = 0; i < inputStream->getSize(); i++)
	{
		currentBitstream.pushBit(inputStream->getBit(i));
		for (const auto& pair : *this)
		{
			if (*(pair.second) == currentBitstream)
			{
				resultVector.push_back(pair.first);
				currentBitstream.clear();
			}
		}
	}

	return resultVector;
}


