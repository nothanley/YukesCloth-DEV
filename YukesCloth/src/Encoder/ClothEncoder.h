#include "clothserial.h"
#include "YukesSimMesh"
#include "csimmesh.h"
#pragma once

#define BINARY_ALIGNMENT 16

class CClothEncoder
{
public:
	CClothEncoder(std::ofstream* outStream, std::shared_ptr<CSimObj>& pSimObj);
	void save();

private:
	void setupHandles();
	void initTagBuffers(TagBuffer* pTagBf);
	void writeTagHead(TagBuffer* pNode);
	void writeTree(TagBuffer* pRootNode);
	void encodeAllTags(TagBuffer* pRootNode);
	void encodeTag(TagBuffer* tag);
	void encodeStringTable(TagBuffer* pTagBuf);
	void encodeString(TagBuffer* pTagBuf);
	uint32_t getTagTotalSize(TagBuffer* pTag);

private:
	std::unique_ptr<CSimEncodeCol>        m_colHandler;
	std::unique_ptr<CSimEncodeConstraint> m_constHandler;
	std::unique_ptr<CSimEncodeDef>        m_defHandler;
	std::unique_ptr<CSimEncodeSubObj>     m_subobjHandler;
	std::unique_ptr<CSimEncodePattern>    m_patternHandler;
	std::unique_ptr<CSimEncodeSkin>       m_skinHandler;
	std::unique_ptr<CSimEncodeRCN>        m_rcnHandler;

private:
	int m_version = YUKES_CLOTH_23;
	std::shared_ptr<CSimObj> m_pSimObj;
	std::ofstream* m_pDataStream;
	uintptr_t m_iStreamPos = std::ios::beg;
};

