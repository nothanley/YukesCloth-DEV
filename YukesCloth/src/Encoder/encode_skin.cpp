#include "encode_skin.h"
#include "cgamecloth.h"
#include "memoryreader.h"
#include "clothencoder.h"

using namespace clothsrl;

CSimEncodeSkin::CSimEncodeSkin(std::shared_ptr<CSimObj> obj, std::ofstream*& data) :
	m_pSimObj(obj),
	m_data(data)
{}


void CSimEncodeSkin::encodeSimMeshSkin(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* sMesh = tag->pSimMesh;

	uint32_t numVerts = sMesh->sSimVtxCount;
	MeshSkin skin = sMesh->skin;
	int maxWeights = 0;

	/* Create a bone palette */
	std::vector<SimNode> nodePalette = getNodePalette(skin, m_pSimObj, maxWeights);

	WriteUInt32(pTagBuf->binary, nodePalette.size());
	WriteUInt32(pTagBuf->binary, numVerts);
	WriteUInt32(pTagBuf->binary, maxWeights);
	WriteUInt32(pTagBuf->binary, 0x1);
	AlignTagHeader(pTagBuf);

	/* Write skin buffers */
	writeNodeSkinMatrices(pTagBuf, skin);
	writeSkinPalette(pTagBuf, nodePalette);
	writeSkin(pTagBuf, skin, nodePalette, maxWeights);

	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void CSimEncodeSkin::encodeSkinCalc(TagBuffer* pTagBuf) 
{
	const StTag* tag = pTagBuf->tag;
	uint32_t numVerts = tag->pSimMesh->skinCalc.size();

	WriteUInt32(pTagBuf->binary, numVerts);

	for (auto& vtxMat : tag->pSimMesh->skinCalc) {
		uint32_t matrixIndex = FindMatrixIndex(tag->pSimMesh, vtxMat);
		WriteUInt32(pTagBuf->binary, matrixIndex);
	}

	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void CSimEncodeSkin::encodeSkinPaste(TagBuffer* pTagBuf) 
{
	const StTag* tag = pTagBuf->tag;
	uint32_t numVerts = tag->pSimMesh->skinPaste.size();
	WriteUInt32(pTagBuf->binary, numVerts);

	for (auto& Idx : tag->pSimMesh->skinPaste) {
		WriteUInt32(pTagBuf->binary, Idx);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}


void CSimEncodeSkin::writeNodeSkinMatrices(TagBuffer* pTagBuf, MeshSkin& skin) {

	for (auto& matrix : skin.vertices) {
		WriteFloat(pTagBuf->binary, matrix.x);
		WriteFloat(pTagBuf->binary, matrix.y);
		WriteFloat(pTagBuf->binary, matrix.z);
		WriteFloat(pTagBuf->binary, matrix.w);
	}

	for (auto& weight : skin.normals) {
		WriteFloat(pTagBuf->binary, weight.x);
		WriteFloat(pTagBuf->binary, weight.y);
		WriteFloat(pTagBuf->binary, weight.z);
		WriteFloat(pTagBuf->binary, weight.w);
	}

}

void CSimEncodeSkin::writeSkinPalette(TagBuffer* pTagBuf, std::vector<SimNode>& nodePalette)
{
	for (auto& node : nodePalette) {
		int index = FindNodeIndex(node.name, m_pSimObj->getNodes());
		WriteUInt32(pTagBuf->binary, index);
	}
}

void CSimEncodeSkin::writeSkin(TagBuffer* pTagBuf, MeshSkin skin, std::vector<SimNode>& nodePalette, const int& weights)
{
	for (auto& blendWeight : skin.blendWeights)
		for (int i = 0; i < weights; i++) {
			uint32_t totalWeights = blendWeight.numWeights;

			if (i >= totalWeights) {
				WriteUInt64(pTagBuf->binary, 0x0);
			}
			else
			{
				std::string boneName = blendWeight.bones.at(i);
				float weight = blendWeight.weights.at(i);
				uint32_t paletteIdx = FindNodeIndex(boneName, nodePalette);

				WriteUInt16(pTagBuf->binary, blendWeight.numWeights);
				WriteUInt16(pTagBuf->binary, paletteIdx);
				WriteFloat(pTagBuf->binary, weight);

			}
		}
}

