/* Defines all sim stream parsing and interpretation logic. */

#include "ClothStructs.h"
#include <cstdint>
#pragma once

class CSimObj;

enum {
    enTagType_Root = 0x0,
    enTagType_Cols = 0x01,
    enTagType_ColPack = 0x02,
    enTagType_Capsule_Standard = 0x03,
    enTagType_Capsule_Tapered = 0x04,
    enTagType_SimMesh = 0x05,
    enTagType_SimMesh_AssignSubObj = 0x06,
    enTagType_SimMesh_AssignSubObjVtx = 0x07,
    enTagType_SimMesh_AssignSimVtx = 0x08,
    enTagType_SimMesh_Stacks = 0x09,
    enTagType_SimMesh_SkinCalc = 0x0A,
    enTagType_SimMesh_SkinPaste = 0x0B,
    enTagType_SimMesh_Force = 0x0C,
    enTagType_SimMesh_CtStdLink = 0x0D,
    enTagType_SimMesh_CtBendLink = 0x0E,
    enTagType_SimMesh_CtLocalRange = 0x0F,
    enTagType_SimMesh_CtFixation = 0x10,
    enTagType_SimMesh_SimLinkTar = 0x11,
    enTagType_SimMesh_ColVtx = 0x12,
    enTagType_SimMesh_RCN = 0x13,
    enTagType_SimMesh_RCNSubObj = 0x14,
    enTagType_SimMesh_Skin = 0x15,
    enTagType_SimMesh_SimLinkSrc = 0x16,
    enTagType_StrTbl = 0x17,
    enTagType_String = 0x18,
    enTagType_NodeTbl = 0x19,
    enTagType_SimMesh_OldVtxSave = 0x1A,
    enTagType_SimLine = 0x1B,
    enTagType_SimLine_LineDef = 0x1C,
    enTagType_SimLine_AssignNode = 0x1D,
    enTagType_BOGUS_30 = 0x1E,
    enTagType_SimMesh_Pattern = 0x1F,
    enTagType_ColIDTbl = 0x20,
    enTagType_SimMesh_CtStretchLink = 0x21,
    enTagType_Box_S = 0x22,
    enTagType_Box_D = 0x23,
    enTagType_ColIDInfo = 0x24,
    enTagType_SimMesh_ColVtxFast = 0x25,
    enTagType_ColMeshLink_Model = 0x26,
    enTagType_ColMeshLink_Object = 0x27,
    enTagType_ColMeshLink_SubObj = 0x28,
    enTagType_ColMeshLink_Col = 0x29,
    enTagType_ColMeshLink_Capsule = 0x2A,
    enTagType_SimMesh_CtNodeFixation = 0x2B,
    enTagType_SimMesh_AnimNodeFixation = 0x2C,
    enTagType_SimMesh_Wind = 0x2D,
    enTagType_SimMesh_TarNodeLink = 0x2E,
    enTagType_SimMesh_SrcNodeLink = 0x2F,
    enTagType_SimMesh_CtLinkFixation = 0x30,
    enTagType_SimMesh_CtCancelFixation = 0x31,
    enTagType_SimMesh_SelfLinkCapsule = 0x32,
    enTagType_ColNameTbl = 0x33,
    enTagType_Box_R = 0x34,
    enTagType_SimMesh_Friction = 0x35,
    enTagType_SimMesh_CtLinkFixationST = 0x36,
    enTagType_SimMesh_CtEdgeLength = 0x37,
    enTagType_SimMesh_BendingStiffness = 0x38
};

class CSimMeshData
{
	public:
        static void GetSimMesh(StTag& parent, const CSimObj* pSimObj);
        static void AssignSubObj(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void AssignSubObjVtx(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void AssignSimVtx(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void GetSkinData(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void Link_DefineSourceMesh(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void GetSimMeshPattern(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void GetSimMeshStacks(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void GetSkinCalc(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void GetSkinPaste(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void SaveOldVtxs(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void GetForce(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void GetConstraintStretchLink(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void GetConstraintStandardLink(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void GetConstraintBendLink(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void GetBendStiffness(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void GetCollisionVerts(StTag& pTag, StSimMesh& sMesh, const CSimObj* pSimObj);
        static void GetConstraintFixation(StSimMesh& sMesh, const CSimObj* pSimObj);
        static void GetSimLine(StTag &pTag, const CSimObj* pSimObj);
        static void AssignNode(StSimMesh& sLine, CSimObj* pSimObj);
        static void GetLineDef(StSimMesh& sMesh, CSimObj* pSimObj);
        static void GetStringTable(CSimObj* pSimObj);
        static void GetNodeTable(CSimObj* pSimObj);
        static void GetLinkTar(StSimMesh& sLine, CSimObj* pSimObj);
        static void GetColIDTbl(CSimObj* pSimObj);
        static void GetCols(CSimObj* pSimObj);
        static void GetColPack(CSimObj* pSimObj);
        static void GetCapsuleTapered(CSimObj* pSimObj, StTag& tag);
        static void GetCapsuleStandard(CSimObj* pSimObj, StTag& tag);
        static std::string GetString(CSimObj* pSimObj);
};

