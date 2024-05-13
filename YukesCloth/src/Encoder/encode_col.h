#include "ClothStructs.h"
#pragma once

class CSimObj;
struct TagBuffer;

class CSimEncodeCol
{
public:
    CSimEncodeCol(std::shared_ptr<CSimObj> obj, std::ofstream*& data);

public:
    void encodeCols(TagBuffer* tag);
    void encodeColIDTbl(TagBuffer* tag);
    void encodeColPack(TagBuffer* tag);
    void encodeCapsuleTapered(TagBuffer* tag);
    void encodeCapsuleStandard(TagBuffer* tag);
    void encodeCollisionVerts(TagBuffer* pTagBuf, StTag* tag, StSimMesh* pSimMesh);
    void encodeColIdInfo(TagBuffer* pTag);

private:
    std::ofstream*& m_data;
    std::shared_ptr<CSimObj> m_pSimObj;
};

