#include "ClothStructs.h"
#pragma once

class CSimObj;

class CSimMeshDef
{
public:
    CSimMeshDef(CSimObj* obj, char*& data);

public:
    void GetSimMesh(StTag& parent);
    void GetSimLine(StTag& pTag);
    void AssignNode(StSimMesh& sLine);
    void GetLineDef(StSimMesh& sMesh);
    void GetNodeTable();

private:
    bool useOldAssignNodeSchema(const uintptr_t& address, char*& fs, int numNodes);

private:
    char* m_data;
    CSimObj* m_pSimObj;
};

