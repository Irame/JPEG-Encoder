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

	int lastCodeLength = dataNodes[0]->codeLength;
	unsigned short code = 0xffff >> (16 - lastCodeLength);
	for (int i = 0; i < dataNodes.size(); i++)
	{
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
				currentBitstream = BitBuffer();
			}
		}
	}

	return resultVector;
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