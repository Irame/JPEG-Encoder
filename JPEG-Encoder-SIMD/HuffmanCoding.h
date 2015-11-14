#pragma once
#include <utility>
#include <memory>
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

	static HuffmanTablePtr create(size_t codeWordLength, const std::vector<byte>& srcData);

	BitBufferPtr encode(const std::vector<byte>& srcData);

	std::vector<byte> decode(BitBufferPtr inputStream);
	std::vector<byte> decode2(BitBufferPtr inputStream);
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

class PackageMergeTreeDataNode : public PackageMergeTreeNode
{
public:
	PackageMergeTreeDataNode(byte data, size_t frequency);

	byte data;
	size_t codeLength;

	void incCodeLength() override;
};