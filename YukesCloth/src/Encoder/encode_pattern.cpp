#include "encode_pattern.h"
#include "cgamecloth.h"
#include "memoryreader.h"
#include "clothencoder.h"

using namespace clothsrl;

CSimEncodePattern::CSimEncodePattern(std::shared_ptr<CSimObj> obj, std::ofstream*& data) :
	m_pSimObj(obj),
	m_data(data)
{}


void CSimEncodePattern::encodeSimMeshPattern(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
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

void CSimEncodePattern::encodeSimMeshStacks(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
	uint32_t numChildren = pTagBuf->children.size();
	WriteUInt32(pTagBuf->binary, numChildren);
}
