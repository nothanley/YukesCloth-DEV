#include "ClothStructs.h"
#pragma once

class CSimObj;

class CSimMeshSkin
{
public:
    CSimMeshSkin(CSimObj* obj, char*& data);

public:
    void loadSkinData(StSimMesh& sMesh);
    void loadSkinCalc(StSimMesh& sMesh);
    void loadSkinPaste(StSimMesh& sMesh);

private:
    char*& m_data;
    CSimObj* m_pSimObj;
};

