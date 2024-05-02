#include "simmesh_constraint.h"
#include "cgamecloth.h"
#include "memoryreader.h"

CSimMeshConstraint::CSimMeshConstraint(CSimObj* obj, char*& data) :
    m_pSimObj(obj),
    m_data(data)
{}

void CSimMeshConstraint::loadForce(StSimMesh& sMesh)
{
	uint32_t numVerts = u32;
	sMesh.force.parameters = std::vector<float>{ f32, f32, f32, f32 };
	m_data = (char*)m_pSimObj->pos + 0x30;

	for (int i = 0; i < numVerts; i++)
	{
		sMesh.force.data.push_back(SkinVertex{ u32,f32 });
	}
}

void CSimMeshConstraint::loadConstraintStretchLink(StSimMesh& sMesh)
{
	uint32_t numLinks = u32;
	m_data = (char*)m_pSimObj->pos + 0x10;
	SimConstraint stretchConstraint{ "Stretch", enTagType_SimMesh_CtStretchLink };

	for (int i = 0; i < numLinks; i++)
	{
		EdgeConstraint edgeLink;
		edgeLink.vertices.x.index = u32;
		edgeLink.vertices.y.index = u32;

		edgeLink.vertices.x.weight = f32;
		edgeLink.vertices.y.weight = f32;;

		stretchConstraint.data.push_back(edgeLink);
	}

	sMesh.constraints.push_back(stretchConstraint);
}


void CSimMeshConstraint::loadConstraintStandardLink(StSimMesh& sMesh)
{

	SimConstraint constraint{ "Standard", enTagType_SimMesh_CtStdLink };
	uint32_t numLinks = u32;
	constraint.parameter = u32;
	m_data = (char*)m_pSimObj->pos + 0x20;

	for (int i = 0; i < numLinks; i++)
	{
		EdgeConstraint edgeLink;
		edgeLink.vertices.x.index = u16;
		edgeLink.vertices.y.index = u16;

		edgeLink.vertices.x.weight = f32;
		edgeLink.vertices.y.weight = f32;
		edgeLink.vertices.z.weight = f32;;

		constraint.data.push_back(edgeLink);
	}

	sMesh.constraints.push_back(constraint);
}

void CSimMeshConstraint::loadConstraintBendLink(StSimMesh& sMesh)
{
	SimConstraint constraint{ "Bend", enTagType_SimMesh_CtBendLink };
	uint32_t numLinks = u32;
	constraint.parameter = u32;
	m_data = (char*)m_pSimObj->pos + 0x20;

	for (int i = 0; i < numLinks; i++)
	{
		EdgeConstraint edgeLink;
		edgeLink.vertices.x.index = u16;
		edgeLink.vertices.y.index = u16;

		edgeLink.vertices.x.weight = f32;
		edgeLink.vertices.y.weight = f32;;
		edgeLink.vertices.z.weight = f32;;
		constraint.data.push_back(edgeLink);
	}

	sMesh.constraints.push_back(constraint);
}

void CSimMeshConstraint::loadBendStiffness(StSimMesh& sMesh)
{
	SimConstraint constraint{ "BendStiffness", enTagType_SimMesh_BendingStiffness };
	uint32_t numTris = u32;
	constraint.parameter = u32;
	float unkFloat = f32;
	m_data = (char*)m_pSimObj->pos + 0x20;

	for (int i = 0; i < numTris; i++) {
		FaceConstraint face;

		face.x.index = u16;
		face.y.index = u16;
		face.z.index = u32;

		face.weights.push_back(f32);
		face.weights.push_back(f32);
		face.weights.push_back(f32);
		face.weights.push_back(f32);

		constraint.faceData.push_back(face);
	}

	sMesh.constraints.push_back(constraint);
}

void CSimMeshConstraint::loadConstraintFixation(StSimMesh& sMesh)
{
	uint32_t numVerts = u32;
	m_data = (char*)m_pSimObj->pos + 0x10;
	SimConstraint constraint{ "Fixation", enTagType_SimMesh_CtFixation };

	for (int i = 0; i < numVerts; i++)
	{
		EdgeConstraint edge;
		edge.vertices.x.index = u32;
		edge.vertices.x.weight = f32;
		constraint.data.push_back(edge);
	}

	sMesh.constraints.push_back(constraint);
}

