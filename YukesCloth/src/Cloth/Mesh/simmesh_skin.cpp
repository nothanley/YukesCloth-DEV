#include "simmesh_skin.h"
#include "cgamecloth.h"
#include "memoryreader.h"


CSimMeshSkin::CSimMeshSkin(CSimObj* obj, char*& data) :
	m_pSimObj(obj),
	m_data(data)
{}


void CSimMeshSkin::loadSkinData(StSimMesh& sMesh)
{
	/* This buffer is arranged with respect to the simVtx table,
		correct the output to match the table */
	uint32_t numBones = u32;
	uint32_t numVerts = u32;
	uint32_t maxBlendWeights = u32;
	uint32_t numOther = u32;
	m_data = (char*)m_pSimObj->pos + 0x20;

	/* Get vertex coordinates */
	for (int i = 0; i < numVerts; i++) {
		Vector4 vertex{ f32, f32, f32, f32 };
		sMesh.skin.vertices.push_back(vertex);
	}

	/* Get vertex normals */
	for (int i = 0; i < numVerts; i++) {
		Vector4 normal{ f32, f32, f32, f32 };
		sMesh.skin.normals.push_back(normal);
	}
	uintptr_t skinBufferAddress = (uintptr_t)m_data;

	/* Get blendweight palette */
	std::vector<SimNode> skinPalette;
	for (int i = 0; i < numBones; i++) {
		int nodeIndex = u32;
		SimNode node = m_pSimObj->getNode(nodeIndex);
		skinPalette.push_back(node);
	}

	/* Get blendweights */
	for (int i = 0; i < numVerts; i++) {
		MeshWeight blendWeight;
		blendWeight.numWeights = maxBlendWeights;

		for (int i = 0; i < maxBlendWeights; i++) {
			if (i >= blendWeight.numWeights) {
				u16;
				u16;
				f32;
			}
			else {
				blendWeight.numWeights = u16;
				blendWeight.bones.push_back(skinPalette.at(u16).name);
				blendWeight.weights.push_back(f32);
			}
		}
		sMesh.skin.blendWeights.push_back(blendWeight);
	}

	sMesh.skin.nodePalette = skinPalette;
}

void CSimMeshSkin::loadSkinCalc(StSimMesh& sMesh)
{
	uint32_t numSkinVerts = u32;
	m_data = (char*)m_pSimObj->pos + 0x10;

	if (sMesh.skin.vertices.empty())
		sMesh.skin.vertices.resize( sMesh.sSimVtxCount );

	for (int i = 0; i < numSkinVerts; i++)
	{
		uint32_t vertIdx = u32;
		Vector4 skinMatrix = sMesh.skin.vertices.at(vertIdx);
		sMesh.skinCalc.push_back(skinMatrix);

#ifdef DEBUG_EDITOR
		sMesh.skin_calc_indices.push_back(vertIdx);
#endif
	}
}

void CSimMeshSkin::loadSkinPaste(StSimMesh& sMesh)
{
	uint32_t numPasteVerts = u32;
	m_data = (char*)m_pSimObj->pos + 0x10;

	for (int i = 0; i < numPasteVerts; i++)
	{
		uint32_t skinIdx = u32;
		sMesh.skinPaste.push_back(skinIdx);
	}
}

