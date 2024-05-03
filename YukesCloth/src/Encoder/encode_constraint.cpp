#include "encode_constraint.h"
#include "cgamecloth.h"
#include "memoryreader.h"
#include "clothencoder.h"

using namespace clothsrl;

CSimEncodeConstraint::CSimEncodeConstraint(std::shared_ptr<CSimObj> obj, std::ofstream*& data) :
    m_pSimObj(obj),
    m_data(data)
{}

void CSimEncodeConstraint::encodeForce(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	uint32_t numVerts = pSimMesh->force.data.size();
	WriteUInt32(pTagBuf->binary, numVerts);

	for (auto& param : pSimMesh->force.parameters) {
		WriteFloat(pTagBuf->binary, param);
	}
	WriteUInt32(pTagBuf->binary, 0x0);
	AlignTagHeader(pTagBuf);

	for (auto& data : pSimMesh->force.data) {
		WriteUInt32(pTagBuf->binary, data.index);
		WriteFloat(pTagBuf->binary, data.weight);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void CSimEncodeConstraint::encodeConstraintStretchLink(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	int ctIdx = GetConstraintIndex(pSimMesh, "Stretch");
	if (ctIdx == -1) return;

	SimConstraint stretchConstraint = pSimMesh->constraints.at(ctIdx);
	uint32_t numLinks = stretchConstraint.data.size();

	WriteUInt32(pTagBuf->binary, numLinks);
	AlignTagHeader(pTagBuf);

	for (auto& link : stretchConstraint.data) {
		WriteUInt32(pTagBuf->binary, link.vertices.x.index);
		WriteUInt32(pTagBuf->binary, link.vertices.y.index);
		WriteFloat(pTagBuf->binary, link.vertices.x.weight);
		WriteFloat(pTagBuf->binary, link.vertices.y.weight);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void CSimEncodeConstraint::encodeConstraintStandardLink(TagBuffer* pTagBuf) 
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	int ctIdx = GetConstraintIndex(pSimMesh, "Standard");
	if (ctIdx == -1) return;

	SimConstraint constraint = pSimMesh->constraints.at(ctIdx);
	uint32_t numLinks = constraint.data.size();

	WriteUInt32(pTagBuf->binary, numLinks);
	WriteUInt32(pTagBuf->binary, constraint.parameter);
	AlignTagHeader(pTagBuf);

	for (auto& link : constraint.data) {
		WriteUInt16(pTagBuf->binary, link.vertices.x.index);
		WriteUInt16(pTagBuf->binary, link.vertices.y.index);
		WriteFloat(pTagBuf->binary, link.vertices.x.weight);
		WriteFloat(pTagBuf->binary, link.vertices.y.weight);
		WriteFloat(pTagBuf->binary, link.vertices.z.weight);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void CSimEncodeConstraint::encodeConstraintBendLink(TagBuffer* pTagBuf)
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	int ctIdx = GetConstraintIndex(pSimMesh, "Bend");
	if (ctIdx == -1) return;

	SimConstraint constraint = pSimMesh->constraints.at(ctIdx);
	uint32_t numLinks = constraint.data.size();

	WriteUInt32(pTagBuf->binary, numLinks);
	WriteUInt32(pTagBuf->binary, constraint.parameter);
	AlignTagHeader(pTagBuf);

	for (auto& link : constraint.data) {
		WriteUInt16(pTagBuf->binary, link.vertices.x.index);
		WriteUInt16(pTagBuf->binary, link.vertices.y.index);
		WriteFloat(pTagBuf->binary, link.vertices.x.weight);
		WriteFloat(pTagBuf->binary, link.vertices.y.weight);
		WriteFloat(pTagBuf->binary, link.vertices.z.weight);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void CSimEncodeConstraint::encodeConstraintFixation(TagBuffer* pTagBuf) 
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	int ctIdx = GetConstraintIndex(pSimMesh, "Fixation");
	if (ctIdx == -1) return;

	SimConstraint constraint = pSimMesh->constraints.at(ctIdx);
	uint32_t numLinks = constraint.data.size();

	WriteUInt32(pTagBuf->binary, numLinks);
	AlignTagHeader(pTagBuf);

	for (auto& link : constraint.data) {
		WriteUInt32(pTagBuf->binary, link.vertices.x.index);
		WriteFloat(pTagBuf->binary, link.vertices.x.weight);
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

void CSimEncodeConstraint::encodeBendStiffness(TagBuffer* pTagBuf) 
{
	const StTag* tag = pTagBuf->tag;
	StSimMesh* pSimMesh = tag->pSimMesh;

	int ctIdx = GetConstraintIndex(pSimMesh, "BendStiffness");
	if (ctIdx == -1) return;

	SimConstraint constraint = pSimMesh->constraints.at(ctIdx);
	uint32_t numLinks = constraint.faceData.size();

	WriteUInt32(pTagBuf->binary, numLinks);
	WriteUInt32(pTagBuf->binary, constraint.parameter);
	WriteFloat(pTagBuf->binary, 1.0f);
	AlignTagHeader(pTagBuf);

	for (auto& link : constraint.faceData) {
		WriteUInt16(pTagBuf->binary, link.x.index);
		WriteUInt16(pTagBuf->binary, link.y.index);
		WriteUInt32(pTagBuf->binary, link.z.index);

		WriteFloat(pTagBuf->binary, link.weights.at(0));
		WriteFloat(pTagBuf->binary, link.weights.at(1));
		WriteFloat(pTagBuf->binary, link.weights.at(2));
		WriteFloat(pTagBuf->binary, link.weights.at(3));
	}
	AlignStream(pTagBuf->binary, BINARY_ALIGNMENT, 0xC);
}

