#include "ClothContainer.h"
#include "../CGameCloth.h"
#include "winsock.h"

using namespace std;
using namespace BinaryIO;


CClothContainer::CClothContainer(const char* filePath) :
	m_pClothSimObj(nullptr),
	m_sFilePath(filePath)
{
}

static uint64_t GetFileBufferSize(std::filebuf* buffer) {
	buffer->pubseekoff(0, std::ios::end);
	std::streampos size = buffer->pubseekoff(0, std::ios::cur);
	return uint64_t(size);
}

void 
CClothContainer::open() 
{
	if (!fs)
		this->fs = new std::ifstream(this->m_sFilePath, ios::binary);

	if (!fs->good())
		throw std::runtime_error("Cannot read YCL stream.");

	this->ValidateContainer();

	if (!this->isOk)
		throw std::runtime_error("Cannot load invalid YCL file.");

	this->ReadContents();
	fs->close();
}

void
CClothContainer::ReadContents() {
	printf("Opening File: %s\n", m_sFilePath.c_str());

	m_pClothSimObj = new CGameCloth(fs);
	m_pClothSimObj->load();
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
