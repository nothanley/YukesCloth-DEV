#include "ClothStructs.h"
#pragma once

class CSimObj;

class CSimMeshConstraint
{
public:
    CSimMeshConstraint(CSimObj* obj, char*& data);

public:
    void loadForce(StSimMesh& sMesh);
    void loadConstraintStretchLink(StSimMesh& sMesh);
    void loadConstraintStandardLink(StSimMesh& sMesh);
    void loadConstraintBendLink(StSimMesh& sMesh);
    void loadConstraintFixation(StSimMesh& sMesh);
    void loadBendStiffness(StSimMesh& sMesh);

private:
    char*& m_data;
    CSimObj* m_pSimObj;
};

