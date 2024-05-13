#include "ClothStructs.h"
#pragma once

class CSimObj;

class CSimMeshDef
{
public:
    CSimMeshDef(CSimObj* obj, char*& data);

public:
    void loadSimMesh(StTag& parent);
    void loadSimLine(StTag& pTag);
    void AssignNode(StSimMesh& sLine);
    void loadLineDef(StSimMesh& sMesh);
    void InitializeNodePalette(const StTag& parent);
    void loadNodeTable();
    void loadNodeCopy(StTag& pTag);

private:
    bool useOldAssignNodeSchema(const uintptr_t& address, char*& fs, int numNodes);

private:
    char*& m_data;
    CSimObj* m_pSimObj;
};

