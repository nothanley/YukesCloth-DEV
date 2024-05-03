#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <ostream>
#pragma once

struct StTag;
struct StSimMesh;
struct Vector4;
struct SimNode;
struct MeshSkin;
class CSimObj;


struct TagBuffer {
	StTag* tag;
	uint32_t sBegin = 0x10;
	uint32_t sSize;
	std::vector<TagBuffer*> children;
	bool bUsePreDef = false;

	std::stringstream binary =
		std::stringstream(
			std::ios::binary |
			std::ios::out);
};

namespace clothsrl
{
	void RegenerateStringTable(StTag* tag, std::shared_ptr<CSimObj>& pSimObj);
	void RegenerateNodeTable(StTag* tag, std::shared_ptr<CSimObj>& pSimObj);
	int FindNodeIndex(const std::string& sTarget, std::vector<SimNode> nodeTable);
	int FindStringIndex(const std::string& sTarget, std::vector<std::string> stringTable);
	void AlignStream(std::stringstream& stream, const int& width, const uintptr_t& offset = 0);
	void AlignTagHeader(TagBuffer* pTagBuf);
	int GetConstraintIndex(StSimMesh* pSimMesh, const std::string& target);
	int FindMatrixIndex(StSimMesh* pSimMesh, const Vector4& target);
	int FindNodeMatrixIndex(const SimNode& sTarget, std::vector<SimNode> nodeTable);
	std::vector<SimNode> getNodePalette(MeshSkin& skin, std::shared_ptr<CSimObj>& pSimObj, int& numWeights);
};

