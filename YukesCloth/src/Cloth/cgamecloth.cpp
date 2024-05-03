#include "CGameCloth.h"
#include "ClothStructs.h"
#include "memoryreader.h"

static const std::vector<int> ROOT_NODES
{
	1,2,5,6,9,19,23,25,27,31,32,
	/* 2024 tags: */ 0x3E 
};

CGameCloth::CGameCloth(char* data)
{
	this->m_data = data;
	this->m_pStHead = nullptr;
	this->setupSimHandles();
}

void CGameCloth::setupSimHandles()
{
	this->m_colHandler      = std::make_unique<CSimMeshCol>        (this, m_data);
	this->m_constHandler    = std::make_unique<CSimMeshConstraint> (this, m_data);
	this->m_defHandler      = std::make_unique<CSimMeshDef>        (this, m_data);
	this->m_subobjHandler   = std::make_unique<CSimMeshSubObj>     (this, m_data);
	this->m_patternHandler  = std::make_unique<CSimMeshPattern>    (this, m_data);
	this->m_skinHandler     = std::make_unique<CSimMeshSkin>       (this, m_data);
	this->m_rcnHandler      = std::make_unique<CSimSubObj_RCN>     (this, m_data);
}

void CGameCloth::load() 
{
	/* Initialize Tag Header */
	this->pos = (uintptr_t)m_data;
	this->m_pStHead = new StTag{ u32, u32, u32, u32 };

#ifdef DEBUG_CONSOLE
	printf("\n====== StHead {sTagCount:%d, eType:%d, sSize:%d, sTotalSize:%d} ======\n",
		m_pStHead->sTagCount, m_pStHead->eType, m_pStHead->sSize, m_pStHead->sTotalSize);
#endif

	/* Iterate and collect all child nodes */
	this->pos += m_pStHead->sSize;
	for (int i = 0; i < m_pStHead->sTagCount; i++) {
		getTag(m_pStHead);
	}

	/* Setup node and string tables */
	initTag( FindTag(enTagType_NodeTbl) );
	initTag( FindTag(enTagType_StrTbl) );

	/* Initialize all other tags */
	setupTags(m_pStHead);
	updateStrings();
};


void CGameCloth::updateStrings()
{
	for (auto child : m_pStHead->children) 
	{
		uint32_t type = child->eType;

		if (type == enTagType_SimMesh) {
			child->pSimMesh->sModelName = getString(child->pSimMesh->sMeshIndex);
			child->pSimMesh->sObjName = getString(child->pSimMesh->sObjIndex);
		}
		else if (type == enTagType_SimLine) {
			child->pSimMesh->sModelName = getString(child->pSimMesh->sNameIdx);
			child->pSimMesh->sObjName = "Line";
		}
	}
}


void CGameCloth::setupTags(StTag* pParentTag) 
{
	initTag(pParentTag);

	/* Setup and LOAD all child nodes */
	for (int i = 0; i < pParentTag->children.size(); i++) 
	{
		if (pParentTag->pSimMesh)
			pParentTag->children.at(i)->pSimMesh = pParentTag->pSimMesh;

		setupTags(pParentTag->children.at(i));
	}

}

StTag* CGameCloth::getTag(StTag* pParentTag) 
{
	this->m_data = (char*)pos;
	StTag* stDataTag = new StTag{ u32,u32,u32 };
	stDataTag->streamPointer = (uintptr_t)pos;

#ifdef DEBUG_CONSOLE
	printf("\n[0x%x]\t- StTag {eType:%d, sSize:%d, sTotalSize:%d}\n",
		pos, stDataTag->eType, stDataTag->sSize, stDataTag->sTotalSize);
#endif

	/* Add node to parent vector */
	stDataTag->pParent = pParentTag;
	pParentTag->children.push_back(stDataTag);

	/* Collect all child nodes */
	uint32_t numNodes = yclutils::hasIndex(ROOT_NODES, stDataTag->eType) ? u32 : 0;
	this->pos += (numNodes == 0) ? stDataTag->sTotalSize : stDataTag->sSize;

	for (uint32_t i = 0; i < numNodes; i++) {
		StTag* childNode = getTag(stDataTag);
	}

	return stDataTag;
}

#ifdef DEBUG_EDITOR
void
SaveTagBinary(std::ifstream* fs, uintptr_t address, StTag* pTag) {
	fs->seekg(address);
	pTag->data.resize(pTag->sTotalSize);
	fs->read(reinterpret_cast<char*>(pTag->data.data()), pTag->sTotalSize);
	fs->seekg(pTag->streamPointer);
}
#endif

void CGameCloth::initTag(StTag* tag) 
{
	if (!tag || tag->eType == 0x0) { return; }
	this->pos    = tag->streamPointer;
	this->m_data = (char*)pos + 0xC;

#ifdef DEBUG_EDITOR
	SaveTagBinary(m_pDataStream, m_iStreamPos, &tag);
#endif

	switch (tag->eType) {
		case enTagType_SimMesh_Collection:
			this->game_format = YUKES_CLOTH_24;
			break;
		case enTagType_SimMesh:
			m_defHandler->loadSimMesh(*tag);
			break;
		case enTagType_SimMesh_AssignSubObj:
			m_subobjHandler->AssignSubObj(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_AssignSubObjVtx:
			m_subobjHandler->AssignSubObjVtx(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_AssignSimVtx:
			m_subobjHandler->AssignSimVtx(*tag->pSimMesh);
			break;
		case enTagType_SimLine_AssignNode:
			if (tag->pSimMesh->nodePalette.size() > 0) { break; }
			m_defHandler->AssignNode(*tag->pParent->pSimMesh);
			break;
		case enTagType_SimMesh_RCN:
			m_rcnHandler->loadRCNData(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_RCNSubObj:
			m_rcnHandler->loadRecalcNormals(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_Skin: // todo: look this over for 2024 format
			m_skinHandler->loadSkinData(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_SimLinkSrc:
			m_subobjHandler->Link_DefineSourceMesh(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_Pattern:
			m_patternHandler->loadSimMeshPattern(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_Stacks:
			m_patternHandler->loadSimMeshStacks(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_SkinCalc:
			m_skinHandler->loadSkinCalc(*tag->pSimMesh); //
			break;
		case enTagType_SimMesh_SkinPaste:
			m_skinHandler->loadSkinPaste(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_OldVtxSave:
			m_subobjHandler->SaveOldVtxs(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_Force:
			m_constHandler->loadForce(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_CtStretchLink:
			m_constHandler->loadConstraintStretchLink(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_CtStdLink:
			m_constHandler->loadConstraintStandardLink(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_CtBendLink:
			m_constHandler->loadConstraintBendLink(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_BendingStiffness:
			m_constHandler->loadBendStiffness(*tag->pSimMesh);
			break;
		case enTagType_SimMesh_CtFixation:
			m_constHandler->loadConstraintFixation(*tag->pSimMesh);
			break;
		case enTagType_SimLine:
			m_defHandler->loadSimLine(*tag);
			break;
		case enTagType_SimLine_LineDef:
			m_defHandler->InitializeNodePalette(*tag);
			m_defHandler->loadLineDef(*tag->pSimMesh);
			break;
		case enTagType_StrTbl:
			if (!m_stringTable.empty()) { break; }
			this->loadStringTable();
			break;
		case enTagType_String:
			tag->sTagName = this->loadString();
			break;
		case enTagType_NodeTbl:
			if (!m_nodeTable.empty()) { break; }
			m_defHandler->loadNodeTable();
			break;
		case enTagType_SimMesh_SimLinkTar:
			m_subobjHandler->loadLinkTar(*tag->pSimMesh);
			break;
		case enTagType_ColIDTbl:
			m_colHandler->loadColIDTbl();
			break;
		case enTagType_Cols:
			m_colHandler->loadCols();
			break;
		case enTagType_ColPack:
			m_colHandler->loadColPack();
			break;
		case enTagType_Capsule_Tapered:
			m_colHandler->loadCapsuleTapered(*tag);
			break;
		case enTagType_Capsule_Standard:
			m_colHandler->loadCapsuleStandard(*tag);
			break;
		case enTagType_SimMesh_ColVtx:
			m_colHandler->loadCollisionVerts(*tag, *tag->pSimMesh);
			break;
		default:
			break;

	}
}



