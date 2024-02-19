#include <iostream>
#include <fstream>
#include <sstream>
#include "BinaryIO.h"
#include <Encoder/ClothEncoder.h>
#include <Cloth/ClothStructs.h>
#include <Cloth/SimObj.h>
#include <Cloth/SimMesh.h>

using namespace std;
using namespace BinaryIO;

struct TagBuffer {
	StTag* tag;
	uint32_t sBegin = 0x10;
	uint32_t sSize;
	std::vector<TagBuffer*> children;
	bool bUsePreDef = false;

	std::stringstream binary = 
		std::stringstream(
		std::ios::binary |
		std::ios::out);
};

CClothEncoder::CClothEncoder(std::ofstream* outStream, CSimObj* pSimObj) {
	m_pDataStream = outStream;
	m_pSimObj = pSimObj;

	/* Setup header tag buffer */
	TagBuffer* pRootBuffer = new TagBuffer;
	pRootBuffer->tag = pSimObj->m_pStHead;

	/* Recursively create all child tag buffers */
	this->InitTagBuffers(pRootBuffer);

	/* Serialize all datasets into formatted binaries */
	this->EncodeAllTags(pRootBuffer);

	/* Merge datasets and write tree hierarchy to out file */
	WriteTree(pRootBuffer);
}

uint32_t
CClothEncoder::GetTagTotalSize(TagBuffer* pTag) {
	uint32_t sSize = pTag->binary.str().size() + 0xC;

	for (auto child : pTag->children) {
		sSize += GetTagTotalSize(child);
	}

	return sSize;
}

void
CClothEncoder::WriteTagHead(TagBuffer* pTagBuf) {
	pTagBuf->sSize = GetTagTotalSize(pTagBuf);
	//printf("\nSize of Tag: %d", pTagBuf->sSize);

	/* Write stream metadata */
	WriteUInt32(m_pDataStream, pTagBuf->tag->eType);
	WriteUInt32(m_pDataStream, pTagBuf->sBegin);
	WriteUInt32(m_pDataStream, pTagBuf->sSize);

	/* Write defined binary */
	m_pDataStream->write(pTagBuf->binary.str().data(), pTagBuf->binary.str().size());
}

void
RegenerateStringTable(StTag* tag, CSimObj* pSimObj) {
	if (tag->eType == enTagType_StrTbl) {
		tag->children.clear();
		for (auto& str : pSimObj->m_sStringTable) {
			StTag* child = new StTag;
			child->eType = enTagType_String;
			child->pParent = tag;
			child->sTagName = str;
			tag->children.push_back(child);
		}
	}
}

void
RegenerateNodeTable(StTag* tag, CSimObj* pSimObj) {
	if (tag->eType == enTagType_NodeTbl) {
		tag->children.clear();
		for (auto& node : pSimObj->m_NodeTable) {
			StTag* child = new StTag;
			child->eType = enTagType_String;
			child->pParent = tag;
			child->sTagName = node.name;
			tag->children.push_back(child);
		}
	}
}
void
CClothEncoder::WriteTree(TagBuffer* pTagBuf) {
	WriteTagHead(pTagBuf);

	for (auto& tag : pTagBuf->children) {
		WriteTree(tag);
	}
}

void
CClothEncoder::InitTagBuffers(TagBuffer* pParentBuf)
{
	for (auto& tag : pParentBuf->tag->children) {

		/* Generate new string and node tables */
		RegenerateStringTable(tag,m_pSimObj);
		RegenerateNodeTable(tag, m_pSimObj);

		/* Create TagBuffer */
		TagBuffer* pChildBuf = new TagBuffer;
		pChildBuf->tag = tag;

		InitTagBuffers(pChildBuf);
		pParentBuf->children.push_back(pChildBuf);
	}
}

void
CClothEncoder::EncodeAllTags(TagBuffer* pRootNode) {
	EncodeTag(pRootNode);

	for (auto &tag : pRootNode->children) {
		EncodeAllTags(tag);
	}
}

void
CClothEncoder::EncodeTag(TagBuffer* pTagBuf) {

	StTag* pTag = pTagBuf->tag;

	if (pTag->eType != uint32_t(0x18)) {
		pTag->sTagName = yclutils::GetNodeName(pTag->eType);
	}


	switch (pTag->eType) {
		case enTagType_Root:
			EncodeRootTag(pTagBuf);
			break;
		case enTagType_SimMesh:
			EncodeSimMesh(pTagBuf);
			break;
		case enTagType_SimMesh_AssignSubObj:
			EncodeSubObj(pTagBuf);
			break;
		case enTagType_SimMesh_AssignSubObjVtx:
			EncodeSubObjVerts(pTagBuf);
			break;
		case enTagType_SimMesh_AssignSimVtx:
			EncodeSimVerts(pTagBuf);
			break;
		case enTagType_SimLine_AssignNode:
			EncodeAssignNode(pTagBuf);
			break;
		case enTagType_SimMesh_RCN:
			EncodeRecalcNormals(pTagBuf);
			break;
		case enTagType_SimMesh_RCNSubObj:
			EncodeRCNData(pTagBuf);
			break;
		case enTagType_SimMesh_Skin:
			EncodeSimMeshSkin(pTagBuf);
			break;
		case enTagType_SimMesh_SimLinkSrc:
			EncodeSimLinkSource(pTagBuf);
			break;
		case enTagType_SimMesh_Pattern:
			EncodePatternEntry(pTagBuf);
			break;
		case enTagType_SimMesh_Stacks:
			EncodeStacks(pTagBuf);
			break;
		case enTagType_SimMesh_SkinCalc:
			EncodeSkinCalc(pTagBuf);
			break;
		case enTagType_SimMesh_SkinPaste:
			EncodeSkinPaste(pTagBuf);
			break;
		case enTagType_SimMesh_OldVtxSave:
			EncodeVertexSave(pTagBuf);
			break;
		case enTagType_SimMesh_Force:
			EncodeForceField(pTagBuf);
			break;
		case enTagType_SimMesh_CtStretchLink:
			EncodeConstraint_Stretch(pTagBuf);
			break;
		case enTagType_SimMesh_CtStdLink:
			EncodeConstraint_Std(pTagBuf);
			break;
		case enTagType_SimMesh_CtBendLink:
			EncodeConstraint_Bend(pTagBuf);
			break;
		case enTagType_SimMesh_BendingStiffness:
			EncodeBendStiffness(pTagBuf);
			break;
		case enTagType_SimMesh_ColVtx:
			EncodeColVerts(pTagBuf);
			break;
		case enTagType_SimMesh_CtFixation:
			EncodeConstraint_Fixation(pTagBuf);
			break;
		case enTagType_SimLine:
			EncodeSimLine(pTagBuf);
			break;
		case enTagType_SimLine_LineDef:
			EncodeLineDef(pTagBuf);
			break;
		case enTagType_SimMesh_SimLinkTar:
			EncodeLinkTar(pTagBuf);
			break;
		case enTagType_StrTbl:
			EncodeStringTable(pTagBuf);
			break;
		case enTagType_String:
			EncodeString(pTagBuf);
			break;
		case enTagType_NodeTbl:
			EncodeNodeTable(pTagBuf);
			break;
		case enTagType_ColIDTbl:
			EncodeColIdTbl(pTagBuf);
			break;
		case enTagType_Cols:
			EncodeCols(pTagBuf);
			break;
		case enTagType_ColPack:
			EncodeColPack(pTagBuf);
			break;
		case enTagType_Capsule_Standard:
			EncodeCapsuleStandard(pTagBuf);
			break;
		case enTagType_Capsule_Tapered:
			EncodeCapsuleTapered(pTagBuf);
			break;
		case enTagType_ColIDInfo:
			EncodeColIdInfo(pTagBuf);
			break;
		default:
			std::cerr << "Could not resolve encode format for tag type: " << pTag->eType << ". Skipping node..." << endl;
			pTagBuf->bUsePreDef = true;
			break;

	}

}

int
FindNodeIndex(const std::string& sTarget, std::vector<SimNode> nodeTable) 
{
	for (int i = 0; i < nodeTable.size(); i++) {
		if (sTarget == nodeTable.at(i).name) {
			return i;
		}
	}

	return -1;
}

int
FindStringIndex(const std::string& sTarget, std::vector<std::string> stringTable)
{
	for (int i = 0; i < stringTable.size(); i++) {
		if (sTarget == stringTable.at(i)) {
			return i;
		}
	}

	return -1;
}

void
AlignStream(std::stringstream& stream, const int& width, const uintptr_t& offset = 0) {
	while ((uint32_t(stream.str().size()) + offset) % width != 0) {
		WriteByte(stream, 0x0);
	}
}

void
AlignTagHeader(TagBuffer* pTagBuf) {
	AlignStream(pTagBuf->binary, 16, 0xC);
	pTagBuf->sBegin = 0xC + pTagBuf->binary.tellp();
}

void
WriteRCNIndexBuffer(std::stringstream& pBf, StSimMesh* sMesh, uint32_t numElements)
{
	/* Write the size buffer */
	for (uint32_t i = 0; i < numElements; i++)
	{
		uint8_t indexSize = sMesh->rcn.degCounts.at(i);
		WriteByte(pBf, indexSize);
	}
	AlignStream(pBf, 0x4);

	/* Write index buffer */
	int itemIdx = 0;
	for (uint32_t i = 0; i < numElements; i++)
	{
		uint8_t width = sMesh->rcn.degCounts.at(i);

		for (uint8_t j = 0; j < width; j++)
		{
			uint32_t index = sMesh->rcn.faceNrmVtxIndices.at(itemIdx + j);
			WriteUInt32(pBf, index);
		}

		itemIdx += width;
	}
}

void
WriteRCNNormalBuffer(std::stringstream& pBf, StSimMesh* sMesh, uint32_t numElements)
{
	/* Write size values */
	for (uint32_t i = 0; i < numElements; i++) {
		uint8_t indexSize = sMesh->rcn.linkCounts.at(i);
		WriteByte(pBf, indexSize);
	}
	AlignStream(pBf, 0x4);

	/* Write index buffer */
	int itemIdx = 0;
	for (uint32_t i = 0; i < numElements; i++)
	{
		uint8_t width = sMesh->rcn.linkCounts.at(i);

		for (uint8_t j = 0; j < width; j++)
		{
			SkinVertex vert = sMesh->rcn.vtxNrmFaceLinks.at(itemIdx + j);
			WriteUInt32(pBf, vert.index);
			WriteFloat(pBf, vert.weight);
		}

		itemIdx += width;
	}
}

std::vector<SimNode> 
GetNodePalette(MeshSkin& skin, CSimObj* pSimObj, int& numWeights)
{
	std::vector<SimNode> nodePalette;
	for (auto& weight : skin.blendWeights)
	{
		if (weight.numWeights > numWeights) {
			numWeights = weight.numWeights;
		}

		for (auto& bone : weight.bones)
		{
			int paletteIndex = FindNodeIndex(bone, nodePalette);
			int nodeIndex = FindNodeIndex(bone, pSimObj->m_NodeTable);
			if (paletteIndex == -1 && nodeIndex != -1) {
				nodePalette.push_back(pSimObj->m_NodeTable.at(nodeIndex));
			}
		}
	}
	return nodePalette;
}

void
WriteNodeSkinMatrices(TagBuffer* pTagBuf, MeshSkin& skin) {

	for (auto& matrix : skin.matrices) {
		WriteFloat(pTagBuf->binary, matrix.x);
		WriteFloat(pTagBuf->binary, matrix.y);
		WriteFloat(pTagBuf->binary, matrix.z);
		WriteFloat(pTagBuf->binary, matrix.w);
	}

	for (auto& weight : skin.attributes) {
		WriteFloat(pTagBuf->binary, weight.x);
		WriteFloat(pTagBuf->binary, weight.y);
		WriteFloat(pTagBuf->binary, weight.z);
		WriteFloat(pTagBuf->binary, weight.w);
	}

}

void
WriteSkinPalette(TagBuffer* pTagBuf, CSimObj* pSimObj, std::vector<SimNode>& nodePalette)
{
	for (auto& node : nodePalette) {
		int index = FindNodeIndex(node.name, pSimObj->m_NodeTable);
		WriteUInt32(pTagBuf->binary, index);
	}
}

void
WriteSkin(TagBuffer* pTagBuf, MeshSkin skin, std::vector<SimNode>& nodePalette, const int& weights)
{
	for (auto& blendWeight : skin.blendWeights)
		for (int i = 0; i < weights; i++) {
			uint32_t totalWeights = blendWeight.numWeights;

			if (i >= totalWeights) {
				WriteUInt64(pTagBuf->binary, 0x0);
			}
			else
			{
				std::string boneName = blendWeight.bones.at(i);
				float weight = blendWeight.weights.at(i);
				uint32_t paletteIdx = FindNodeIndex(boneName, nodePalette);

				WriteUInt16(pTagBuf->binary, blendWeight.numWeights);
				WriteUInt16(pTagBuf->binary, paletteIdx);
				WriteFloat(pTagBuf->binary, weight);

			}
		}
}

int
GetConstraintIndex(StSimMesh* pSimMesh, const std::string& target) {
	int ctIdx = -1;
	for (int i = 0; i < pSimMesh->constraints.size(); i++) {
		std::string type = pSimMesh->constraints.at(i).name;

		if (type == target)
			return i;
	}

	return ctIdx;
}

void 
CClothEncoder::EncodeRootTag(TagBuffer* pTagBuf) {
	uint32_t numChildren = pTagBuf->children.size();

	WriteUInt32(pTagBuf->binary, numChildren);
	WriteUInt32(pTagBuf->binary, 0x10000);
	AlignTagHeader(pTagBuf);
}

void
CClothEncoder::EncodeSimMesh(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* sMesh = tag->pSimMesh;
	uint32_t numChildren = pTagBuf->children.size();
	uint32_t nameIdx = FindStringIndex(sMesh->sObjName, m_pSimObj->m_sStringTable);

	WriteUInt32(pTagBuf->binary, numChildren);
	WriteUInt32(pTagBuf->binary, nameIdx);
	WriteUInt32(pTagBuf->binary, sMesh->sSimVtxCount);
	WriteUInt32(pTagBuf->binary, sMesh->sIterationCount);
	WriteUInt32(pTagBuf->binary, sMesh->bCalcNormal);
	WriteUInt32(pTagBuf->binary, sMesh->bDispObject);
	AlignTagHeader(pTagBuf);
}

void
CClothEncoder::EncodeSubObj(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	uint32_t numChildren = pTagBuf->children.size();
	uint32_t sModelNameIdx = FindStringIndex(tag->pSimMesh->sModelName, m_pSimObj->m_sStringTable);
	uint32_t sObjNameIdx = FindStringIndex(tag->pSimMesh->sObjName, m_pSimObj->m_sStringTable);

	WriteUInt32(pTagBuf->binary, numChildren);
	WriteUInt32(pTagBuf->binary, sModelNameIdx);
	WriteUInt32(pTagBuf->binary, sObjNameIdx);
	AlignTagHeader(pTagBuf);
}

void
CClothEncoder::EncodeSubObjVerts(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* sMesh = tag->pSimMesh;

	uint32_t numChildren = pTagBuf->children.size();
	WriteUInt32(pTagBuf->binary, numChildren);
	WriteUInt32(pTagBuf->binary, sMesh->sObjVerts.size());
	AlignTagHeader(pTagBuf);

	for (auto& vertIdx : sMesh->sObjVerts) {
		WriteUInt32(pTagBuf->binary, vertIdx);
	}

	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeSimVerts(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* sMesh = tag->pSimMesh;

	uint32_t unkVal0 = 0x1;
	uint32_t sObjNameIdx = FindStringIndex(sMesh->sObjName, m_pSimObj->m_sStringTable);
	uint32_t numSimVerts = tag->pSimMesh->simVerts.size();

	WriteUInt32(pTagBuf->binary, unkVal0);
	WriteUInt32(pTagBuf->binary, sObjNameIdx);
	WriteUInt32(pTagBuf->binary, numSimVerts);
	AlignTagHeader(pTagBuf);

	for (auto& vertIdx : sMesh->simVerts) {
		WriteUInt32(pTagBuf->binary, 0x0);
		WriteUInt32(pTagBuf->binary, vertIdx);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeRecalcNormals(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* sMesh = tag->pSimMesh;

	uint32_t numChildren = pTagBuf->children.size();
	WriteUInt32(pTagBuf->binary, numChildren);

	for (auto& param : sMesh->rcn.parameters) {
		WriteUInt32(pTagBuf->binary, param);
	}

	AlignTagHeader(pTagBuf);
}


void
CClothEncoder::EncodeRCNData(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* sMesh = tag->pSimMesh;

	uint32_t numChildren = pTagBuf->children.size();
	uint32_t numIdxs = sMesh->rcn.degCounts.size();
	uint32_t numVtxs = sMesh->rcn.linkCounts.size();

	/* Write tag metadata */
	WriteUInt32(pTagBuf->binary, numChildren);
	WriteUInt32(pTagBuf->binary, numIdxs);
	WriteUInt32(pTagBuf->binary, numVtxs);

	/* Write index and vertex buffers */
	std::stringstream indexBuffer;
	std::stringstream vertexBuffer;
	WriteRCNIndexBuffer(indexBuffer, sMesh, numIdxs);
	WriteRCNNormalBuffer(vertexBuffer, sMesh, numVtxs);

	/* Update tag total sizes */
	WriteUInt32(pTagBuf->binary, indexBuffer.str().size());
	WriteUInt32(pTagBuf->binary, vertexBuffer.str().size());
	AlignTagHeader(pTagBuf);

	/* Append vertex and index buffers to tag binary */
	pTagBuf->binary.write(indexBuffer.str().data(), indexBuffer.str().size());
	pTagBuf->binary.write(vertexBuffer.str().data(), vertexBuffer.str().size());

	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeSimMeshSkin(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* sMesh = tag->pSimMesh;

	uint32_t numVerts = sMesh->sSimVtxCount;
	MeshSkin skin = sMesh->skin;
	int maxWeights = 0;

	/* Create a bone palette */
	std::vector<SimNode> nodePalette = GetNodePalette(skin, m_pSimObj, maxWeights);

	WriteUInt32(pTagBuf->binary, nodePalette.size());
	WriteUInt32(pTagBuf->binary, numVerts);
	WriteUInt32(pTagBuf->binary, maxWeights);
	WriteUInt32(pTagBuf->binary, 0x1);
	AlignTagHeader(pTagBuf);

	/* Write skin buffers */
	WriteNodeSkinMatrices(pTagBuf, skin);
	WriteSkinPalette(pTagBuf, m_pSimObj, nodePalette);
	WriteSkin(pTagBuf, skin, nodePalette, maxWeights);
	//WriteUInt32(pTagBuf->binary, 0x0);

	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeSimLinkSource(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* sMesh = tag->pSimMesh;

	uint32_t numTriangles = sMesh->sourceEdges.size();
	WriteUInt32(pTagBuf->binary, numTriangles);
	WriteUInt32(pTagBuf->binary, 0x0);
	AlignTagHeader(pTagBuf);

	for (auto& triangle : sMesh->sourceEdges) {
		WriteUInt16(pTagBuf->binary, triangle.point0);
		WriteUInt16(pTagBuf->binary, triangle.point1);
		WriteUInt32(pTagBuf->binary, triangle.point2);
	}

	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}


void
CClothEncoder::EncodePatternEntry(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	uint32_t numChildren = pTagBuf->children.size();

	WriteUInt32(pTagBuf->binary, numChildren);

	if (!tag->pSimMesh->bIsSimLine)
	{
		WriteUInt32(pTagBuf->binary, -1);
	}
	else {
		WriteUInt32(pTagBuf->binary, 0);
	}

	AlignTagHeader(pTagBuf);
}

void
CClothEncoder::EncodeStacks(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	uint32_t numChildren = pTagBuf->children.size();
	WriteUInt32(pTagBuf->binary, numChildren);
}

int
FindMatrixIndex(StSimMesh* pSimMesh, const Vector4& target) {

	int numMatrices = pSimMesh->skin.matrices.size();
	for (int i = 0; i < numMatrices; i++) {
		Vector4 skinVtx = pSimMesh->skin.matrices.at(i);
		bool isMatching = false;

		if (skinVtx == target) 
			return i;
	}

	return -1;
}

void
CClothEncoder::EncodeSkinCalc(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	uint32_t numVerts = tag->pSimMesh->skinCalc.size();

	WriteUInt32(pTagBuf->binary, numVerts);

    for (auto& vtxMat : tag->pSimMesh->skinCalc) {
        uint32_t matrixIndex = FindMatrixIndex(tag->pSimMesh, vtxMat);
		WriteUInt32(pTagBuf->binary, matrixIndex);
	}

	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeSkinPaste(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	uint32_t numVerts = tag->pSimMesh->skinPaste.size();
	WriteUInt32(pTagBuf->binary, numVerts);

	for (auto& Idx : tag->pSimMesh->skinPaste) {
		WriteUInt32(pTagBuf->binary, Idx);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeVertexSave(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	uint32_t numVerts = tag->pSimMesh->saveVerts.size();
	WriteUInt32(pTagBuf->binary, numVerts);

	for (auto& Idx : tag->pSimMesh->saveVerts) {
		WriteUInt32(pTagBuf->binary, Idx);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeForceField(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	uint32_t numVerts = pSimMesh->force.data.size();
	WriteUInt32(pTagBuf->binary, numVerts);

	for (auto& param : pSimMesh->force.parameters) {
		WriteFloat(pTagBuf->binary, param);
	}
	WriteUInt32(pTagBuf->binary, 0x0);
	AlignTagHeader(pTagBuf);

	for (auto& data : pSimMesh->force.data) {
		WriteUInt32(pTagBuf->binary, data.index);
		WriteFloat(pTagBuf->binary, data.weight);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeConstraint_Stretch(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	int ctIdx = GetConstraintIndex(pSimMesh,"Stretch");
	if (ctIdx == -1) return;

	SimConstraint stretchConstraint = pSimMesh->constraints.at(ctIdx);
	uint32_t numLinks = stretchConstraint.data.size();

	WriteUInt32(pTagBuf->binary, numLinks);
	AlignTagHeader(pTagBuf);

	for (auto& link : stretchConstraint.data) {
		WriteUInt32(pTagBuf->binary, link.vertices.x.index);
		WriteUInt32(pTagBuf->binary, link.vertices.y.index);
		WriteFloat(pTagBuf->binary, link.vertices.x.weight);
		WriteFloat(pTagBuf->binary, link.vertices.y.weight);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeConstraint_Std(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	int ctIdx = GetConstraintIndex(pSimMesh, "Standard");
	if (ctIdx == -1) return;

	SimConstraint constraint = pSimMesh->constraints.at(ctIdx);
	uint32_t numLinks = constraint.data.size();

	WriteUInt32(pTagBuf->binary, numLinks);
	WriteUInt32(pTagBuf->binary, constraint.parameter);
	AlignTagHeader(pTagBuf);

	for (auto& link : constraint.data) {
		WriteUInt16(pTagBuf->binary, link.vertices.x.index);
		WriteUInt16(pTagBuf->binary, link.vertices.y.index);
		WriteFloat(pTagBuf->binary, link.vertices.x.weight);
		WriteFloat(pTagBuf->binary, link.vertices.y.weight);
		WriteFloat(pTagBuf->binary, link.vertices.z.weight);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeConstraint_Bend(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	int ctIdx = GetConstraintIndex(pSimMesh, "Bend");
	if (ctIdx == -1) return;

	SimConstraint constraint = pSimMesh->constraints.at(ctIdx);
	uint32_t numLinks = constraint.data.size();

	WriteUInt32(pTagBuf->binary, numLinks);
	WriteUInt32(pTagBuf->binary, constraint.parameter);
	AlignTagHeader(pTagBuf);

	for (auto& link : constraint.data) {
		WriteUInt16(pTagBuf->binary, link.vertices.x.index);
		WriteUInt16(pTagBuf->binary, link.vertices.y.index);
		WriteFloat(pTagBuf->binary, link.vertices.x.weight);
		WriteFloat(pTagBuf->binary, link.vertices.y.weight);
		WriteFloat(pTagBuf->binary, link.vertices.z.weight);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeConstraint_Fixation(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	int ctIdx = GetConstraintIndex(pSimMesh, "Fixation");
	if (ctIdx == -1) return;

	SimConstraint constraint = pSimMesh->constraints.at(ctIdx);
	uint32_t numLinks = constraint.data.size();

	WriteUInt32(pTagBuf->binary, numLinks);
	AlignTagHeader(pTagBuf);

	for (auto& link : constraint.data) {
		WriteUInt32(pTagBuf->binary, link.vertices.x.index);
		WriteFloat(pTagBuf->binary, link.vertices.x.weight);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeBendStiffness(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	int ctIdx = GetConstraintIndex(pSimMesh, "BendStiffness");
	if (ctIdx == -1) return;

	SimConstraint constraint = pSimMesh->constraints.at(ctIdx);
	uint32_t numLinks = constraint.faceData.size();

	WriteUInt32(pTagBuf->binary, numLinks);
	WriteUInt32(pTagBuf->binary, constraint.parameter);
	WriteFloat(pTagBuf->binary, 1.0f);
	AlignTagHeader(pTagBuf);

	for (auto& link : constraint.faceData) {
		WriteUInt16(pTagBuf->binary, link.x.index);
		WriteUInt16(pTagBuf->binary, link.y.index);
		WriteUInt32(pTagBuf->binary, link.z.index);

		WriteFloat(pTagBuf->binary, link.weights.at(0));
		WriteFloat(pTagBuf->binary, link.weights.at(1));
		WriteFloat(pTagBuf->binary, link.weights.at(2));
		WriteFloat(pTagBuf->binary, link.weights.at(3));
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void 
WriteColVertHeader(TagBuffer* pTagBuf, StSimMesh* pSimMesh) {
	int32_t unkFlagA = pSimMesh->colVtx.unkFlag;
	int32_t unkFlagB = pSimMesh->colVtx.unkFlagB;
	int32_t unkVal = pSimMesh->colVtx.unkVal;
	int32_t numItems = pSimMesh->colVtx.numItems;
	int32_t numVerts = pSimMesh->colVtx.numVerts;

	WriteInt32(pTagBuf->binary, pSimMesh->colVtx.unkFlag);
	for (int i = 0 ; i < 4; i++)
		WriteUInt64(pTagBuf->binary, 0x0);

	WriteInt32(pTagBuf->binary, pSimMesh->colVtx.unkVal);
	WriteInt32(pTagBuf->binary, pSimMesh->colVtx.numItems);
	WriteInt32(pTagBuf->binary, pSimMesh->colVtx.numVerts);
	WriteInt32(pTagBuf->binary, pSimMesh->colVtx.unkFlagB);

	uint32_t numPads = (pSimMesh->colVtx.tagSize != 0x50) ? 4 : 2;
	for (int i = 0; i < numPads; i++)
		WriteUInt64(pTagBuf->binary, 0x0);

	AlignTagHeader(pTagBuf);
}

void
CClothEncoder::EncodeColVerts(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;
	WriteColVertHeader(pTagBuf, pSimMesh);

	for (auto& point : pSimMesh->colVtx.items) {
		WriteUInt32(pTagBuf->binary, point.x.index);
		WriteUInt32(pTagBuf->binary, point.y.index);

		WriteFloat(pTagBuf->binary, point.x.weight);
		WriteFloat(pTagBuf->binary, point.y.weight);
	}

	for (auto& point : pSimMesh->colVtx.indices) {
		WriteUInt32(pTagBuf->binary, point);
	}

	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeSimLine(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;
	
	uint32_t nameIdx = FindStringIndex(pSimMesh->sModelName, m_pSimObj->m_sStringTable);
	WriteUInt32(pTagBuf->binary, pSimMesh->numTags);
	WriteUInt32(pTagBuf->binary, nameIdx);
	WriteUInt32(pTagBuf->binary, pSimMesh->sSimVtxCount);
	WriteUInt32(pTagBuf->binary, pSimMesh->sIterationCount);
	AlignTagHeader(pTagBuf);

}


int
FindNodeMatrixIndex(const SimNode& sTarget, std::vector<SimNode> nodeTable)
{
	for (int i = 0; i < nodeTable.size(); i++) {
		SimNode nodeItem = nodeTable.at(i);

		if (sTarget.name == nodeItem.name &&
			sTarget.vecf == nodeItem.vecf) {
			return i;
		}
	}

	return -1;
}


void
CClothEncoder::EncodeLineDef(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	uint32_t numLines = pSimMesh->lines.sSize;
	WriteUInt32(pTagBuf->binary, numLines);
	AlignTagHeader(pTagBuf);

	for (int i = 0; i < numLines; i++) {
		NodeLink link = pSimMesh->lines.links.at(i);
		SimNode beginNode = link.link.at(0);
		SimNode endNode = link.link.back();

		uint32_t beginIndex = FindNodeIndex(beginNode.name, pSimMesh->nodePalette);
		uint32_t endIndex   = FindNodeMatrixIndex(endNode, pSimMesh->nodePalette);

		WriteUInt32(pTagBuf->binary, i);
		WriteUInt32(pTagBuf->binary, beginIndex);
		WriteUInt32(pTagBuf->binary, endIndex);
	}

	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeAssignNode(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	uint32_t numNodes = pSimMesh->nodePalette.size();

	WriteUInt32(pTagBuf->binary, numNodes);
	AlignTagHeader(pTagBuf);

	for (auto& node : pSimMesh->nodePalette) {

		WriteFloat(pTagBuf->binary, node.vecf.x);
		WriteFloat(pTagBuf->binary, node.vecf.y);
		WriteFloat(pTagBuf->binary, node.vecf.z);
		uint32_t nodeIdx = FindNodeIndex(node.name, m_pSimObj->m_NodeTable);

		WriteUInt32(pTagBuf->binary, nodeIdx);
	}

	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeLinkTar(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	LinkTarget target = pSimMesh->target;
	pTagBuf->binary.write(target.source.c_str(), target.source.size() + 1);

	while (pTagBuf->binary.str().size() < 0x80) 
	{
		WriteByte(pTagBuf->binary, 0);
	}

	WriteUInt32(pTagBuf->binary, target.indices.size());
	WriteUInt32(pTagBuf->binary, target.palette.size());
	WriteUInt32(pTagBuf->binary, target.totalWeights);
	AlignTagHeader(pTagBuf);

	for (auto& idx : target.indices) {
		WriteUInt32(pTagBuf->binary, idx);
	}

	for (auto& idx : target.palette) {
		WriteUInt32(pTagBuf->binary, idx);
	}

	for (auto& influence : target.weights)
		for (int i = 0; i < target.totalWeights; i++) 
		{
			uint32_t totalWeights = influence.weights.size();

			if (i > totalWeights) {
				WriteUInt64(pTagBuf->binary, 0x0);
			}
			else
			{
				float value = influence.weights.at(i);
				uint32_t paletteIdx = influence.indices.at(i);

				WriteUInt16(pTagBuf->binary, totalWeights);
				WriteUInt16(pTagBuf->binary, paletteIdx);
				WriteFloat(pTagBuf->binary, value);
			}
		}

	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void
CClothEncoder::EncodeStringTable(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;

	uint32_t numChildren = tag->children.size();
	uint32_t numStrings = m_pSimObj->m_sStringTable.size();

	WriteUInt32(pTagBuf->binary, numChildren);
	WriteUInt32(pTagBuf->binary, numStrings);
	AlignTagHeader(pTagBuf);
}

void
CClothEncoder::EncodeNodeTable(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;

	uint32_t numChildren = tag->children.size();
	uint32_t numNodes = m_pSimObj->m_NodeTable.size();

	WriteUInt32(pTagBuf->binary, numChildren);
	WriteUInt32(pTagBuf->binary, numNodes);
	AlignTagHeader(pTagBuf);

	/* Create child node objects */
	tag->children.clear();
	for (auto& node : m_pSimObj->m_NodeTable) {
		StTag* child = new StTag;
		child->eType = enTagType_String;
		child->pParent = tag;
		child->sTagName = node.name;
		tag->children.push_back(child);
	}

}


void
CClothEncoder::EncodeString(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;
	AlignTagHeader(pTagBuf);

	std::string str = tag->sTagName;
	pTagBuf->binary.write(str.c_str(), str.size() + 1);
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void 
CClothEncoder::EncodeColIdTbl(TagBuffer* pTag) {
	StTag* tag = pTag->tag;

	WriteUInt32(pTag->binary, tag->children.size() );
	WriteUInt32(pTag->binary, tag->children.size());
	AlignTagHeader(pTag);
}

void 
CClothEncoder::EncodeCols(TagBuffer* pTag) {
	StTag* tag = pTag->tag;
	uint32_t numChilds = tag->children.size();
	WriteUInt32(pTag->binary, numChilds);
	WriteUInt32(pTag->binary, -1);

	for (int i = 0; i < 4; i++)
		WriteUInt64(pTag->binary, 0x0);
	AlignTagHeader(pTag);
}

void 
CClothEncoder::EncodeColPack(TagBuffer* pTag) {
	StTag* tag = pTag->tag;
	uint32_t numChilds = tag->children.size();

	WriteUInt32(pTag->binary, numChilds);
	WriteUInt32(pTag->binary, 0);
	WriteUInt32(pTag->binary, tag->children.front()->eType);
	AlignTagHeader(pTag);
}
void 
CClothEncoder::EncodeCapsuleStandard(TagBuffer* pTag) {
	StTag* tag = pTag->tag;
	CollisionVolume col = tag->col;

	WriteUInt32(pTag->binary, FindStringIndex(col.name, m_pSimObj->m_sStringTable));
	WriteUInt32(pTag->binary, FindNodeIndex(col.joint, m_pSimObj->m_NodeTable));

	for (auto& weight : col.weights) {
		WriteFloat(pTag->binary, weight);
	}

	WriteUInt32(pTag->binary, col.id);

	for (int i = 0; i < 4; i++)
		WriteUInt64(pTag->binary, 0x0);

	AlignTagHeader(pTag);
}

void 
CClothEncoder::EncodeCapsuleTapered(TagBuffer* pTag) {
	StTag* tag = pTag->tag;
	CollisionVolume col = tag->col;

	WriteUInt32(pTag->binary, FindStringIndex(col.name, m_pSimObj->m_sStringTable));
	WriteUInt32(pTag->binary, FindNodeIndex(col.joint, m_pSimObj->m_NodeTable));

	for (auto& weight : col.weights) {
		WriteFloat(pTag->binary, weight);
	}
	
	WriteFloat(pTag->binary, col.radius);
	WriteUInt32(pTag->binary, col.id);

	for (int i = 0; i < 6; i++)
		WriteUInt64(pTag->binary, 0x0);

	AlignTagHeader(pTag);
}


void
CClothEncoder::EncodeColIdInfo(TagBuffer* pTag) {
	StTag* tag = pTag->tag;
	uint32_t numChilds = tag->children.size();

	WriteUInt32(pTag->binary, numChilds);
	WriteUInt32(pTag->binary, 0x0);
	WriteUInt32(pTag->binary, 0x1);
	WriteUInt32(pTag->binary, 0x1);
	AlignTagHeader(pTag);

	WriteUInt64(pTag->binary, 0x0);
	//WriteUInt64(pTag->binary, 0x0);

	AlignStream(pTag->binary, BINARY_ALIGNMENT, 0xC);
}
