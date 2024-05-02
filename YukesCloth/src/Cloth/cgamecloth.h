/* Interprets Yukes Cloth container data and stores sim data in a Sim Object */
#include "csimmesh.h"
#include "YukesSimMesh"
#include <memory>
#pragma once

#ifdef DEBUG_CONSOLE
#include <iostream> 
#endif 

class CGameCloth : public CSimObj
{
public:
	CGameCloth(char* data);
	void load();

protected:
	void InitTag(StTag& tag);
	void SetupTags(StTag* tag);
	StTag* GetTag(StTag* pParentTag = nullptr);
	StTag* FindTag(uint32_t enTagType); /* Non-Recursive root search */
	StTag* FindTag(uint32_t enTagType, StTag* pParent); /* Recursive relative search */
	void InitializeNodePalette(const StTag& parent);
	void UpdateStrings();

private:
	void setupSimHandles();

private:
	std::unique_ptr<CSimMeshCol>        m_colHandler;
	std::unique_ptr<CSimMeshConstraint> m_constHandler;
	std::unique_ptr<CSimMeshDef>        m_defHandler;
	std::unique_ptr<CSimMeshSubObj>     m_subobjHandler;
	std::unique_ptr<CSimMeshPattern>    m_patternHandler;
	std::unique_ptr<CSimMeshSkin>       m_skinHandler;
	std::unique_ptr<CSimSubObj_RCN>     m_rcnHandler;
};

