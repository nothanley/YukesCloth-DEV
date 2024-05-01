/* Interprets Yukes Cloth container data and stores sim data in a Sim Object */
#pragma once
#include <vector>
#include <fstream>
#include "SimMesh.h"

class StHead;
class StTag;
class StSimMesh;
class SimNode;
class CollisionVolume;

static const std::vector<int> ROOT_NODES{ 1,2,5,6,9,19,23,25,27,31,32, /* New tags => */ 0x3E };

class CSimObj : public CSimMeshData
{

public:
	CSimObj();

	std::vector<std::string> m_sStringTable;
	std::vector<SimNode> m_NodeTable;
	std::vector<CollisionVolume> m_ColTable;
	std::string collisionID;
	StTag* m_pStHead;

protected:
	void InitTag(StTag& tag);
	void SetupTags(StTag* tag);
	StTag* GetTag(uintptr_t& streamBegin, StTag* pParentTag = nullptr);
	StTag* FindTag(uint32_t enTagType); /* Non-Recursive root search */
	StTag* FindTag(uint32_t enTagType, StTag* pParent); /* Recursive relative search */
	void InitializeNodePalette(const StTag& parent);
	void UpdateStrings();

};

