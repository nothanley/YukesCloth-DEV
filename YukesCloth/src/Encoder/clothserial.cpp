#include "clothserial.h"
#include "YukesCloth"
#include "BinaryIO.h"
#include "YukesSimMesh"

using namespace BinaryIO;

void clothsrl::RegenerateStringTable(StTag* tag, std::shared_ptr<CSimObj>& pSimObj)
{
	if (tag->eType == enTagType_StrTbl) {
		tag->children.clear();
		for (auto& str : pSimObj->stringTable()) {
			StTag* child = new StTag;
			child->eType = enTagType_String;
			child->pParent = tag;
			child->sTagName = str;
			tag->children.push_back(child);
		}
	}
}

void clothsrl::RegenerateNodeTable(StTag* tag, std::shared_ptr<CSimObj>& pSimObj)
{
	if (tag->eType == enTagType_NodeTbl) {
		tag->children.clear();
		for (auto& node : pSimObj->getNodes()) {
			StTag* child = new StTag;
			child->eType = enTagType_String;
			child->pParent = tag;
			child->sTagName = node.name;
			tag->children.push_back(child);
		}
	}
}


int clothsrl::FindNodeIndex(const std::string& sTarget, std::vector<SimNode> nodeTable)
{
	for (int i = 0; i < nodeTable.size(); i++) {
		if (sTarget == nodeTable.at(i).name) {
			return i;
		}
	}

	return -1;
}

int clothsrl::FindStringIndex(const std::string& sTarget, std::vector<std::string> stringTable)
{
	for (int i = 0; i < stringTable.size(); i++) {
		if (sTarget == stringTable.at(i)) {
			return i;
		}
	}

	return -1;
}

void clothsrl::AlignStream(std::stringstream& stream, const int& width, const uintptr_t& offset) {
	while ((uint32_t(stream.str().size()) + offset) % width != 0) {
		WriteByte(stream, 0x0);
	}
}

void clothsrl::AlignTagHeader(TagBuffer* pTagBuf) {
	AlignStream(pTagBuf->binary, 16, 0xC);
	pTagBuf->sBegin = 0xC + pTagBuf->binary.tellp();
}

int clothsrl::GetConstraintIndex(StSimMesh* pSimMesh, const std::string& target) {
	int ctIdx = -1;
	for (int i = 0; i < pSimMesh->constraints.size(); i++) {
		std::string type = pSimMesh->constraints.at(i).name;

		if (type == target)
			return i;
	}

	return ctIdx;
}

int clothsrl::FindMatrixIndex(StSimMesh* pSimMesh, const Vector4& target) {

	int numMatrices = pSimMesh->skin.vertices.size();
	for (int i = 0; i < numMatrices; i++) {
		Vector4 skinVtx = pSimMesh->skin.vertices.at(i);
		bool isMatching = false;

		if (skinVtx == target)
			return i;
	}

	return -1;
}

int clothsrl::FindNodeMatrixIndex(const SimNode& sTarget, std::vector<SimNode> nodeTable)
{
	for (int i = 0; i < nodeTable.size(); i++) {
		SimNode nodeItem = nodeTable.at(i);

		if (sTarget.name == nodeItem.name &&
			sTarget.vecf == nodeItem.vecf) {
			return i;
		}
	}

	return -1;
}

std::vector<SimNode> clothsrl::getNodePalette(MeshSkin& skin, std::shared_ptr<CSimObj>& pSimObj, int& numWeights)
{
	std::vector<SimNode> nodePalette;
	for (auto& weight : skin.blendWeights)
	{
		if (weight.numWeights > numWeights) {
			numWeights = weight.numWeights;
		}

		for (auto& bone : weight.bones)
		{
			int paletteIndex = FindNodeIndex(bone, nodePalette);
			int nodeIndex = FindNodeIndex(bone, pSimObj->getNodes());
			if (paletteIndex == -1 && nodeIndex != -1) {
				nodePalette.push_back(pSimObj->getNodes().at(nodeIndex));
			}
		}
	}
	return nodePalette;
}

