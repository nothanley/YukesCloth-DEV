#include "CGameCloth.h"
#include "BinaryIO.h"
using namespace BinaryIO;

CGameCloth::CGameCloth(std::ifstream* fs)
	: CSimObj()
{
	this->m_pDataStream = fs;
}


void
CGameCloth::load() {
	/* Initialize Tag Header */
	m_pStHead = new StTag{ _U32,_U32,_U32,_U32 };

#ifdef DEBUG_CONSOLE
	printf("\n====== StHead {sTagCount:%d, eType:%d, sSize:%d, sTotalSize:%d} ======\n",
		m_pStHead->sTagCount, m_pStHead->eType, m_pStHead->sSize, m_pStHead->sTotalSize);
#endif

	/* Update stream pointer */
	m_iStreamPos += m_pStHead->sSize;

	/* Iterate and collect all child nodes */
	for (int i = 0; i < m_pStHead->sTagCount; i++) {
		GetTag(m_iStreamPos, m_pStHead);
	}

	/* Setup node and string tables */
	StTag* nTableTag = FindTag(enTagType_NodeTbl);
	if (nTableTag) { InitTag(*nTableTag); }
	StTag* strTableTag = FindTag(enTagType_StrTbl);
	if (strTableTag) { InitTag(*strTableTag); }

	/* Initialize all other tags */
	SetupTags(m_pStHead);
	UpdateStrings();
};


