#include "encode_def.h"
#include "cgamecloth.h"
#include "memoryreader.h"
#include "clothencoder.h"

using namespace clothsrl;

CSimEncodeDef::CSimEncodeDef(std::shared_ptr<CSimObj> obj, std::ofstream*& data) :
	m_pSimObj(obj),
	m_data(data)
{}

void CSimEncodeDef::encodeSimMesh(TagBuffer* pTagBuf) 
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* sMesh = tag->pSimMesh;
	uint32_t numChildren = pTagBuf->children.size();
	uint32_t nameIdx = FindStringIndex(sMesh->sObjName, m_pSimObj->stringTable());

	WriteUInt32(pTagBuf->binary, numChildren);
	WriteUInt32(pTagBuf->binary, nameIdx);
	WriteUInt32(pTagBuf->binary, sMesh->sSimVtxCount);
	WriteUInt32(pTagBuf->binary, sMesh->sIterationCount);
	WriteUInt32(pTagBuf->binary, sMesh->bCalcNormal);
	WriteUInt32(pTagBuf->binary, sMesh->bDispObject);
	AlignTagHeader(pTagBuf);
}

void CSimEncodeDef::encodeSimLine(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;
	uint32_t numChildren = pTagBuf->children.size();

	uint32_t nameIdx = FindStringIndex(pSimMesh->sModelName, m_pSimObj->stringTable());
	WriteUInt32(pTagBuf->binary, numChildren);
	WriteUInt32(pTagBuf->binary, nameIdx);
	WriteUInt32(pTagBuf->binary, pSimMesh->sSimVtxCount);
	WriteUInt32(pTagBuf->binary, pSimMesh->sIterationCount);
	AlignTagHeader(pTagBuf);

}


void CSimEncodeDef::encodeAssignNode(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	uint32_t numNodes = pSimMesh->nodePalette.size();

	WriteUInt32(pTagBuf->binary, numNodes);
	AlignTagHeader(pTagBuf);

	for (auto& node : pSimMesh->nodePalette) {

		WriteFloat(pTagBuf->binary, node.vecf.x);
		WriteFloat(pTagBuf->binary, node.vecf.y);
		WriteFloat(pTagBuf->binary, node.vecf.z);
		uint32_t nodeIdx = FindNodeIndex(node.name, m_pSimObj->getNodes());

		WriteUInt32(pTagBuf->binary, nodeIdx);
	}

	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}


void CSimEncodeDef::encodeLineDef(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	uint32_t numLines = pSimMesh->lines.sSize;
	WriteUInt32(pTagBuf->binary, numLines);
	AlignTagHeader(pTagBuf);

	for (int i = 0; i < numLines; i++) {
		NodeLink link = pSimMesh->lines.links.at(i);
		SimNode beginNode = link.link.at(0);
		SimNode endNode = link.link.back();

		uint32_t beginIndex = FindNodeIndex(beginNode.name, pSimMesh->nodePalette);
		uint32_t endIndex = FindNodeMatrixIndex(endNode, pSimMesh->nodePalette);

		WriteUInt32(pTagBuf->binary, i);
		WriteUInt32(pTagBuf->binary, beginIndex);
		WriteUInt32(pTagBuf->binary, endIndex);
	}

	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void CSimEncodeDef::encodeRootTag(TagBuffer* pTagBuf) 
{
	uint32_t numChildren = pTagBuf->children.size();

	WriteUInt32(pTagBuf->binary, numChildren);
	WriteUInt32(pTagBuf->binary, 0x10000);
	AlignTagHeader(pTagBuf);
}


void CSimEncodeDef::encodeNodeTable(TagBuffer* pTagBuf) {
	StTag* tag = pTagBuf->tag;

	uint32_t numChildren = tag->children.size();
	uint32_t numNodes = m_pSimObj->getNodes().size();

	WriteUInt32(pTagBuf->binary, numChildren);
	WriteUInt32(pTagBuf->binary, numNodes);
	AlignTagHeader(pTagBuf);

	/* Create child node objects */
	tag->children.clear();
	for (auto& node : m_pSimObj->getNodes()) {
		StTag* child = new StTag;
		child->eType = enTagType_String;
		child->pParent = tag;
		child->sTagName = node.name;
		tag->children.push_back(child);
	}

}


void CSimEncodeDef::encodePasteNode(TagBuffer* pTagBuf)
{
	StTag* tag = pTagBuf->tag;
	uint32_t numChildren = tag->children.size();
    uint32_t pasteIndex = tag->copy_index;

	WriteUInt32(pTagBuf->binary, numChildren);
	WriteUInt32(pTagBuf->binary, pasteIndex);
	AlignTagHeader(pTagBuf);
}

