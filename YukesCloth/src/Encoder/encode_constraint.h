#include "ClothStructs.h"
#pragma once

class CSimObj;
struct TagBuffer;

class CSimEncodeConstraint
{
public:
    CSimEncodeConstraint(std::shared_ptr<CSimObj> obj, std::ofstream*& data);

public:
    void encodeForce(TagBuffer* sMesh);
    void encodeConstraintStretchLink(TagBuffer* sMesh);
    void encodeConstraintStandardLink(TagBuffer* sMesh);
    void encodeConstraintBendLink(TagBuffer* sMesh);
    void encodeConstraintFixation(TagBuffer* sMesh);
    void encodeBendStiffness(TagBuffer* sMesh);

private:
    std::ofstream*& m_data;
    std::shared_ptr<CSimObj> m_pSimObj;
};



