/* Defines all sim stream parsing and interpretation logic. */

#include "ClothStructs.h"
#include <cstdint>
#include "../cloth_tags.h"
#include <fstream>
#pragma once

class CSimObj;

class CSimMeshData
{

	public:
        void GetSimMesh(StTag& parent, const CSimObj* pSimObj);
        void AssignSubObj(StSimMesh& sMesh, const CSimObj* pSimObj);
        void AssignSubObjVtx(StSimMesh& sMesh, const CSimObj* pSimObj);
        void AssignSimVtx(StSimMesh& sMesh, const CSimObj* pSimObj);
        void GetSkinData(StSimMesh& sMesh, const CSimObj* pSimObj);
        void Link_DefineSourceMesh(StSimMesh& sMesh, const CSimObj* pSimObj);
        void GetSimMeshPattern(StSimMesh& sMesh, const CSimObj* pSimObj);
        void GetSimMeshStacks(StSimMesh& sMesh, const CSimObj* pSimObj);
        void GetSkinCalc(StSimMesh& sMesh, const CSimObj* pSimObj);
        void GetSkinPaste(StSimMesh& sMesh, const CSimObj* pSimObj);
        void SaveOldVtxs(StSimMesh& sMesh, const CSimObj* pSimObj);
        void GetForce(StSimMesh& sMesh, const CSimObj* pSimObj);
        void GetConstraintStretchLink(StSimMesh& sMesh, const CSimObj* pSimObj);
        void GetConstraintStandardLink(StSimMesh& sMesh, const CSimObj* pSimObj);
        void GetConstraintBendLink(StSimMesh& sMesh, const CSimObj* pSimObj);
        void GetBendStiffness(StSimMesh& sMesh, const CSimObj* pSimObj);
        void GetCollisionVerts(StTag& pTag, StSimMesh& sMesh, const CSimObj* pSimObj);
        void GetConstraintFixation(StSimMesh& sMesh, const CSimObj* pSimObj);
        void GetSimLine(StTag &pTag, const CSimObj* pSimObj);
        void AssignNode(StSimMesh& sLine, CSimObj* pSimObj);
        void GetLineDef(StSimMesh& sMesh, CSimObj* pSimObj);
        void GetStringTable(CSimObj* pSimObj);
        void GetNodeTable(CSimObj* pSimObj);
        void GetLinkTar(StSimMesh& sLine, CSimObj* pSimObj);
        void GetColIDTbl(CSimObj* pSimObj);
        void GetCols(CSimObj* pSimObj);
        void GetColPack(CSimObj* pSimObj);
        void GetCapsuleTapered(CSimObj* pSimObj, StTag& tag);
        void GetCapsuleStandard(CSimObj* pSimObj, StTag& tag);
        std::string GetString(CSimObj* pSimObj);

protected:
    std::ifstream* m_pDataStream = nullptr;
    uintptr_t m_iStreamPos = std::ios::beg;

    friend class CSimSubObj_RCN;
};



