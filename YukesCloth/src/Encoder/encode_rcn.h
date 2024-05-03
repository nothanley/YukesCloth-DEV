#include "ClothStructs.h"
#pragma once

class CSimObj;
struct TagBuffer;

class CSimEncodeRCN
{
public:
    CSimEncodeRCN(std::shared_ptr<CSimObj> obj, std::ofstream*& data);

public:
    void encodeRCNData(TagBuffer* sMesh);
    void encodeRecalcNormals(TagBuffer* sMesh);
    void WriteRCNIndexBuffer(std::stringstream& pBf, StSimMesh* sMesh, uint32_t numElements);
    void WriteRCNNormalBuffer(std::stringstream& pBf, StSimMesh* sMesh, uint32_t numElements);

private:
    std::ofstream*& m_data;
    std::shared_ptr<CSimObj> m_pSimObj;
};
