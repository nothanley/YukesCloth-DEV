#include "clothencoder.h"
#include "csimmesh.h"
#include "memoryreader.h"
#include <iostream>

using namespace clothsrl;

CClothEncoder::CClothEncoder(std::ofstream* outStream, std::shared_ptr<CSimObj>& pSimObj) :
	m_pDataStream(outStream),
	m_pSimObj(pSimObj)
{
	this->setupHandles();
}


void CClothEncoder::save()
{
	/* Setup header tag buffer */
	TagBuffer* pRootBuffer = new TagBuffer;
	pRootBuffer->tag = new StTag;
	*pRootBuffer->tag = *m_pSimObj->getRootTag();	

	/* Recursively create all child tag buffers */
	this->initTagBuffers(pRootBuffer);

	/* Serialize all datasets into formatted binaries */
	this->encodeAllTags(pRootBuffer);

	/* Merge datasets and write tree hierarchy to out file */
	writeTree(pRootBuffer);
}


uint32_t CClothEncoder::getTagTotalSize(TagBuffer* pTag)
{
	uint32_t sSize = pTag->binary.str().size() + 0xC;

	for (auto child : pTag->children) {
		sSize += getTagTotalSize(child);
	}

	return sSize;
}

void CClothEncoder::setupHandles()
{
	this->m_colHandler      = std::make_unique<CSimEncodeCol>        (m_pSimObj, m_pDataStream);
	this->m_constHandler    = std::make_unique<CSimEncodeConstraint> (m_pSimObj, m_pDataStream);
	this->m_defHandler      = std::make_unique<CSimEncodeDef>        (m_pSimObj, m_pDataStream);
	this->m_subobjHandler   = std::make_unique<CSimEncodeSubObj>     (m_pSimObj, m_pDataStream);
	this->m_patternHandler  = std::make_unique<CSimEncodePattern>    (m_pSimObj, m_pDataStream);
	this->m_skinHandler     = std::make_unique<CSimEncodeSkin>       (m_pSimObj, m_pDataStream);
	this->m_rcnHandler      = std::make_unique<CSimEncodeRCN>        (m_pSimObj, m_pDataStream);
}

void CClothEncoder::writeTagHead(TagBuffer* pTagBuf) 
{
	pTagBuf->sSize = getTagTotalSize(pTagBuf);
	//printf("\nSize of Tag: %d", pTagBuf->sSize);

	/* Write stream metadata */
	WriteUInt32(m_pDataStream, pTagBuf->tag->eType);
	WriteUInt32(m_pDataStream, pTagBuf->sBegin);
	WriteUInt32(m_pDataStream, pTagBuf->sSize);

	/* Write defined binary */
	m_pDataStream->write(pTagBuf->binary.str().data(), pTagBuf->binary.str().size());
}

void CClothEncoder::writeTree(TagBuffer* pTagBuf) 
{
	writeTagHead(pTagBuf);

	for (auto& tag : pTagBuf->children) {
		writeTree(tag);
	}
}

void CClothEncoder::initTagBuffers(TagBuffer* pParentBuf)
{
	for (auto& tag : pParentBuf->tag->children) {

		/* Generate new string and node tables */
		RegenerateStringTable(tag, m_pSimObj);
		RegenerateNodeTable(tag, m_pSimObj);

		/* Create TagBuffer */
		TagBuffer* pChildBuf = new TagBuffer;
		pChildBuf->tag = tag;

		initTagBuffers(pChildBuf);
		pParentBuf->children.push_back(pChildBuf);
	}
}

void CClothEncoder::encodeAllTags(TagBuffer* pRootNode) 
{
	encodeTag(pRootNode);

	for (auto &tag : pRootNode->children) {
		encodeAllTags(tag);
	}
}

void
CClothEncoder::encodeTag(TagBuffer* pTagBuf) 
{
	StTag* pTag = pTagBuf->tag;

	if (pTag->eType != 0x18) {
		pTag->sTagName = yclutils::GetNodeName(pTag->eType);
	}

	switch (pTag->eType) {
		case enTagType_Root:
			m_defHandler->encodeRootTag(pTagBuf);
			break;
		case enTagType_SimMesh:
			m_defHandler->encodeSimMesh(pTagBuf);
			break;
		case enTagType_SimMesh_AssignSubObj:
			m_subobjHandler->encodeSubObj(pTagBuf);
			break;
		case enTagType_SimMesh_AssignSubObjVtx:
			m_subobjHandler->encodeSubObjVtx(pTagBuf);
			break;
		case enTagType_SimMesh_AssignSimVtx:
			m_subobjHandler->encodeSimVtx(pTagBuf);
			break;
		case enTagType_SimLine_AssignNode:
			m_defHandler->encodeAssignNode(pTagBuf);
			break;
		case enTagType_SimMesh_RCN:
			m_rcnHandler->encodeRecalcNormals(pTagBuf);
			break;
		case enTagType_SimMesh_RCNSubObj:
			m_rcnHandler->encodeRCNData(pTagBuf);
			break;
		case enTagType_SimMesh_Skin:
			m_skinHandler->encodeSimMeshSkin(pTagBuf);
			break;
		case enTagType_SimMesh_SimLinkSrc:
			m_subobjHandler->encodeSimLinkSource(pTagBuf);
			break;
		case enTagType_SimMesh_Pattern:
			m_patternHandler->encodeSimMeshPattern(pTagBuf);
			break;
		case enTagType_SimMesh_Stacks:
			m_patternHandler->encodeSimMeshStacks(pTagBuf);
			break;
		case enTagType_SimMesh_SkinCalc:
			m_skinHandler->encodeSkinCalc(pTagBuf);
			break;
		case enTagType_SimMesh_SkinPaste:
			m_skinHandler->encodeSkinPaste(pTagBuf);
			break;
		case enTagType_SimMesh_OldVtxSave:
			m_subobjHandler->encodeSaveVtx(pTagBuf);
			break;
		case enTagType_SimMesh_Force:
			m_constHandler->encodeForce(pTagBuf);
			break;
		case enTagType_SimMesh_CtStretchLink:
			m_constHandler->encodeConstraintStretchLink(pTagBuf);
			break;
		case enTagType_SimMesh_CtStdLink:
			m_constHandler->encodeConstraintStandardLink(pTagBuf);
			break;
		case enTagType_SimMesh_CtBendLink:
			m_constHandler->encodeConstraintBendLink(pTagBuf);
			break;
		case enTagType_SimMesh_BendingStiffness:
			m_constHandler->encodeBendStiffness(pTagBuf);
			break;
		case enTagType_SimMesh_ColVtx:
			m_colHandler->encodeCollisionVerts(pTagBuf);
			break;
		case enTagType_SimMesh_CtFixation:
			m_constHandler->encodeConstraintFixation(pTagBuf);
			break;
		case enTagType_SimLine:
			m_defHandler->encodeSimLine(pTagBuf);
			break;
		case enTagType_SimLine_LineDef:
			m_defHandler->encodeLineDef(pTagBuf);
			break;
		case enTagType_SimMesh_SimLinkTar:
			m_subobjHandler->encodeLinkTar(pTagBuf);
			break;
		case enTagType_StrTbl:
			encodeStringTable(pTagBuf);
			break;
		case enTagType_String:
			encodeString(pTagBuf);
			break;
		case enTagType_NodeTbl:
			m_defHandler->encodeNodeTable(pTagBuf);
			break;
		case enTagType_ColIDTbl:
			m_colHandler->encodeColIDTbl(pTagBuf);
			break;
		case enTagType_Cols:
			m_colHandler->encodeCols(pTagBuf);
			break;
		case enTagType_ColPack:
			m_colHandler->encodeColPack(pTagBuf);
			break;
		case enTagType_Capsule_Standard:
			m_colHandler->encodeCapsuleStandard(pTagBuf);
			break;
		case enTagType_Capsule_Tapered:
			m_colHandler->encodeCapsuleTapered(pTagBuf);
			break;
		case enTagType_ColIDInfo:
			m_colHandler->encodeColIdInfo(pTagBuf);
			break;
		case enTagType_SimMesh_Collection:
			//todo
			break;
		default:
			std::cerr << "Could not resolve encode format for tag type: " << pTag->eType << ". Skipping node..." << endl;
			pTagBuf->bUsePreDef = true;
			break;
	}

}


void CClothEncoder::encodeStringTable(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;

	uint32_t numChildren = tag->children.size();
	uint32_t numStrings = m_pSimObj->stringTable().size();

	WriteUInt32(pTagBuf->binary, numChildren);
	WriteUInt32(pTagBuf->binary, numStrings);
	AlignTagHeader(pTagBuf);
}


void CClothEncoder::encodeString(TagBuffer* pTagBuf) 
{
	const StTag* tag = pTagBuf->tag;
	AlignTagHeader(pTagBuf);

	std::string str = tag->sTagName;
	pTagBuf->binary.write(str.c_str(), str.size() + 1);
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

