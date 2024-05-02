#include "csimmesh.h"
#include "cgamecloth.h"
#include "simmesh_rcn.h"
#include "memoryreader.h"
#include <algorithm>


CSimSubObj_RCN::CSimSubObj_RCN(CSimObj* obj, char*& data) :
	m_pSimObj(obj),
	m_data(data)
{}


void CSimSubObj_RCN::getRCNData(StSimMesh& sMesh)
{
	uint32_t numNodes = u32;
	sMesh.rcn.parameters.push_back(u32);
	sMesh.rcn.parameters.push_back(u32); /* number of item a */
	sMesh.rcn.parameters.push_back(u32);
	sMesh.rcn.parameters.push_back(u32); /* number of item b */
}

void CSimSubObj_RCN::getFaceNormalDegCountTbl(StSimMesh& sMesh, const int& numFaceNrmVtxIndices) 
{
	/* Gets the total length of each element */
	/* Defined as face normal degree count table */
	for (int i = 0; i < numFaceNrmVtxIndices; i++) {
		uint8_t size = s8;
		sMesh.rcn.degCounts.push_back(size);
	}

	::align_binary_stream(m_data);
}

/* Collects all corner indices of specified polygon and creates a table */
void CSimSubObj_RCN::getFaceNormalVtxIndexTbl(StSimMesh& sMesh, const int& numFaceNrmVtxIndices)
{
	for (int i = 0; i < numFaceNrmVtxIndices; i++) {

		/* Iterate and collect all face corner indices */
		uint8_t totalIndices = sMesh.rcn.degCounts.at(i);
		MeshPolygon face;

		for (int j = 0; j < totalIndices; j++){
			int index = u32;
			sMesh.rcn.faceNrmVtxIndices.push_back(index);
			int subIndex = sMesh.simVerts.at(index);
			face.points.push_back(subIndex);
		}

		/* Add to collection */
		sMesh.rcn.faces.push_back(face);
	}
}

/* Defines total number of links for each vertex normal face value */
void CSimSubObj_RCN::getVtxNormalFaceLinkCountTbl(StSimMesh& sMesh, const int& numVtxNrmFaceLinks)
{
	/* Gets the total length of each element */
	/* Defined as vertex normal face link count table*/
	for (int i = 0; i < numVtxNrmFaceLinks; i++) {
		uint8_t size = s8;
		sMesh.rcn.linkCounts.push_back(size);
	}

	::align_binary_stream(m_data);
}



void CSimSubObj_RCN::getVtxNormalFaceLinkTbl(StSimMesh& sMesh, const int& numVtxNrmFaceLinks)
{
	/* Reads element index values using specified length value */
	/* Defined as vertex normal face link table*/
	for (int i = 0; i < numVtxNrmFaceLinks; i++) {
		uint8_t numLinks = sMesh.rcn.linkCounts.at(i);

		for (int j = 0; j < numLinks; j++){
			uint32_t index = u32;
			float weight   = f32;

			MeshPolygon face = sMesh.rcn.faces.at(index);
			sMesh.rcn.vtxNrmFaceLinks.push_back(SkinVertex{ index ,weight });
		}
	}
}

void CSimSubObj_RCN::getRecalcNormals(StSimMesh& sMesh) 
{
	uint32_t numNodes                = u32;
	sMesh.rcn.numFaceNrmVtxIndices   = u32;
	sMesh.rcn.numVtxNrmFaceLinks     = u32;
	uint32_t faceNrmVtxIdxSize       = u32;
	uint32_t vtxNrmFaceLinkCountSize = u32;

	/* Defines number & index of vertices connected to quad face normals*/
	getFaceNormalDegCountTbl(sMesh, sMesh.rcn.numFaceNrmVtxIndices);
	getFaceNormalVtxIndexTbl(sMesh, sMesh.rcn.numFaceNrmVtxIndices);

	/* Defines ... */
	getVtxNormalFaceLinkCountTbl(sMesh, sMesh.rcn.numVtxNrmFaceLinks);
	getVtxNormalFaceLinkTbl(sMesh, sMesh.rcn.numVtxNrmFaceLinks);
}


