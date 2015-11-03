#include "stdafx.h"
#include "HuffmanCoding.h"
#include <queue>

void HuffmanTable::addSymbolCode(byte symbol, BitBufferPtr code)
{
	codeMap[symbol] = code;
	codeVector.push_back(code);
}

int HuffmanTable::getSymbolCount()
{
	return codeVector.size();
}

void HuffmanTable::fillArrays(byte* countArr, byte* codeArr)
{
	std::sort(codeVector.begin(), codeVector.end(), [](BitBufferPtr a, BitBufferPtr b)
	{
		return a->getSize() > b->getSize();
	});

	memset(countArr, 0, 16);
		
	int i = 0;
	for (auto symbolCodePair : codeMap)
	{
		countArr[symbolCodePair.second->getSize() - 1] += 1;
		codeArr[i++] = symbolCodePair.first;
	}
}


HuffmanTreeNode::HuffmanTreeNode(int frequency, HuffmanTreeNodePtr leftChild, HuffmanTreeNodePtr rightChild)
	: frequency(frequency), children(leftChild, rightChild)
{}

void HuffmanTreeNode::pushCodeBit(bool bit)
{
	children.first->pushCodeBit(bit);
	children.second->pushCodeBit(bit);
}

HuffmanTreeDataNode::HuffmanTreeDataNode(int frequency, BitBufferPtr bitBuffer)
	: HuffmanTreeNode(frequency, nullptr, nullptr), bitBuffer(bitBuffer)
{}

void HuffmanTreeDataNode::pushCodeBit(bool bit)
{
	bitBuffer->pushBit(bit);
}

std::map<byte, BitBufferPtr> HuffmanCoding::createHuffmanTable(std::vector<byte> srcData)
{
	std::map<byte, BitBufferPtr> result;

	std::vector<int> symbolsCount(NUM_BYTE_VALUES);

	auto comp = [](HuffmanTreeNodePtr a, HuffmanTreeNodePtr b) { return a->frequency > b->frequency; };
	std::priority_queue<HuffmanTreeNodePtr, std::vector<HuffmanTreeNodePtr>, decltype(comp)> huffmanTreeNodes(comp);

	for (byte symbol : srcData)
	{
		symbolsCount[(int)symbol]++;
	}

	for (int i = 0; i < symbolsCount.size(); i++)
	{
		int curSymbolCount = symbolsCount[i];
		if (curSymbolCount > 0) {
			BitBufferPtr bitBuffer = std::make_shared<BitBuffer>();
			huffmanTreeNodes.push(std::make_shared<HuffmanTreeDataNode>(curSymbolCount, bitBuffer));
			result[(byte)i] = bitBuffer;
		}
	}
	
	huffmanTreeNodes.top()->pushCodeBit(false);

	while (huffmanTreeNodes.size() > 1)
	{
		HuffmanTreeNodePtr lowNode0 = huffmanTreeNodes.top();
		huffmanTreeNodes.pop();
		HuffmanTreeNodePtr lowNode1 = huffmanTreeNodes.top();
		huffmanTreeNodes.pop();

		lowNode0->pushCodeBit(true);
		lowNode1->pushCodeBit(false);

		huffmanTreeNodes.push(std::make_shared<HuffmanTreeNode>(lowNode0->frequency + lowNode1->frequency, lowNode1, lowNode0));
	}

	return result;
}