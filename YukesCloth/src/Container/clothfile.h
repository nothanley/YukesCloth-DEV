/* Validates YANM file and initializes a stream upon validation success  */
#include <fstream>
#include <memory>
#pragma once

enum enHeaderTags {
	YCLHEAD = 0x0
};

class CGameCloth;

class CClothContainer 
{
public:
	CClothContainer(const char* FilePath);
	~CClothContainer();
	void open();
    std::shared_ptr<CGameCloth>& getClothObj(){ return m_pClothObj;}

private:
	void ReadContents();
	bool isValidContainer(char* data);

private:
    std::shared_ptr<CGameCloth> m_pClothObj;
	std::string m_filePath;
	char* m_data;
};

