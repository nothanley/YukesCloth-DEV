/* Validates YANM file and initializes a stream upon validation success  */
#include <BinaryIO.h>
#include <fstream>
#pragma once

using namespace std;
using namespace BinaryIO;

class CGameCloth;
class CClothContainer 
{
	enum enHeaderTags{
		YCLHEAD = 0x0
	};

public:
	CClothContainer(const char* FilePath);
	void open();

private:
	void ReadContents();
	void ValidateContainer();

	CGameCloth* m_pClothSimObj;
	std::ifstream* fs = nullptr;
	std::string m_sFilePath;
	uint64_t m_iFileSize;
	float m_fVersion;
	bool isOk = false;

};
