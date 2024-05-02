#include "ClothStructs.h"
#pragma once

class CSimObj;

class CSimSubObj_RCN
{
public:
    CSimSubObj_RCN(CSimObj* obj, char*& data);

public:
    void loadRCNData(StSimMesh& sMesh);
    void loadRecalcNormals(StSimMesh& sMesh);
    void loadFaceNormalDegCountTbl(StSimMesh& sMesh, const int& numFaceNrmVtxIndices);
    void loadFaceNormalVtxIndexTbl(StSimMesh& sMesh, const int& numFaceNrmVtxIndices);
    void loadVtxNormalFaceLinkCountTbl(StSimMesh& sMesh, const int& numVtxNrmFaceLinks);
    void loadVtxNormalFaceLinkTbl(StSimMesh& sMesh, const int& numVtxNrmFaceLinks);

private:
    char*& m_data;
    CSimObj* m_pSimObj;
};
