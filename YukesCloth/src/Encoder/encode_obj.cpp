#include "encode_obj.h"
#include "cgamecloth.h"
#include "memoryreader.h"
#include "clothencoder.h"

using namespace clothsrl;

CSimEncodeSubObj::CSimEncodeSubObj(std::shared_ptr<CSimObj> obj, std::ofstream*& data) :
	m_pSimObj(obj),
	m_data(data)
{}

void CSimEncodeSubObj::encodeSubObj(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
	uint32_t numChildren = pTagBuf->children.size();
	uint32_t sModelNameIdx = FindStringIndex(tag->pSimMesh->sModelName, m_pSimObj->stringTable());
	uint32_t sObjNameIdx = FindStringIndex(tag->pSimMesh->sObjName, m_pSimObj->stringTable());

	WriteUInt32(pTagBuf->binary, numChildren);
	WriteUInt32(pTagBuf->binary, sModelNameIdx);
	WriteUInt32(pTagBuf->binary, sObjNameIdx);
	AlignTagHeader(pTagBuf);
}

void CSimEncodeSubObj::encodeSubObjVtx(TagBuffer* pTagBuf) 
{
	const StTag* tag = pTagBuf->tag;
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

void CSimEncodeSubObj::encodeSimVtx(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* sMesh = tag->pSimMesh;

	uint32_t unkVal0 = 0x1;
	uint32_t sObjNameIdx = FindStringIndex(sMesh->sObjName, m_pSimObj->stringTable());
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


void CSimEncodeSubObj::encodeSaveVtx(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
	uint32_t numVerts = tag->pSimMesh->saveVerts.size();
	WriteUInt32(pTagBuf->binary, numVerts);

	for (auto& Idx : tag->pSimMesh->saveVerts) {
		WriteUInt32(pTagBuf->binary, Idx);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}


void CSimEncodeSubObj::encodeLinkTar(TagBuffer* pTagBuf) 
{
	const StTag* tag = pTagBuf->tag;
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


void CSimEncodeSubObj::encodeSimLinkSource(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
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


void CSimEncodeSubObj::encodeCollection(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;
	uint32_t numVerts = pSimMesh->skin.vertices.size();
	//printf("\nEncoding collection mesh with %d verts.", numVerts);

	WriteUInt32(pTagBuf->binary, tag->children.size());
	WriteUInt32(pTagBuf->binary, 0x0);
	WriteUInt32(pTagBuf->binary, numVerts);
	AlignTagHeader(pTagBuf);
}

void CSimEncodeSubObj::encodeLinkTar_2024(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;
	LinkTarget target = pSimMesh->target;

	int mesh_index = clothsrl::FindStringIndex(target.source, m_pSimObj->stringTable());
	WriteUInt32(pTagBuf->binary, mesh_index);
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



