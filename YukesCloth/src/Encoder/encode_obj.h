#include "ClothStructs.h"
#pragma once

class CSimObj;
struct TagBuffer;

class CSimEncodeSubObj
{
public:
    CSimEncodeSubObj(std::shared_ptr<CSimObj> obj, std::ofstream*& data);

public:
    void encodeSubObj(TagBuffer* sMesh);
    void encodeSubObjVtx(TagBuffer* sMesh);
    void encodeSimVtx(TagBuffer* sMesh);
    void encodeSaveVtx(TagBuffer* sMesh);
    void encodeLinkTar(TagBuffer* sLine);
    void encodeLinkTar_2024(TagBuffer* sLine);
    void encodeSimLinkSource(TagBuffer* sMesh);
    void encodeCollection(TagBuffer* sMesh);

private:
    std::ofstream*& m_data;
    std::shared_ptr<CSimObj> m_pSimObj;
};

