#include <Cloth/SimMesh.h>
#include <Cloth/SimObj.h>
#include "Cloth/Mesh/SimSubObj_RCN.h"
#include "BinaryIO.h"
#include<algorithm>
using namespace BinaryIO;

#define _U32  ReadUInt32(*pSimObj->m_pDataStream)
#define _S32  ReadSInt32(*pSimObj->m_pDataStream)
#define _U16  ReadUShort(*pSimObj->m_pDataStream)
#define _U8  ReadByte(*pSimObj->m_pDataStream)
#define _BOOL ReadBool(*pSimObj->m_pDataStream)
#define _FLOAT ReadFloat(*pSimObj->m_pDataStream)

void
CSimSubObj_RCN::GetRCNData(StSimMesh& sMesh, const CSimObj* pSimObj) {
	uint32_t numNodes = _U32;
	sMesh.rcn.parameters.push_back(_U32);
	sMesh.rcn.parameters.push_back(_U32); /* number of item a */
	sMesh.rcn.parameters.push_back(_U32);
	sMesh.rcn.parameters.push_back(_U32); /* number of item b */
}


void
CSimSubObj_RCN::GetFaceNormalDegCountTbl(StSimMesh& sMesh, 
											const CSimObj* pSimObj,
											const int& numFaceNrmVtxIndices) 
{
	/* Gets the total length of each element */
	/* Defined as face normal degree count table */
	for (int i = 0; i < numFaceNrmVtxIndices; i++) {
		uint8_t size = _U8;
		sMesh.rcn.degCounts.push_back(size);
	}

	/* align buffer */
	while (pSimObj->m_pDataStream->tellg() % 4 != 0) {
		ReadByte(*pSimObj->m_pDataStream);
	}

}

/* Collects all corner indices of specified polygon and creates a table */
void
CSimSubObj_RCN::GetFaceNormalVtxIndexTbl(StSimMesh& sMesh,
	const CSimObj* pSimObj,
	const int& numFaceNrmVtxIndices)
{
	for (int i = 0; i < numFaceNrmVtxIndices; i++) {

		/* Iterate and collect all face corner indices */
		uint8_t totalIndices = sMesh.rcn.degCounts.at(i);
		MeshPolygon face;

		for (int j = 0; j < totalIndices; j++){
			int index = _U32;
			sMesh.rcn.faceNrmVtxIndices.push_back(index);
			int subIndex = sMesh.simVerts.at(index);
			face.points.push_back(subIndex);
		}

		/* Add to collection */
		sMesh.rcn.faces.push_back(face);
	}
}

/* Defines total number of links for each vertex normal face value */
void
CSimSubObj_RCN::GetVtxNormalFaceLinkCountTbl(StSimMesh& sMesh,
	const CSimObj* pSimObj,
	const int& numVtxNrmFaceLinks)
{
	/* Gets the total length of each element */
	/* Defined as vertex normal face link count table*/
	for (int i = 0; i < numVtxNrmFaceLinks; i++) {
		uint8_t size = _U8;
		sMesh.rcn.linkCounts.push_back(size);
	}

	/* align buffer */
	while (pSimObj->m_pDataStream->tellg() % 4 != 0) {
		ReadByte(*pSimObj->m_pDataStream);
	}
}



void
CSimSubObj_RCN::GetVtxNormalFaceLinkTbl(StSimMesh& sMesh,
	const CSimObj* pSimObj,
	const int& numVtxNrmFaceLinks)
{
	/* Reads element index values using specified length value */
	/* Defined as vertex normal face link table*/
	for (int i = 0; i < numVtxNrmFaceLinks; i++) {
		uint8_t numLinks = sMesh.rcn.linkCounts.at(i);

		for (int j = 0; j < numLinks; j++){
			uint32_t index = _U32;
			float weight = _FLOAT;

			MeshPolygon face = sMesh.rcn.faces.at(index);
			sMesh.rcn.vtxNrmFaceLinks.push_back(SkinVertex{ index ,weight });
		}
	}
}

void
CSimSubObj_RCN::GetRecalcNormals(StSimMesh& sMesh, const CSimObj* pSimObj) {
	uint32_t numNodes = _U32;
	sMesh.rcn.numFaceNrmVtxIndices = _U32;
	sMesh.rcn.numVtxNrmFaceLinks = _U32;
	uint32_t faceNrmVtxIdxSize = _U32;
	uint32_t vtxNrmFaceLinkCountSize = _U32;

	/* Defines number & index of vertices connected to quad face normals*/
	GetFaceNormalDegCountTbl(sMesh, pSimObj, sMesh.rcn.numFaceNrmVtxIndices);
	GetFaceNormalVtxIndexTbl(sMesh, pSimObj, sMesh.rcn.numFaceNrmVtxIndices);

	/* Defines ... */
	GetVtxNormalFaceLinkCountTbl(sMesh, pSimObj, sMesh.rcn.numVtxNrmFaceLinks);
	GetVtxNormalFaceLinkTbl(sMesh, pSimObj, sMesh.rcn.numVtxNrmFaceLinks);
}
