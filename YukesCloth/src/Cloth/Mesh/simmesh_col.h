#include "ClothStructs.h"
#pragma once

class CSimObj;

class CSimMeshCol
{
public:
    CSimMeshCol(CSimObj* obj, char*& data);

public:
    void loadCols();
    void loadColIDTbl();
    void loadColPack();
    void loadCapsuleTapered(StTag& tag);
    void loadCapsuleStandard(StTag& tag);
    void loadCollisionVerts(StTag& tag, StSimMesh& sMesh);

private:
    char* m_data;
    CSimObj* m_pSimObj;
};

