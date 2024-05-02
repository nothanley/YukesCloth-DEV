#include "clothfile.h"
#include "cgamecloth.h"
#include "winsock.h"
#include "common.h"
#include "memoryreader.h"


CClothContainer::CClothContainer(const char* path) :
	m_pClothSimObj(nullptr),
	m_filePath(path),
	m_fileBuffer(nullptr)
{}

CClothContainer::~CClothContainer()
{
	if (m_fileBuffer)
		delete m_fileBuffer;
}

void CClothContainer::open() 
{
	char* data = SysCommon::readBinaryFile(m_filePath);

	if (!data || !isValidContainer())
		throw std::runtime_error("Cannot load invalid YCL file.");

	this->ReadContents();
}

void CClothContainer::ReadContents() 
{
	printf("Opening File: %s\n", m_filePath.c_str());
	m_pClothSimObj = new CGameCloth(m_fileBuffer);
	m_pClothSimObj->load();
}

bool CClothContainer::isValidContainer()
{
	uint32_t signature = ReadUInt32(m_fileBuffer);

	return (ntohl(signature) != YCLHEAD);
}
