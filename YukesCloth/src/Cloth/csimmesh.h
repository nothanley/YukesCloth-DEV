/* Defines all sim stream parsing and interpretation logic. */
#include "ClothStructs.h"
#include <fstream>
#include "../cloth_tags.h"
#pragma once

class CGameCloth;

enum GAME_TARGET {
    YUKES_CLOTH_22,
    YUKES_CLOTH_23,
    YUKES_CLOTH_24,
};

class CSimObj
{
public:
    const std::vector<SimNode>& getNodes() const { return m_nodeTable; }
    const std::vector<std::string>& stringTable() const { return m_stringTable; }
    const std::vector<CollisionVolume>& getCollisionVolumes() const { return m_colTable; }
    const std::string& getColId () const { return m_colId; }
    const StTag* getRootTag() const { return m_pStHead; }
    const SimNode& getNode(const int index) { return m_nodeTable.at(index); }
    const std::string& getString(const int index) { return m_stringTable.at(index); }
    StTag* FindTag(uint32_t enTagType); /* Non-Recursive root search */
    StTag* FindTag(uint32_t enTagType, StTag* pParent); /* Recursive relative search */

public:
    void clear_nodes() { m_nodeTable.clear(); }
    void setNumNodes(const int num) { m_nodeTable.resize(num); }
    void setColId(const std::string& name) { this->m_colId = name; }
    void addCol(const CollisionVolume& col) { m_colTable.push_back(col); }
    void addNode(const SimNode& node) { m_nodeTable.push_back(node); }
    void addRootChild(StTag* child_tag);
    void addString(const std::string& target);

public:
    virtual void initTag(StTag* tag) = 0;
    void loadStringTable();
    std::string loadString();
    uintptr_t pos = std::ios::beg;
    int game_format = YUKES_CLOTH_23;
   
protected:
    std::vector<std::string> m_stringTable;
    std::vector<SimNode> m_nodeTable;
    std::vector<CollisionVolume> m_colTable;
    std::string m_colId;
    StTag* m_pStHead;

protected:
    char* m_memOffset;
    char* m_data;
};



