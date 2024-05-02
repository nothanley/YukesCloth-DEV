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

private:
	void ReadContents();
	bool isValidContainer(char* data);

private:
	std::unique_ptr<CGameCloth> m_pClothObj;
	std::string m_filePath;
	char* m_data;
};

