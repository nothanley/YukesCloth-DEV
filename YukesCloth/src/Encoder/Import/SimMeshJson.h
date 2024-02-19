#include <Cloth/ClothStructs.h>
#include <Json.hpp>
#include <cstdint>
#include <functional>
#include <map>
#pragma once

class CSimObj;
class StTag;

enum {
	SIM_MESH_SOURCE,
	SIM_LINE,
	SIM_MESH_TARGET
};

class CJsonSimMesh
{
public:
	CJsonSimMesh(const char* filePath);

public:
    StTag* GetMeshTag(){
        return this->m_pTagHead;
    }

private:
	void ReadJson(nlohmann::json& json, const char* filePath);
	void LoadSimMesh();

private:
	StTag* BuildNewTag(const uint32_t& type, const char* name);
	void Load_AssignSubObj(StTag* pTagParent);
	void Load_AssignSubObjVtx(StTag* pTagParent);
	void Load_AssignSimVtx(StTag* pTagParent);
	void Load_Skin(StTag* pTagParent);
	void Load_SkinVerts(StSimMesh* pSimMesh);
	void Load_SkinWeights(StSimMesh* pSimMesh, uint32_t& maxBlendWeights);
	void Load_SimLinkSrc(StTag* pTagParent);
	void Load_Pattern(StTag* pTagParent);
	void Load_Stacks(StTag* pTagParent);
	void Load_RCN(StTag* pTagParent);
	void Load_RCNSubObj(StTag* pTagParent);

private:
	void Load_SkinCalc(StTag* pTagParent);
	void Load_SkinPaste(StTag* pTagParent);
	void Load_OldVtxSave(StTag* pTagParent);
	void Load_Force(StTag* pTagParent);
	void Load_Constraints(StTag* pTagParent);

private:
	void Load_CtStdLink(StTag* pTagParent);
	void Load_CtBendLink(StTag* pTagParent);
	void Load_CtStretchLink(StTag* pTagParent);
	void Load_CtFixation(StTag* pTagParent);
	void Load_ColVerts(StTag* pTagParent);

	std::map<std::string, std::function<void(StTag*)>> constraintLoaders =
	{
		{"Standard", std::bind(&CJsonSimMesh::Load_CtStdLink, this, std::placeholders::_1)},
		{"Bend", std::bind(&CJsonSimMesh::Load_CtBendLink, this, std::placeholders::_1)},
		{"Stretch", std::bind(&CJsonSimMesh::Load_CtStretchLink, this, std::placeholders::_1)},
		{"Fixation", std::bind(&CJsonSimMesh::Load_CtFixation, this, std::placeholders::_1)}
	};

private:
	int meshType = SIM_MESH_SOURCE;
	StTag* m_pTagHead = nullptr;
	StSimMesh* m_pSMesh = nullptr;
	std::string m_sJsonPath;
	nlohmann::json m_Json;
};

