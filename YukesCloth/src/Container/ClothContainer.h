/* Validates YANM file and initializes a stream upon validation success  */
#include <BinaryIO.h>
#include <fstream>
#pragma once

using namespace std;
using namespace BinaryIO;

class CSimObj;
class CClothContainer {
	enum {
		YCLHEAD = 0x0
	};

public:
	CClothContainer(const char* FilePath) {
		this->m_sFilePath = FilePath;
		Load();
		fs->close();
	}

	CSimObj* m_pClothSimObj = nullptr;

private:
	void Load();
	void ReadContents();
	void ValidateContainer();

	std::ifstream* fs = nullptr;
	std::string m_sFilePath;
	uint64_t m_iFileSize;
	float m_fVersion;
	bool isOk = false;

};
