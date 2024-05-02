#include "ClothStructs.h"
#pragma once

class CSimObj;

class CSimMeshSkin
{
public:
    CSimMeshSkin(CSimObj* obj, char*& data);

public:
    void GetSkinData(StSimMesh& sMesh);
    void GetSkinCalc(StSimMesh& sMesh);
    void GetSkinPaste(StSimMesh& sMesh);

private:
    char* m_data;
    CSimObj* m_pSimObj;
};

