#pragma once
#include <utility>
#include <memory>
#include <vector>
#include "BitBuffer.h"
#include <map>
#include <algorithm>

class HuffmanTable;
class HuffmanTreeNode;
class HuffmanTreeDataNode;
typedef std::shared_ptr<HuffmanTable> HuffmanTablePtr;
typedef std::shared_ptr<HuffmanTreeNode> HuffmanTreeNodePtr;
typedef std::shared_ptr<HuffmanTreeDataNode> HuffmanTreeDataNodePtr;

class HuffmanTable
{
	HuffmanTable() {};
public:

	std::map<byte, BitBufferPtr> codeMap;

	size_t getSymbolCount() const;

	static HuffmanTablePtr createHuffmanTable(std::vector<byte> srcData);
};


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