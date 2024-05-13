#include "simmesh_col.h"
#include "cgamecloth.h"
#include "memoryreader.h"

CSimMeshCol::CSimMeshCol(CSimObj* obj, char*& data) :
	m_pSimObj(obj),
	m_data(data)
{}


void CSimMeshCol::loadCols()
{
	uint32_t numChildren = ReadUInt32(m_data);
	int32_t unkVal = ReadUInt32(m_data);
}

void CSimMeshCol::loadColIDTbl()
{
	uint32_t numChildren = u32;
	uint32_t numColliders = u32;
	m_pSimObj->pos = uintptr_t(m_data) + 0xC;
	m_data = (char*)(m_pSimObj->pos);

	for (int i = 0; i < numColliders; i++) {
		std::string name = m_pSimObj->loadString();
		m_pSimObj->setColId(name);
	}
}


void CSimMeshCol::loadColPack() {
	uint32_t numCapsules = u32;
	uint32_t pad = u32;
	uint32_t packType = u32;
}

void CSimMeshCol::loadCapsuleStandard(StTag& tag)
{
	int numNodes = m_pSimObj->getNodes().size();
	CollisionVolume capsule;
	capsule.indexA = u32;
	capsule.indexB = u32;

	capsule.weights = std::vector<float>
	{ f32, f32, f32, f32, f32, f32, f32, f32 };

	capsule.id = u32;
	if (capsule.indexB >= numNodes) {
		printf("\t{YCL Log} Invalid capsule join. Could not get node table index.\n");
		capsule.indexB = numNodes - 1;
	}

	capsule.name  = m_pSimObj->getString(capsule.indexA);
	capsule.joint = m_pSimObj->getNode(capsule.indexB).name;

	m_pSimObj->addCol(capsule);
	tag.col = capsule;
}

void CSimMeshCol::loadCapsuleTapered(StTag& tag)
{
	int numNodes = m_pSimObj->getNodes().size();
	CollisionVolume capsule;
	capsule.indexA = u32;
	capsule.indexB = u32;

	capsule.weights = std::vector<float>
	{ f32, f32, f32, f32, f32, f32, f32, f32 };

	capsule.radius = f32;
	capsule.id     = u32;

	if (capsule.indexB >= numNodes) {
		printf("Invalid capsule join. Could not get node table index.\n");
		capsule.indexB = numNodes - 1;
	}

	capsule.name  = m_pSimObj->getString(capsule.indexA);
	capsule.joint = m_pSimObj->getNode(capsule.indexB).name;

	m_pSimObj->addCol(capsule);
	tag.col = capsule;
}


void CSimMeshCol::loadCollisionVerts(StTag& tag, StSimMesh& sMesh) 
{
	CollisionVerts colVtxBf;
	colVtxBf.tagSize = tag.sSize;
	colVtxBf.unkFlag = s32;
	m_data = (char*)m_pSimObj->pos + 0x30;
	
	colVtxBf.unkVal   = s32;
	colVtxBf.numItems = s32;
	colVtxBf.numVerts = s32;
	colVtxBf.unkFlagB = s32;
	m_data = (char*)m_pSimObj->pos + tag.sSize;
	
	for (int i = 0; i < colVtxBf.numItems; i++){
		Points segment;
		segment.x.index  = u32;
		segment.y.index  = u32;
		segment.x.weight = f32;
		segment.y.weight = f32;
	
		colVtxBf.items.push_back(segment);
	}
	
	for (int i = 0; i < colVtxBf.numVerts; i++){
		colVtxBf.indices.push_back(u32);
	}
	
	tag.vtxCol_index = tag.pSimMesh->vtxColGroups.size();
	sMesh.vtxColGroups.push_back(colVtxBf);
}


