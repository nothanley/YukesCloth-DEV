#include "ClothStructs.h"
#pragma once

class CSimObj;

class CSimMeshPattern
{
public:
    CSimMeshPattern(CSimObj* obj, char*& data);

public:
    void loadSimMeshPattern(StSimMesh& sMesh);
    void loadSimMeshStacks(StSimMesh& sMesh);

private:
    char*& m_data;
    CSimObj* m_pSimObj;
};
