#include "ClothStructs.h"
#pragma once

class CSimObj;
struct TagBuffer;

class CSimEncodePattern
{
public:
    CSimEncodePattern(std::shared_ptr<CSimObj> obj, std::ofstream*& data);

public:
    void encodeSimMeshPattern(TagBuffer* sMesh);
    void encodeSimMeshStacks(TagBuffer* sMesh);

private:
    std::ofstream*& m_data;
    std::shared_ptr<CSimObj> m_pSimObj;
};
