#pragma once
#include <utility>
#include <memory>
#include "BitBuffer.h"
#include <map>

template<typename SymbolType>
class HuffmanTable;
class PackageMergeTreeNode;
template<typename SymbolType>
class PackageMergeTreeDataNode;
template<typename SymbolType>
using HuffmanTablePtr = std::shared_ptr<HuffmanTable<SymbolType>> ;
typedef std::shared_ptr<PackageMergeTreeNode> PackageMergeTreeNodePtr;
template<typename SymbolType>
using PackageMergeTreeDataNodePtr =  std::shared_ptr<PackageMergeTreeDataNode<SymbolType>>;

template<typename SymbolType>
class HuffmanTable
{
	typedef uint64_t InternalCodeType;

	HuffmanTable(size_t maxCodewordLength) : maxCodewordLength(maxCodewordLength) {};

	std::map<SymbolType, BitBufferPtr> codeMap;
	size_t maxCodewordLength;

public:
	size_t getSymbolCount() const;

	typename std::map<SymbolType, BitBufferPtr>::const_iterator begin() const;
	typename std::map<SymbolType, BitBufferPtr>::const_iterator end() const;

	static HuffmanTablePtr<SymbolType> create(size_t codeWordLength, const std::vector<SymbolType>& srcData);

	BitBufferPtr encode(const std::vector<SymbolType>& srcData);

	std::vector<SymbolType> decode(BitBufferPtr inputStream);
	std::vector<SymbolType> decode2(BitBufferPtr inputStream);

	friend std::ostream& operator<<(std::ostream& strm, const HuffmanTable<SymbolType>& huffmanTable)
	{
		for (auto it = huffmanTable.codeMap.cbegin(); it != huffmanTable.codeMap.cend(); ++it)
		{
			std::cout << int(it->first) << ": " << *it->second << std::endl;
		}
		return strm;
	}
};

class PackageMergeTreeNode
{
public:
	virtual ~PackageMergeTreeNode() {}

	PackageMergeTreeNode(size_t frequency, PackageMergeTreeNodePtr leftChild, PackageMergeTreeNodePtr rightChild);

	size_t frequency;
	std::pair<PackageMergeTreeNodePtr, PackageMergeTreeNodePtr> children;

	bool isLeave() const;

	virtual void incCodeLength();
};

template<typename SymbolType>
class PackageMergeTreeDataNode : public PackageMergeTreeNode
{
public:
	PackageMergeTreeDataNode(SymbolType data, size_t frequency);

	SymbolType data;
	size_t codeLength;

	void incCodeLength() override;
};




template<typename SymbolType>
size_t HuffmanTable<SymbolType>::getSymbolCount() const
{
	return codeMap.size();
}

template<typename SymbolType>
typename std::map<SymbolType, BitBufferPtr>::const_iterator HuffmanTable<SymbolType>::begin() const
{
	return codeMap.begin();
}

template<typename SymbolType>
typename std::map<SymbolType, BitBufferPtr>::const_iterator HuffmanTable<SymbolType>::end() const
{
	return codeMap.end();
}

// package-merge algorithm
// Managing Gigabytes: Compressing and Indexing Documents and Images (p. 402-404)
// https://books.google.de/books?id=2F74jyPl48EC&pg=PA402
template<typename SymbolType>
HuffmanTablePtr<SymbolType> HuffmanTable<SymbolType>::create(size_t codeWordLength, const std::vector<SymbolType>& srcData)
{
	if (codeWordLength > sizeof(InternalCodeType) * 8) return nullptr;

	HuffmanTablePtr<SymbolType> huffmanTable = std::shared_ptr<HuffmanTable>(new HuffmanTable(codeWordLength));

	if (srcData.size() == 0) return huffmanTable;

	auto comp = [](PackageMergeTreeNodePtr a, PackageMergeTreeNodePtr b) { return a->frequency < b->frequency; };
	std::vector<PackageMergeTreeNodePtr> origNodes;

	std::map<SymbolType, size_t> symbolsCount;
	for (SymbolType symbol : srcData)
	{
		++symbolsCount[static_cast<size_t>(symbol)];
	}

	for (std::pair<const SymbolType, size_t>& entry : symbolsCount)
	{
		origNodes.push_back(std::make_shared<PackageMergeTreeDataNode<SymbolType>>(entry.first, entry.second));
	}
	origNodes.push_back(std::make_shared<PackageMergeTreeDataNode<SymbolType>>(0, 0));			// eliminates 1*-codes

	if (codeWordLength < ceil(log2(origNodes.size()))) return nullptr;

	std::vector<PackageMergeTreeNodePtr> curNodes = origNodes;

	for (size_t l = 1; l <= codeWordLength; l++)
	{
		std::sort(curNodes.begin(), curNodes.end(), [](PackageMergeTreeNodePtr a, PackageMergeTreeNodePtr b)
		{
			return a->frequency < b->frequency;
		});

		std::vector<PackageMergeTreeNodePtr> nextNodes;
		if (l < codeWordLength) nextNodes = origNodes;
		for (size_t i = 1; i < curNodes.size(); i += 2)
		{
			PackageMergeTreeNodePtr leftNode = curNodes[i - 1];
			PackageMergeTreeNodePtr rightNode = curNodes[i];
			nextNodes.push_back(std::make_shared<PackageMergeTreeNode>(leftNode->frequency + rightNode->frequency, leftNode, rightNode));
		}
		curNodes = nextNodes;
	}

	for (auto node : curNodes)
	{
		node->incCodeLength();
	}

	std::vector<PackageMergeTreeDataNodePtr<SymbolType>> dataNodes(origNodes.size());

	transform(origNodes.begin(), origNodes.end(), dataNodes.begin(), std::static_pointer_cast<PackageMergeTreeDataNode<SymbolType>, PackageMergeTreeNode>);

	std::sort(dataNodes.begin(), dataNodes.end(), [](PackageMergeTreeDataNodePtr<SymbolType> a, PackageMergeTreeDataNodePtr<SymbolType> b)
	{
		return a->codeLength > b->codeLength;
	});

	size_t lastCodeLength = dataNodes[0]->frequency == 0 ? dataNodes[1]->codeLength : dataNodes[0]->codeLength;				// not sure if we need this
	InternalCodeType code = (~InternalCodeType(0) >> (sizeof(InternalCodeType) * 8 - lastCodeLength)) - 1;			// rightgrowing tree without 1*-codes
	for (size_t i = 0; i < dataNodes.size(); i++)
	{
		if (dataNodes[i]->frequency == 0) continue;				// eliminates 1*-codes
		size_t curCodeLength = dataNodes[i]->codeLength;
		if (curCodeLength != lastCodeLength) {
			code++;
			code >>= lastCodeLength - curCodeLength;
			code--;
		}
		auto curBitBuffer = std::make_shared<BitBuffer>();
		InternalCodeType beCode = _byteswap_uint64(code << (sizeof(InternalCodeType) * 8 - curCodeLength));
		curBitBuffer->pushBits(curCodeLength, &beCode);
		huffmanTable->codeMap[dataNodes[i]->data] = curBitBuffer;

		code--;
		lastCodeLength = curCodeLength;
	}

	return huffmanTable;
}

template<typename SymbolType>
BitBufferPtr HuffmanTable<SymbolType>::encode(const std::vector<SymbolType>& srcData)
{
	BitBufferPtr result = std::make_shared<BitBuffer>();

	for (SymbolType b : srcData)
	{
		result->pushBits(*codeMap[b]);
	}

	return result;
}

template<typename SymbolType>
std::vector<SymbolType> HuffmanTable<SymbolType>::decode(BitBufferPtr inputStream)
{
	std::vector<SymbolType> resultVector;
	BitBuffer currentBitstream = BitBuffer();

	for (int i = 0; i < inputStream->getSize(); i++)
	{
		currentBitstream.pushBit(inputStream->getBit(i));
		for (auto it = codeMap.cbegin(); it != codeMap.cend(); ++it)
		{
			if (*it->second == currentBitstream)
			{
				resultVector.push_back(it->first);
				currentBitstream.clear();
			}
		}
	}

	return resultVector;
}

template<typename SymbolType>
std::vector<SymbolType> HuffmanTable<SymbolType>::decode2(BitBufferPtr inputStream)
{
	struct EntryType
	{
		SymbolType symbol;
		BitBuffer code;
		size_t codeLength;


		EntryType(SymbolType symbol, const BitBuffer& code, size_t codeLength)
			: symbol(symbol), code(code), codeLength(codeLength)
		{}
	};

	std::vector<SymbolType> result;

	std::vector<EntryType> decodePairs;

	// create a sortable <BitBuffer, byte> map to work with
	for (auto it = codeMap.cbegin(); it != codeMap.cend(); ++it)
	{
		decodePairs.emplace_back(it->first, *it->second, it->second->getSize());
	}

	// sort table by codeword length and codeword bits
	std::sort(decodePairs.begin(), decodePairs.end(), [](const EntryType& a, const EntryType& b)
	{
		return a.codeLength == b.codeLength ? a.code < b.code : a.codeLength < b.codeLength;
	});

	// fill codewords with 1-bits
	unsigned short fillBits = 0xffff;
	for (auto& entry : decodePairs)
	{
		BitBuffer& curBitBuffer = entry.code;
		curBitBuffer.pushBits(maxCodewordLength - curBitBuffer.getSize(), &fillBits);
	}

	size_t offset = 0;
	while (offset < inputStream->getSize())
	{
		BitBuffer curBitBuffer;
		InternalCodeType buffer = 0;
		inputStream->getBits(offset, &buffer, std::min(maxCodewordLength, inputStream->getSize()));
		curBitBuffer.pushBits(maxCodewordLength, &buffer);

		for (auto& entry : decodePairs)
		{
			if (!(entry.code < curBitBuffer))
			{
				result.push_back(entry.symbol);
				offset += entry.codeLength;
				break;
			}
		}
	}

	return result;
}

inline PackageMergeTreeNode::PackageMergeTreeNode(size_t frequency, PackageMergeTreeNodePtr leftChild, PackageMergeTreeNodePtr rightChild)
	: frequency(frequency), children(leftChild, rightChild)
{}

inline bool PackageMergeTreeNode::isLeave() const
{
	return children.first != nullptr && children.second != nullptr;
}

inline void PackageMergeTreeNode::incCodeLength()
{
	children.first->incCodeLength();
	children.second->incCodeLength();
}

template<typename SymbolType>
PackageMergeTreeDataNode<SymbolType>::PackageMergeTreeDataNode(SymbolType data, size_t frequency)
	: PackageMergeTreeNode(frequency, nullptr, nullptr), data(data), codeLength(0)
{}

template<typename SymbolType>
void PackageMergeTreeDataNode<SymbolType>::incCodeLength()
{
	codeLength++;
}