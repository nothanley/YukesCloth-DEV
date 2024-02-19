#include <Cloth/ClothStructs.h>
#include <cstdint>
#pragma once

class CSimObj;


class CSimSubObj_RCN
{
public:
    static void GetRCNData(StSimMesh& sMesh, const CSimObj* pSimObj);

    static void GetRecalcNormals(StSimMesh& sMesh, const CSimObj* pSimObj);

    static void GetFaceNormalDegCountTbl(StSimMesh& sMesh,
        const CSimObj* pSimObj, const int& numFaceNrmVtxIndices);

    static void GetFaceNormalVtxIndexTbl(StSimMesh& sMesh,
        const CSimObj* pSimObj, const int& numFaceNrmVtxIndices);

    static void GetVtxNormalFaceLinkCountTbl(StSimMesh& sMesh,
        const CSimObj* pSimObj, const int& numVtxNrmFaceLinks);

    static void GetVtxNormalFaceLinkTbl(StSimMesh& sMesh,
        const CSimObj* pSimObj, const int& numVtxNrmFaceLinks);
};

