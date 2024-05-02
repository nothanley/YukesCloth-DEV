#include "ClothStructs.h"
#pragma once

class CSimObj;

class CSimMeshSubObj
{
public:
    CSimMeshSubObj(CSimObj* obj, char*& data);

public:
    void AssignSubObj(StSimMesh& sMesh);
    void AssignSubObjVtx(StSimMesh& sMesh);
    void AssignSimVtx(StSimMesh& sMesh);
    void SaveOldVtxs(StSimMesh& sMesh);
    void loadLinkTar(StSimMesh& sLine);
    void Link_DefineSourceMesh(StSimMesh& sMesh);

private:
    StSimMesh* GetMeshSourceObj(const std::string& meshName);

private:
    char*& m_data;
    CSimObj* m_pSimObj;
};

