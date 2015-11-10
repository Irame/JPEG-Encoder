#pragma once
#include <utility>
#include <memory>
#include <vector>
#include "BitBuffer.h"
#include <map>

class HuffmanTable;
class PackageMergeTreeNode;
class PackageMergeTreeDataNode;
typedef std::shared_ptr<HuffmanTable> HuffmanTablePtr;
typedef std::shared_ptr<PackageMergeTreeNode> PackageMergeTreeNodePtr;
typedef std::shared_ptr<PackageMergeTreeDataNode> PackageMergeTreeDataNodePtr;

class HuffmanTable
{
	HuffmanTable() {};
public:

	std::map<byte, BitBufferPtr> codeMap;

	size_t getSymbolCount() const;

	static HuffmanTablePtr createHuffmanTable(size_t codeWordLength, std::vector<byte> srcData);
};


class PackageMergeTreeNode
{
public:
	virtual ~PackageMergeTreeNode() {}

	PackageMergeTreeNode(int frequency, PackageMergeTreeNodePtr leftChild, PackageMergeTreeNodePtr rightChild);

	int frequency;
	std::pair<PackageMergeTreeNodePtr, PackageMergeTreeNodePtr> children;

	bool isLeave() const;

	virtual void incCodeLength();
};

class PackageMergeTreeDataNode : public PackageMergeTreeNode
{
public:
	PackageMergeTreeDataNode(byte data, int frequency);

	byte data;
	int codeLength;

	void incCodeLength() override;
};