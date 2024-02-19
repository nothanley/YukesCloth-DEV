#include "ClothContainer.h"
#include "../Cloth/SimObj.h"
#include "winsock.h"

using namespace std;
using namespace BinaryIO;


static uint64_t GetFileBufferSize(std::filebuf* buffer) {
	buffer->pubseekoff(0, std::ios::end);
	std::streampos size = buffer->pubseekoff(0, std::ios::cur);
	return uint64_t(size);
}

void 
CClothContainer::Load() {
	if (this->fs == nullptr)
		this->fs = new std::ifstream(this->m_sFilePath, ios::binary);
	if (!fs->good())
		throw("Cannot read YCL stream.");
	ValidateContainer();

	if (this->isOk) { ReadContents(); }
    else{ throw("Invalid YCL file."); }

	fs->close();
}

void
CClothContainer::ReadContents() {
	printf("Opening File: %s\n", m_sFilePath.c_str());

	m_pClothSimObj = new CSimObj(fs);
	m_pClothSimObj->Create();
}

void 
CClothContainer::ValidateContainer() {
	fs->seekg(ios::beg);
	uint32_t signature = ReadUInt32(*fs);
	this->m_iFileSize = GetFileBufferSize(fs->rdbuf());

	// Validates type and version
	if (ntohl(signature) == YCLHEAD)
		this->isOk = true;

	/* Reset stream pointer */
	fs->seekg(ios::beg);
}
