#include "csimmesh.h"
#include "cgamecloth.h"
#include "encode_rcn.h"
#include "memoryreader.h"
#include <algorithm>
#include "clothencoder.h"

using namespace clothsrl;

CSimEncodeRCN::CSimEncodeRCN(std::shared_ptr<CSimObj> obj, std::ofstream*& data) :
	m_pSimObj(obj),
	m_data(data)
{}


void CSimEncodeRCN::encodeRCNData(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
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

void CSimEncodeRCN::encodeRecalcNormals(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* sMesh = tag->pSimMesh;

	uint32_t numChildren = pTagBuf->children.size();
	WriteUInt32(pTagBuf->binary, numChildren);

	for (auto& param : sMesh->rcn.parameters) {
		WriteUInt32(pTagBuf->binary, param);
	}

	AlignTagHeader(pTagBuf);
}


void CSimEncodeRCN::WriteRCNIndexBuffer(std::stringstream & pBf, StSimMesh * sMesh, uint32_t numElements)
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

void CSimEncodeRCN::WriteRCNNormalBuffer(std::stringstream& pBf, StSimMesh* sMesh, uint32_t numElements)
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

