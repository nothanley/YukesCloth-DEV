#include "ClothStructs.h"
#pragma once

class CSimObj;

class CSimMeshConstraint
{
public:
    CSimMeshConstraint(CSimObj* obj, char*& data);

public:
    void GetForce(StSimMesh& sMesh);
    void GetConstraintStretchLink(StSimMesh& sMesh);
    void GetConstraintStandardLink(StSimMesh& sMesh);
    void GetConstraintBendLink(StSimMesh& sMesh);
    void GetConstraintFixation(StSimMesh& sMesh);
    void GetBendStiffness(StSimMesh& sMesh);

private:
    char* m_data;
    CSimObj* m_pSimObj;
};

