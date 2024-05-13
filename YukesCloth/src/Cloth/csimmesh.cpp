#include "csimmesh.h"
#include "cgamecloth.h"
#include "memoryreader.h"
#include <algorithm>

void CSimObj::loadStringTable()
{
	m_data = (char*)pos + 0x8;
	uint32_t numNodes = u32;
	uint32_t numStrings = u32;
	this->pos += 0x20;

	printf("\t{YCL Log} Total Strings: %d\n", numStrings);
	m_data = (char*)this->pos;

	for (int i = 0; i < numStrings; i++) {
		std::string item = loadString();
		m_stringTable.push_back(item);
	}
}

std::string CSimObj::loadString()
{
	m_data = (char*)this->pos;
	uint32_t eTagType = u32;
	uint32_t sSize = u32;
	uint32_t sTotalSize = u32;
	uint32_t numChildNodes = u32;

	int charBufferSize = sTotalSize - 0x10;
	std::string item = ReadString(m_data, charBufferSize);

	this->pos += sTotalSize;
	return item;
}

float GetVectorDistance(Vector4& vec_a, Vector4& vec_b) {
	float a = (vec_a.x - vec_b.x);
	float b = (vec_a.y - vec_b.y);
	float c = (vec_a.z - vec_b.z);

	return sqrtf((a * a) + (b * b) + (c * c));
}


StTag* CSimObj::FindTag(uint32_t enTagType)
{
	for (auto child : m_pStHead->children) {
		if (child->eType == enTagType) {
			return child;
		}
	}
	return nullptr;
}

StTag* CSimObj::FindTag(uint32_t enTagType, StTag* pParent)
{
	StTag* result = nullptr;

	for (auto child : pParent->children) {
		if (child->eType == enTagType) {
			return child;
		}

		result = FindTag(enTagType, child);

		if (result)
			break;
	}

	return result;
}


void CSimObj::addRootChild(StTag* child_tag) {
	auto children = m_pStHead->children;
	children.insert(children.begin(), child_tag);
}

void CSimObj::addString(const std::string& target) {
	for (const auto& str : m_stringTable)
		if (str == target) return;

	m_stringTable.push_back(target);
}

