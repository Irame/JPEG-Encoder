#include "stdafx.h"
#include "HuffmanCoding.h"
#include <queue>
#include <functional>
#include <vector>

size_t HuffmanTable::getSymbolCount() const
{
	return codeMap.size();
}

// package-merge algorithm
// Managing Gigabytes: Compressing and Indexing Documents and Images (p. 402-404)
// https://books.google.de/books?id=2F74jyPl48EC&pg=PA402
HuffmanTablePtr HuffmanTable::create(size_t codeWordLength, const std::vector<byte>& srcData)
{
	HuffmanTablePtr huffmanTable = std::shared_ptr<HuffmanTable>(new HuffmanTable());
	
	if (srcData.size() == 0) return huffmanTable;

	std::vector<int> symbolsCount(NUM_BYTE_VALUES);

	auto comp = [](PackageMergeTreeNodePtr a, PackageMergeTreeNodePtr b) { return a->frequency < b->frequency; };
	std::vector<PackageMergeTreeNodePtr> origNodes;

	for (byte symbol : srcData)
	{
		symbolsCount[(int)symbol]++;
	}

	for (int i = 0; i < symbolsCount.size(); i++)
	{
		int curSymbolCount = symbolsCount[i];
		if (curSymbolCount > 0) {
			origNodes.push_back(std::make_shared<PackageMergeTreeDataNode>(i, curSymbolCount));
		}
	}
	origNodes.push_back(std::make_shared<PackageMergeTreeDataNode>(0, 0));			// eliminates 1*-codes

	if (codeWordLength < ceil(log2(origNodes.size()))) return nullptr;

	std::vector<PackageMergeTreeNodePtr> curNodes = origNodes;

	for (int l = 1; l <= codeWordLength; l++)
	{
		std::sort(curNodes.begin(), curNodes.end(), [](PackageMergeTreeNodePtr a, PackageMergeTreeNodePtr b)
		{
			return a->frequency < b->frequency;
		});

		std::vector<PackageMergeTreeNodePtr> nextNodes;
		if (l < codeWordLength) nextNodes = origNodes;
		for (int i = 1; i < curNodes.size(); i += 2)
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

	std::vector<PackageMergeTreeDataNodePtr> dataNodes(origNodes.size());

	transform(origNodes.begin(), origNodes.end(), dataNodes.begin(), std::static_pointer_cast<PackageMergeTreeDataNode, PackageMergeTreeNode>);

	std::sort(dataNodes.begin(), dataNodes.end(), [](PackageMergeTreeDataNodePtr a, PackageMergeTreeDataNodePtr b)
	{
		return a->codeLength > b->codeLength;
	});

	int lastCodeLength = dataNodes[0]->frequency == 0 ? dataNodes[1]->codeLength : dataNodes[0]->codeLength;				// not sure if we need this
	unsigned short code = (0xffff >> (16 - lastCodeLength)) - 1;			// rightgrowing tree without 1*-codes
	for (int i = 0; i < dataNodes.size(); i++)
	{
		if (dataNodes[i]->frequency == 0) continue;				// eliminates 1*-codes
		int curCodeLength = dataNodes[i]->codeLength;
		if (curCodeLength != lastCodeLength) {
			code++;
			code >>= lastCodeLength - curCodeLength;
			code--;
		}
		auto curBitBuffer = std::make_shared<BitBuffer>();
		unsigned short beCode = _byteswap_ushort(code << (16 - curCodeLength));
		curBitBuffer->pushBits(curCodeLength, &beCode);
		huffmanTable->codeMap[dataNodes[i]->data] = curBitBuffer;

		code--;
		lastCodeLength = curCodeLength;
	}

	return huffmanTable;
}

BitBufferPtr HuffmanTable::encode(const std::vector<byte>& srcData)
{
	BitBufferPtr result = std::make_shared<BitBuffer>();

	for (byte b : srcData)
	{
		result->pushBits(*codeMap[b]);
	}

	return result;
}

std::vector<byte> HuffmanTable::decode(BitBufferPtr inputStream)
{
	std::vector<byte> resultVector;
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

std::vector<byte> HuffmanTable::decode2(BitBufferPtr inputStream)
{
	std::vector<byte> result;

	typedef std::tuple<BitBuffer, size_t, byte> EntryType;
	std::vector<EntryType> decodePairs;

	// create a sortable <BitBuffer, byte> map to work with
	for (auto it = codeMap.cbegin(); it != codeMap.cend(); ++it)
	{
		decodePairs.emplace_back(*it->second, it->second->getSize(), it->first);
	}

	// sort table by codeword length and codeword bits
	std::sort(decodePairs.begin(), decodePairs.end(), [](const EntryType& a, const EntryType& b)
	{
		return std::get<1>(a) == std::get<1>(b) ? std::get<0>(a) < std::get<0>(b) : std::get<1>(a) < std::get<1>(b);
	});

	// fill codewords with 1-bits
	unsigned short fillBits = 0xffff;
	for (auto& tuple : decodePairs)
	{
		BitBuffer& curBitBuffer = std::get<0>(tuple);
		curBitBuffer.pushBits(16 - curBitBuffer.getSize(), &fillBits);
	}

	size_t offset = 0;
	while(offset < inputStream->getSize())
	{
		BitBuffer curBitBuffer;
		byte buffer[2] = {0, 0};
		inputStream->getBits(offset, buffer, std::min(size_t(16), inputStream->getSize()));
		curBitBuffer.pushBits(16, buffer);

		for (auto& tuple : decodePairs)
		{
			if (!(std::get<0>(tuple) < curBitBuffer))
			{
				result.push_back(std::get<2>(tuple));
				offset += std::get<1>(tuple);
				break;
			}
		}
	}
	
	return result;
}

PackageMergeTreeNode::PackageMergeTreeNode(int frequency, PackageMergeTreeNodePtr leftChild, PackageMergeTreeNodePtr rightChild)
	: frequency(frequency), children(leftChild, rightChild)
{}

bool PackageMergeTreeNode::isLeave() const
{
	return children.first != nullptr && children.second != nullptr;
}

void PackageMergeTreeNode::incCodeLength()
{
	children.first->incCodeLength();
	children.second->incCodeLength();
}

PackageMergeTreeDataNode::PackageMergeTreeDataNode(byte data, int frequency)
	: PackageMergeTreeNode(frequency, nullptr, nullptr), data(data), codeLength(0)
{}

void PackageMergeTreeDataNode::incCodeLength()
{
	codeLength++;
}