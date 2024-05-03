#include "ClothStructs.h"
#pragma once

class CSimObj;
struct TagBuffer;

class CSimEncodeDef
{
public:
    CSimEncodeDef(std::shared_ptr<CSimObj> obj, std::ofstream*& data);

public:
    void encodeSimMesh(TagBuffer* parent);
    void encodeSimLine(TagBuffer* pTag);
    void encodeAssignNode(TagBuffer* sLine);
    void encodeLineDef(TagBuffer* sMesh);
    void encodeRootTag(TagBuffer* pTagBuf);
    void encodeNodeTable(TagBuffer* pTagBuf);

private:
    std::ofstream*& m_data;
    std::shared_ptr<CSimObj> m_pSimObj;
};

