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
	void initTag(StTag* tag);
	void setupTags(StTag* tag);
	StTag* getTag(StTag* pParentTag = nullptr);
	void updateStrings();

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

