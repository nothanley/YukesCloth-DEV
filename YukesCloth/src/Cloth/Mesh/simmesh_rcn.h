#include "ClothStructs.h"
#pragma once

class CSimObj;

class CSimSubObj_RCN
{
public:
    CSimSubObj_RCN(CSimObj* obj, char*& data);

public:
    void getRCNData(StSimMesh& sMesh);
    void getRecalcNormals(StSimMesh& sMesh);
    void getFaceNormalDegCountTbl(StSimMesh& sMesh, const int& numFaceNrmVtxIndices);
    void getFaceNormalVtxIndexTbl(StSimMesh& sMesh, const int& numFaceNrmVtxIndices);
    void getVtxNormalFaceLinkCountTbl(StSimMesh& sMesh, const int& numVtxNrmFaceLinks);
    void getVtxNormalFaceLinkTbl(StSimMesh& sMesh, const int& numVtxNrmFaceLinks);

private:
    char* m_data;
    CSimObj* m_pSimObj;
};
