#include "ClothStructs.h"
#pragma once

class CSimObj;
struct TagBuffer;

class CSimEncodeSkin
{
public:
    CSimEncodeSkin(std::shared_ptr<CSimObj> obj, std::ofstream*& data);

public:
    void encodeSimMeshSkin(TagBuffer* sMesh);
    void encodeSkinCalc(TagBuffer* sMesh);
    void encodeSkinPaste(TagBuffer* sMesh);

public:
    void writeNodeSkinMatrices(TagBuffer* pTagBuf, MeshSkin& skin);
	void writeSkinPalette(TagBuffer* pTagBuf, std::vector<SimNode>& nodePalette);
	void writeSkin(TagBuffer* pTagBuf, MeshSkin skin, std::vector<SimNode>& nodePalette, const int& weights);

private:
    std::ofstream*& m_data;
    std::shared_ptr<CSimObj> m_pSimObj;
};

