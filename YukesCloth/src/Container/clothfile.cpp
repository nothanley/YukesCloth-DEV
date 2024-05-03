#include "clothfile.h"
#include "cgamecloth.h"
#include "winsock.h"
#include "common.h"
#include "memoryreader.h"


CClothContainer::CClothContainer(const char* path) :
	m_pClothObj(nullptr),
	m_filePath(path),
	m_data(nullptr)
{}

CClothContainer::~CClothContainer()
{
	if (m_data) 
		delete m_data;
}

void CClothContainer::open() 
{
	m_data = SysCommon::readBinaryFile(m_filePath);

	if (!m_data)
		throw std::runtime_error("Cannot load invalid YCL file.");

	if (this->isValidContainer(m_data))
		ReadContents();
}

void CClothContainer::ReadContents() 
{
	printf("Opening File: %s\n", m_filePath.c_str());
	m_pClothObj = std::make_shared<CGameCloth>(m_data);
	m_pClothObj->load();
}

bool CClothContainer::isValidContainer(char* data)
{
	uint32_t signature = ReadUInt32(data);
	return (ntohl(signature) == YCLHEAD);
}
