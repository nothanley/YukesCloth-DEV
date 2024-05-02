#include "ClothStructs.h"
#pragma once

class CSimObj;

class CSimMeshPattern
{
public:
    CSimMeshPattern(CSimObj* obj, char*& data);

public:
    void GetSimMeshPattern(StSimMesh& sMesh);
    void GetSimMeshStacks(StSimMesh& sMesh);

private:
    char* m_data;
    CSimObj* m_pSimObj;
};
