#pragma once
#include <utility>
#include <memory>
#include <vector>
#include "BitBuffer.h"
#include <map>
#include <algorithm>

class HuffmanTable
{
	std::map<byte, BitBufferPtr> codeMap;
	std::vector<BitBufferPtr> codeVector;
	
public:
	void addSymbolCode(byte symbol, BitBufferPtr code);

	int getSymbolCount() const;

	void fillArrays(byte* countArr, byte* codeArr);
};


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

	bool hasChildren() const;

	void pushCodeBitToNodes();
	virtual void pushCodeBitToLeaves(bool bit);
};

class HuffmanTreeDataNode : public HuffmanTreeNode
{
public:
	HuffmanTreeDataNode(int frequency, BitBufferPtr bitBuffer);

	BitBufferPtr bitBuffer;

	void pushCodeBitToLeaves(bool bit) override;
};


class HuffmanCoding
{
public:
	static std::map<byte, BitBufferPtr> createHuffmanTable(std::vector<byte> srcData);
};