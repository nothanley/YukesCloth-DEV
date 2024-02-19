/* Interprets Yukes Cloth container data and stores sim data in a Sim Object */
#include <fstream>
#pragma once

class CSimObj;
class CClothSave
{

public:
	CClothSave(const char* filePath, CSimObj* pSimObj);

public:
	static void SaveToDisk(const char* filePath, CSimObj* pSimObj)
	{
		CClothSave saveFile(filePath, pSimObj);
	}

private:
	bool Save();
	bool isFileOk();

private:
	std::string m_sFilePath;
	std::ofstream* m_pDataStream = nullptr;
	CSimObj* m_pSimObj = nullptr;


};
