#include <vector>
#include <string>
#include <memory>
#pragma once

struct Vector4 {
	float x, y, z, w;
};

struct MeshPolygon {
	std::vector<uint32_t> points;
};

struct Triangle {
	uint32_t point0, point1, point2;
};

struct SkinVertex {
	uint32_t index;
    float weight = 1.0;
};

struct Points {
	SkinVertex x, y, z;
};

struct CollisionVolume {
	int id;
	std::string name;
	std::string joint;
	int32_t indexA, indexB, indexC;
	uint32_t unkVal0, unkVal1, unkVal2;
	std::vector<float> weights;
	float radius;
};

struct SimNode {
    uint32_t index;
    std::string name;
    Vector4 vecf;
};

struct NodeLink {
    uint32_t begin_index;
    uint32_t end_index;
    std::vector<SimNode> link;
};

struct LineDef {
	uint32_t sSize;
	uint32_t index;
	std::vector<NodeLink> links;
};

struct EdgeConstraint {
	Points vertices;
};

struct FaceConstraint {
	SkinVertex x, y, z;
	std::vector<float> weights;
};

struct RCNData {
	uint32_t numFaceNrmVtxIndices;
	uint32_t numVtxNrmFaceLinks;

	std::vector<uint32_t> parameters;
	std::vector<uint8_t> degCounts;
	std::vector<uint8_t> linkCounts;
	std::vector<uint32_t> faceNrmVtxIndices;
	std::vector<SkinVertex> vtxNrmFaceLinks;

	std::vector<MeshPolygon> faces;
};

struct SimConstraint {
	std::string name;
	uint32_t type;
	std::vector<EdgeConstraint> data;
	std::vector<FaceConstraint> faceData;
	uint32_t parameter;
};

struct ForceField {
	std::vector<float> parameters;
	std::vector<SkinVertex> data;
};

struct CollisionVerts {
	int32_t unkFlag;
	uint32_t unkVal;
	uint32_t numItems;
	uint32_t numVerts;
	int32_t unkFlagB;
	uint32_t tagSize;

	char* debug_binary = nullptr;

	std::vector<Points> items;
	std::vector<uint32_t> indices;
};

struct MeshWeight {
	int numWeights;
	std::vector<int> indices;
	std::vector<std::string> bones;
	std::vector<float> weights;
};

struct MeshSkin {
	std::vector<Vector4> vertices;
	std::vector<Vector4> normals;
	std::vector<MeshWeight> blendWeights;
	std::vector<SimNode> nodePalette;
};

struct LinkTarget {
	std::string source;
	int totalWeights;

	std::vector<uint32_t> indices;
	std::vector<uint32_t> palette;
	std::vector<MeshWeight> weights;

	std::vector<Triangle> sourceTris;
	std::vector<Vector4> vertexTable;
};


struct StSimMesh {
	uint32_t numTags;
	uint32_t sNameIdx;
	uint32_t sSimVtxCount;
	uint32_t sIterationCount;

	bool bCalcNormal;
	bool bDispObject;
	int32_t bSimPattern;

	bool bIsSimLine;
    int sMeshIndex;
    int sObjIndex;

	std::string sModelName;
	std::string sObjName;

	std::vector<uint32_t> sObjVerts;
	std::vector<uint32_t> simVerts;
	std::vector<uint32_t> saveVerts;
	std::vector<uint32_t> skinPaste;
    std::vector<Vector4> skinCalc;
	std::vector<Triangle> sourceEdges;
    std::vector<SimConstraint> constraints;
	std::vector<SimNode> nodePalette;
	std::vector<CollisionVerts> vtxColGroups;

	MeshSkin skin;
	RCNData rcn;
	ForceField force;
	LineDef lines;
	LinkTarget target;

#ifdef DEBUG_EDITOR
    std::vector<uint32_t> skin_calc_indices;
#endif
};

struct StTag {
	uint32_t eType;
	uint32_t sSize;
	uint32_t sTotalSize;
	uint32_t sTagCount;

	StSimMesh* pSimMesh = nullptr;
	StTag* pParent = nullptr;
	uintptr_t streamPointer;
	CollisionVolume col;
	int16_t copy_index = -1;
	int16_t vtxCol_index = -1;

	std::string sTagName;
	std::vector<StTag*> children;

#ifdef DEBUG_EDITOR
    std::vector<uint8_t> data;
#endif
};

static bool operator==(const Vector4& vec_a, const Vector4& vec_b) {
	bool isMatching = (vec_a.x == vec_b.x && vec_a.y == vec_b.y &&
		vec_a.z == vec_b.z && vec_a.w == vec_b.w);

	return isMatching;
}

static Vector4 operator-(const Vector4& vec_a, const Vector4& vec_b) {
	Vector4 result{ vec_a.x - vec_b.x, vec_a.y - vec_b.y,
					vec_a.z - vec_b.z, vec_a.w - vec_b.w };

	return result;
}


namespace yclutils {
	void debugPrintFloats(std::vector<Vector4>* floatArray);
	bool hasIndex(const std::vector<int>& vec, int target);
    const std::string GetNodeName(const uint32_t &tagEnum);
};











