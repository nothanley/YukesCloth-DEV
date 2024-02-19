#include <fstream>
#pragma once

class StTag;
class CSimObj;
class TagBuffer;

#define BINARY_ALIGNMENT 16

class CClothEncoder
{

public:
	CClothEncoder(std::ofstream* outStream, CSimObj* pSimObj);

private:
	void WriteTagHead(TagBuffer* pNode);
	void WriteTree(TagBuffer* pRootNode);
	void EncodeAllTags(TagBuffer* pRootNode);
	void EncodeTag(TagBuffer* tag);

private:
	void InitTagBuffers(TagBuffer* pTagBf);
	void EncodeRootTag(TagBuffer* pTag);
	void EncodeSimMesh(TagBuffer* pTag);
	void EncodeSimVerts(TagBuffer* pTag);
	void EncodeSubObj(TagBuffer* pTag);
	void EncodeSubObjVerts(TagBuffer* pTag);
	void EncodeRecalcNormals(TagBuffer* pTag);
	void EncodeRCNData(TagBuffer* pTag);
	void EncodeSimMeshSkin(TagBuffer* pTag);
	void EncodeSimLinkSource(TagBuffer* pTag);
	void EncodePatternEntry(TagBuffer* pTag);
	void EncodeStacks(TagBuffer* pTag);
	void EncodeSkinCalc(TagBuffer* pTag);
	void EncodeSkinPaste(TagBuffer* pTag);
	void EncodeVertexSave(TagBuffer* pTag);
	void EncodeForceField(TagBuffer* pTag);
	void EncodeConstraint_Stretch(TagBuffer* pTag);
	void EncodeConstraint_Std(TagBuffer* pTag);
	void EncodeConstraint_Bend(TagBuffer* pTag);
	void EncodeBendStiffness(TagBuffer* pTag);
	void EncodeColVerts(TagBuffer* pTag);
	void EncodeConstraint_Fixation(TagBuffer* pTag);
	void EncodeSimLine(TagBuffer* pTag);
	void EncodeLineDef(TagBuffer* pTag);
	void EncodeAssignNode(TagBuffer* pTag);
	void EncodeLinkTar(TagBuffer* pTag);
	void EncodeStringTable(TagBuffer* pTag);
	void EncodeString(TagBuffer* pTag);
	void EncodeNodeTable(TagBuffer* pTag);
	void EncodeColIdTbl(TagBuffer* pTag);
	void EncodeCols(TagBuffer* pTag);
	void EncodeColPack(TagBuffer* pTag);
	void EncodeCapsuleStandard(TagBuffer* pTag);
	void EncodeCapsuleTapered(TagBuffer* pTag);
	void EncodeColIdInfo(TagBuffer* pTag);


private:
	uint32_t GetTagTotalSize(TagBuffer* pTag);

private:
	CSimObj* m_pSimObj = nullptr;
	std::ofstream* m_pDataStream = nullptr;
	uintptr_t m_iStreamPos = std::ios::beg;
};
