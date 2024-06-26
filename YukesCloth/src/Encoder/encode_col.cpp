#include "encode_col.h"
#include "cgamecloth.h"
#include "memoryreader.h"
#include "clothencoder.h"

using namespace clothsrl;

CSimEncodeCol::CSimEncodeCol(std::shared_ptr<CSimObj> obj, std::ofstream*& data) :
	m_pSimObj(obj),
	m_data(data)
{}

void CSimEncodeCol::encodeColIDTbl(TagBuffer* pTag)
{
	const StTag* tag = pTag->tag;

	WriteUInt32(pTag->binary, tag->children.size());
	WriteUInt32(pTag->binary, tag->children.size());
	AlignTagHeader(pTag);
}

void CSimEncodeCol::encodeCols(TagBuffer* pTag)
{
	const StTag* tag = pTag->tag;
	uint32_t numChilds = tag->children.size();
	WriteUInt32(pTag->binary, numChilds);
	WriteUInt32(pTag->binary, -1);

	for (int i = 0; i < 4; i++)
		WriteUInt64(pTag->binary, 0x0);
	AlignTagHeader(pTag);
}

void CSimEncodeCol::encodeColPack(TagBuffer* pTag)
{
	const StTag* tag = pTag->tag;
	uint32_t numChilds = tag->children.size();

	WriteUInt32(pTag->binary, numChilds);
	WriteUInt32(pTag->binary, 0);
	WriteUInt32(pTag->binary, tag->children.front()->eType);
	AlignTagHeader(pTag);
}
void CSimEncodeCol::encodeCapsuleStandard(TagBuffer* pTag)
{
	const StTag* tag = pTag->tag;
	CollisionVolume col = tag->col;

	WriteUInt32(pTag->binary, FindStringIndex(col.name, m_pSimObj->stringTable()));
	WriteUInt32(pTag->binary, FindNodeIndex(col.joint, m_pSimObj->getNodes()));

	for (auto& weight : col.weights) {
		WriteFloat(pTag->binary, weight);
	}

	WriteUInt32(pTag->binary, col.id);

	for (int i = 0; i < 4; i++)
		WriteUInt64(pTag->binary, 0x0);

	AlignTagHeader(pTag);
}

void CSimEncodeCol::encodeCapsuleTapered(TagBuffer* pTag)
{
	const StTag* tag = pTag->tag;
	CollisionVolume col = tag->col;

	WriteUInt32(pTag->binary, FindStringIndex(col.name, m_pSimObj->stringTable()));
	WriteUInt32(pTag->binary, FindNodeIndex(col.joint, m_pSimObj->getNodes()));

	for (auto& weight : col.weights) {
		WriteFloat(pTag->binary, weight);
	}

	WriteFloat(pTag->binary, col.radius);
	WriteUInt32(pTag->binary, col.id);

	for (int i = 0; i < 6; i++)
		WriteUInt64(pTag->binary, 0x0);

	AlignTagHeader(pTag);
}

void CSimEncodeCol::encodeColIdInfo(TagBuffer* pTag)
{
	const StTag* tag = pTag->tag;
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

static void WriteColVertHeader(TagBuffer* pTagBuf, const CollisionVerts& cGroup)
{
	int32_t unkFlagA = cGroup.unkFlag;
	int32_t unkFlagB = cGroup.unkFlagB;
	int32_t unkVal   = cGroup.unkVal;
	int32_t numItems = cGroup.numItems;
	int32_t numVerts = cGroup.numVerts;

	WriteInt32(pTagBuf->binary, cGroup.unkFlag);
	for (int i = 0; i < 4; i++)
		WriteUInt64(pTagBuf->binary, 0x0);

	WriteInt32(pTagBuf->binary, cGroup.unkVal);
	WriteInt32(pTagBuf->binary, cGroup.numItems);
	WriteInt32(pTagBuf->binary, cGroup.numVerts);
	WriteInt32(pTagBuf->binary, cGroup.unkFlagB);

	uint32_t numPads = (cGroup.tagSize != 0x50) ? 4 : 2;
	for (int i = 0; i < numPads; i++)
		WriteUInt64(pTagBuf->binary, 0x0);

	AlignTagHeader(pTagBuf);
}

void CSimEncodeCol::encodeCollisionVerts(TagBuffer* pTagBuf, StTag* tag, StSimMesh* pSimMesh)
{
	int col_index = tag->vtxCol_index;
	if (col_index > pSimMesh->vtxColGroups.size())
		throw std::runtime_error("Col group index cannot be greater than sim mesh array.");

	CollisionVerts& cGroup = pSimMesh->vtxColGroups.at(col_index);
	::WriteColVertHeader(pTagBuf, cGroup);

	for (auto& point : cGroup.items) {
		WriteUInt32(pTagBuf->binary, point.x.index);
		WriteUInt32(pTagBuf->binary, point.y.index);

		WriteFloat(pTagBuf->binary,  point.x.weight);
		WriteFloat(pTagBuf->binary,  point.y.weight);
	}

	for (auto& point : cGroup.indices) {
		WriteUInt32(pTagBuf->binary, point);
	}

	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

