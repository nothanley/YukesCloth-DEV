#include "ClothSave.h"
#include <fstream>
#include "Encoder/ClothEncoder.h"
using namespace std;

CClothSave::CClothSave(const char* filePath, std::shared_ptr<CSimObj> pSimObj) {
	m_sFilePath = filePath;
	m_pSimObj = pSimObj;

	this->Save();
}

bool
CClothSave::Save() {
	if (!this->isFileOk()) {
		printf("\t{YCL Log} Cannot access YCL file");
		return false;
	}

	/* Converts and serializes simulation object into .ycl format.
	   Writes all data to specified out stream */
	CClothEncoder encoder(m_pDataStream, m_pSimObj);
	encoder.save();
    m_pDataStream->close();
}

bool
CClothSave::isFileOk() {
	if (this->m_pDataStream == nullptr)
		this->m_pDataStream = new std::ofstream(this->m_sFilePath, ios::binary);

	return m_pDataStream->good();
}

