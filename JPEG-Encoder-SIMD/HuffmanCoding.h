#pragma once
#include <utility>
#include <memory>
#include <vector>
#include "BitBuffer.h"
#include <map>


class HuffmanTreeNode;
class HuffmanTreeDataNode;
typedef std::shared_ptr<HuffmanTreeNode> HuffmanTreeNodePtr;
typedef std::shared_ptr<HuffmanTreeDataNode> HuffmanTreeDataNodePtr;

class HuffmanTreeNode
{
public:
	virtual ~HuffmanTreeNode() {}

	HuffmanTreeNode(int frequency, HuffmanTreeNodePtr leftChild, HuffmanTreeNodePtr rightChild);

	int frequency;
	std::pair<HuffmanTreeNodePtr, HuffmanTreeNodePtr> children;

	virtual void pushCodeBit(bool bit);
};

class HuffmanTreeDataNode : public HuffmanTreeNode
{
public:
	HuffmanTreeDataNode(int frequency, BitBufferPtr bitBuffer);

	BitBufferPtr bitBuffer;

	void pushCodeBit(bool bit) override;
};


class HuffmanCoding
{
public:
	static std::map<byte, BitBufferPtr> createHuffmanTable(std::vector<byte> srcData);
};