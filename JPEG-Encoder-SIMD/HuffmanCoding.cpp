#include "stdafx.h"
#include "HuffmanCoding.h"
#include <queue>
#include "SortedLinkedList.h"

int HuffmanTable::getSymbolCount() const
{
	return codeMap.size();
}

HuffmanTreeNode::HuffmanTreeNode(int frequency, HuffmanTreeNodePtr leftChild, HuffmanTreeNodePtr rightChild)
	: frequency(frequency), children(leftChild, rightChild)
{}

void HuffmanTreeNode::pushCodeBitToLeaves(bool bit)
{
	children.first->pushCodeBitToLeaves(bit);
	children.second->pushCodeBitToLeaves(bit);
}

void HuffmanTreeNode::pushCodeBitToNodes()
{
	if (!hasChildren()) return;
	children.first->pushCodeBitToLeaves(false);
	children.second->pushCodeBitToLeaves(true);
	children.first->pushCodeBitToNodes();
	children.second->pushCodeBitToNodes();
}

bool HuffmanTreeNode::hasChildren() const
{
	return children.first != nullptr || children.second != nullptr;
}

HuffmanTreeDataNode::HuffmanTreeDataNode(int frequency, BitBufferPtr bitBuffer)
	: HuffmanTreeNode(frequency, nullptr, nullptr), bitBuffer(bitBuffer)
{}

void HuffmanTreeDataNode::pushCodeBitToLeaves(bool bit)
{
	bitBuffer->pushBit(bit);
}

HuffmanTablePtr HuffmanTable::createHuffmanTable(std::vector<byte> srcData)
{
	std::map<byte, BitBufferPtr> result;

	std::vector<int> symbolsCount(NUM_BYTE_VALUES);

	auto comp = [](HuffmanTreeNodePtr a, HuffmanTreeNodePtr b) { return a->frequency < b->frequency; };
	SortedLinkedList<HuffmanTreeNodePtr, decltype(comp)> huffmanTreeNodes(comp);

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

	while (huffmanTreeNodes.size() > 1)
	{
		HuffmanTreeNodePtr lowNodeRight = huffmanTreeNodes.top();
		huffmanTreeNodes.pop();
		HuffmanTreeNodePtr lowNodeLeft = huffmanTreeNodes.top();
		huffmanTreeNodes.pop();

		if (lowNodeLeft->hasChildren() && !lowNodeRight->hasChildren())
			swap(lowNodeLeft, lowNodeRight);

		huffmanTreeNodes.push(std::make_shared<HuffmanTreeNode>(lowNodeRight->frequency + lowNodeLeft->frequency, lowNodeLeft, lowNodeRight));
	}

	HuffmanTreeNodePtr curNode = huffmanTreeNodes.top();
	curNode->pushCodeBitToNodes();
	
	while(curNode->hasChildren())
	{
		curNode = curNode->children.second;
	}
	
	curNode->pushCodeBitToLeaves(false);

	HuffmanTablePtr huffmanTable = std::make_shared<HuffmanTable>();
	huffmanTable->codeMap = result;

	return huffmanTable;
}