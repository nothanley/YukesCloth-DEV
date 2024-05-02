#include "simmesh_pattern.h"
#include "cgamecloth.h"
#include "memoryreader.h"


CSimMeshPattern::CSimMeshPattern(CSimObj* obj, char*& data) :
	m_pSimObj(obj),
	m_data(data)
{}


void
CSimMeshPattern::loadSimMeshPattern(StSimMesh& sMesh) {
	if (!&sMesh) {
		printf("Could not parse skin pattern - Missing sim mesh destination.\n");
		return;
	}

	uint32_t numNodes = u32;
	sMesh.bSimPattern = s32;
}

void
CSimMeshPattern::loadSimMeshStacks(StSimMesh& sMesh) {
	uint32_t numProperties = u32;
}
