#include "Cloth/SimObj.h"
#pragma once

class CGameCloth : public CSimObj
{

public:
	CGameCloth(std::ifstream* fs);

	void load();

};


