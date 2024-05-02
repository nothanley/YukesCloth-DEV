/* Validates YANM file and initializes a stream upon validation success  */
#include <fstream>
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
	bool isValidContainer();

private:
	CGameCloth* m_pClothSimObj;
	std::string m_filePath;
	char* m_fileBuffer;
};

