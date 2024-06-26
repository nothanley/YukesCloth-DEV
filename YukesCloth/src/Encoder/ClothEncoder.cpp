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
	this->checkFileFormat();

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

static bool hasCollectionStack(const StTag* tag)
{
	for (auto& child : tag->children)
		if (child->eType == enTagType_SimMesh_Collection)
			return true;

	return false;
}

static void clearDeprecatedSkinDataNode(StTag* root)
{
	std::vector<StTag*> filtered_childs;

	for (auto& node : root->children)
	{
		if (node->eType != enTagType_SimMesh_Skin)
			filtered_childs.push_back(node);
	}
	root->children = filtered_childs;
}

static void makeRootCollectionTag(StTag* root)
{
	StTag* collectionTag = new StTag;
	collectionTag->eType = enTagType_SimMesh_Collection;
	collectionTag->pParent = root;
	collectionTag->children.clear();
	collectionTag->pSimMesh = root->pSimMesh;

	for (auto& child : root->children) {
		collectionTag->children.push_back(child);
	}

	root->children = std::vector<StTag*>{ collectionTag };
}

static std::vector<StTag*> getAllPatternStacks(const StTag* root)
{
	std::vector<StTag*> tags;

	for (auto& tag : root->children) {
		if (tag->eType == enTagType_SimMesh_Pattern) {
			tags.push_back(tag);
		}
	}

	return tags;
}

static void makeCopyNode(StTag* parent, StTag* sourceNode, const int index)
{
	StTag* copyNodeTag       = new StTag;
	copyNodeTag->eType       = enTagType_Copy_Node;
	copyNodeTag->pParent     = parent;
	copyNodeTag->pSimMesh    = sourceNode->pSimMesh;
	copyNodeTag->copy_index  = sourceNode->eType;
	
	parent->children.at(index) = copyNodeTag;
	delete sourceNode;
}

static void reformatPatternNodeCopy(StTag* root)
{
	auto stackTag = (!root->children.empty()) ? root->children.front() : nullptr;
	if (!stackTag || (stackTag->eType != enTagType_SimMesh_Stacks))
		return;

	int numStackProps = stackTag->children.size();
	for (int i = 0; i < numStackProps; i++)
	{
		auto propTag = stackTag->children.at(i);
		bool isLegalNode = (propTag->eType == enTagType_Copy_Node) || (propTag->eType == enTagType_SimMesh_ColVtx);

		if (!isLegalNode)
			makeCopyNode(stackTag, propTag, i);
	}
}

void CClothEncoder::retargetPatternCopy(StTag* root)
{
	auto tags = getAllPatternStacks(root);
	if (tags.size() < 2)
		return;

	for (int i = 1; i < tags.size(); i++)
	{
		::reformatPatternNodeCopy(tags.at(i));
	}
}

void CClothEncoder::retargetNodeTree_2024(StTag* root)
{
	bool isSimObj = (root->eType == enTagType_SimMesh || root->eType == enTagType_SimLine);
	bool usingCollection = (isSimObj) ? (!root->children.empty() && hasCollectionStack(root)) : false;

	if (isSimObj) {
		//::clearDeprecatedSkinDataNode(root);
		this->retargetPatternCopy(root);
	}

	if (isSimObj && !usingCollection){
		::makeRootCollectionTag(root);
	}
	
	for (auto& child : root->children) {
		this->retargetNodeTree_2024(child);
	}
}

void CClothEncoder::checkFileFormat()
{
	StTag* new_col_tag_24 = m_pSimObj->FindTag(enTagType_SimMesh_Collection);
	m_version = (new_col_tag_24) ? YUKES_CLOTH_24 : m_version;

	if (m_compileTarget == YUKES_CLOTH_24)
	{
		this->retargetNodeTree_2024(const_cast<StTag*>(m_pSimObj->getRootTag()));
	}
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
		
		this->initTagBuffers(pChildBuf);

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
			m_colHandler->encodeCollisionVerts(pTagBuf, pTagBuf->tag, pTagBuf->tag->pSimMesh);
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
			if (m_version == YUKES_CLOTH_24)
			{
				m_subobjHandler->encodeLinkTar_2024(pTagBuf);
			}
			else {
				m_subobjHandler->encodeLinkTar(pTagBuf);
			}
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
			m_subobjHandler->encodeCollection(pTagBuf);
			this->m_version = YUKES_CLOTH_24;
			break;
		case enTagType_Copy_Node:
			m_defHandler->encodePasteNode(pTagBuf);
			break;
		default:
			std::cerr << "\t\t{YCL Log} Could not resolve encode format for tag type: " << pTag->eType << ". Skipping node..." << endl;
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

