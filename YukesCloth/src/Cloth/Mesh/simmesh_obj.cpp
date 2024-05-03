#include "simmesh_obj.h"
#include "cgamecloth.h"
#include "memoryreader.h"

CSimMeshSubObj::CSimMeshSubObj(CSimObj* obj, char*& data) :
	m_pSimObj(obj),
	m_data(data)
{}


void CSimMeshSubObj::AssignSubObj(StSimMesh& sMesh) {
	uint32_t numTags = u32;
	sMesh.sMeshIndex = u32;
	sMesh.sObjIndex = u32;

#ifdef DEBUG_CONSOLE
	printf("\n\t\t- StSimMesh_AssignSubObj {sTags: %d, sModelName:%d, sObjName:%d }\n",
		numTags, sMesh.sMeshIndex, sMesh.sObjIndex);
#endif
}

void
CSimMeshSubObj::AssignSubObjVtx(StSimMesh& sMesh) {
	m_data = (char*)m_pSimObj->pos + 0x10;
	uint32_t numVerts = u32;

	m_data = (char*)m_pSimObj->pos + 0x20;
	for (uint32_t i = 0; i < numVerts; i++) {
		sMesh.sObjVerts.push_back(u32);
	}

}

void
CSimMeshSubObj::AssignSimVtx(StSimMesh& sMesh) {
	uint32_t unkVal0 = u32;
	uint32_t unkVal1 = u32;
	uint32_t numSimVerts = u32;
	m_data = (char*)m_pSimObj->pos + 0x20;

	for (uint32_t i = 0; i < numSimVerts; i++) {
		uint32_t nullValue = u32;
		sMesh.simVerts.push_back(u32);
	}
}

void
CSimMeshSubObj::SaveOldVtxs(StSimMesh& sMesh) {
	uint32_t numVerts = u32;
	m_data = (char*)m_pSimObj->pos + 0x10;

	for (int i = 0; i < numVerts; i++)
	{
		sMesh.saveVerts.push_back(u32);
	}
}

StSimMesh* CSimMeshSubObj::GetMeshSourceObj(const std::string& meshName)
{
	const StTag* pTag = m_pSimObj->getRootTag();

	for (auto& child : pTag->children) {
		StSimMesh* pMesh = child->pSimMesh;
		if (pMesh)
		{
			int stringIndex = pMesh->sNameIdx;
			std::string childObjName = m_pSimObj->getString(stringIndex);
			if (childObjName == meshName)
				return pMesh;
		}
	}

	return nullptr;
}


void CSimMeshSubObj::loadLinkTar_2024(StSimMesh& sLine) // using 2k24 format
{
	LinkTarget meshTarget;
	meshTarget.source = m_pSimObj->getString(u32);

	StSimMesh* pSourceMesh = GetMeshSourceObj(meshTarget.source);
	if (!pSourceMesh) {
		throw("\nCould not locate source mesh for target: %s", sLine.sObjName);
	}

	if (sLine.skin.vertices.empty())
		sLine.skin.vertices.resize(sLine.sSimVtxCount);

	uint32_t numIdxs = u32;
	uint32_t paletteSize = u32;
	meshTarget.totalWeights = u32;
	m_data = (char*)(m_pSimObj->pos + 0x20);

	/* Index and create a table of every used source edge from the sim mesh */
	for (uint32_t i = 0; i < numIdxs; i++) {
		int index = u32;
		Triangle srcTriangle = pSourceMesh->sourceEdges.at(index);

		meshTarget.sourceTris.push_back(srcTriangle);
		meshTarget.indices.push_back(index);
	}

	/* Index and create a table of all vertex coords for the target mesh */
	for (uint32_t i = 0; i < paletteSize; i++) {
		int index = u32;
		Vector4 vertexMat = sLine.skin.vertices.at(index);

		meshTarget.vertexTable.push_back(vertexMat);
		meshTarget.palette.push_back(index);
	}

	/* Iterate and define the influence of the source triangle on the specified vertex */
	for (uint32_t i = 0; i < paletteSize; i++) {
		MeshWeight blendWeight;
		for (int j = 0; j < meshTarget.totalWeights; j++) {
			blendWeight.numWeights = u16;
			int srcIndex = u16;
			float influence = f32;

			blendWeight.indices.push_back(srcIndex);
			blendWeight.weights.push_back(influence);

		}

		meshTarget.weights.push_back(blendWeight);
	}

	sLine.target = meshTarget;
}

void CSimMeshSubObj::loadLinkTar(StSimMesh& sLine)
{
	if (m_pSimObj->game_format == YUKES_CLOTH_24) {
		this->loadLinkTar_2024(sLine);
		return;
	}

	LinkTarget meshTarget;
	meshTarget.source = ReadString(m_data, 0x80);
	m_data = (char*)m_pSimObj->pos + 0x8C;

	StSimMesh* pSourceMesh = GetMeshSourceObj(meshTarget.source);
	if (!pSourceMesh) {
		throw("\nCould not locate source mesh for target: %s", sLine.sObjName);
	}

	uint32_t numIdxs = u32;
	uint32_t paletteSize = u32;
	meshTarget.totalWeights = u32;
	m_data = (char*)(m_pSimObj->pos + 0xA0);

	/* Index and create a table of every used source edge from the sim mesh */
	for (uint32_t i = 0; i < numIdxs; i++) {
		int index = u32;
		Triangle srcTriangle = pSourceMesh->sourceEdges.at(index);

		meshTarget.sourceTris.push_back(srcTriangle);
		meshTarget.indices.push_back(index);
	}

	/* Index and create a table of all vertex coords for the target mesh */
	for (uint32_t i = 0; i < paletteSize; i++) {
		int index = u32;
		Vector4 vertexMat = sLine.skin.vertices.at(index);

		meshTarget.vertexTable.push_back(vertexMat);
		meshTarget.palette.push_back(index);
	}

	/* Iterate and define the influence of the source triangle on the specified vertex */
	for (uint32_t i = 0; i < paletteSize; i++) {
		MeshWeight blendWeight;
		for (int j = 0; j < meshTarget.totalWeights; j++) {
			blendWeight.numWeights = u16;
			int srcIndex = u16;
			float influence = f32;

			blendWeight.indices.push_back(srcIndex);
			blendWeight.weights.push_back(influence);

		}

		meshTarget.weights.push_back(blendWeight);
	}

	sLine.target = meshTarget;
}


void CSimMeshSubObj::Link_DefineSourceMesh(StSimMesh& sMesh)
{
	uint32_t numTriangles = u32;

	m_data = (char*)m_pSimObj->pos + 0x20;

	/* Define source edges */
	for (int i = 0; i < numTriangles; i++) {
		sMesh.sourceEdges.push_back(Triangle{ u16, u16, u32 });
	}

}

void CSimMeshSubObj::loadCollection(StTag& tag)
{
	uint32_t numChildren = u32;
	uint32_t numUnk2 = u32;
	uint32_t numVerts = u32;


}