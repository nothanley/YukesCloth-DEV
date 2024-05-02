#include "simmesh_def.h"
#include "cgamecloth.h"
#include "memoryreader.h"


CSimMeshDef::CSimMeshDef(CSimObj* obj, char*& data) :
	m_pSimObj(obj),
	m_data(data)
{}

void CSimMeshDef::GetSimMesh(StTag& parent) 
{
	StSimMesh* _SimMesh = new StSimMesh{ u32, u32, u32, u32, (bool)u32, (bool)u32 };

#ifdef DEBUG_CONSOLE
	//printf("Pos: %d\n", uint64_t(this->m_pDataStream->tellg()) );
	printf("\n\t\t- StSimMesh {sTags: %d, sName:%d, sSimVtxCount:%d, sIterationCount:%d, bCalcNormal: %s, bDispObject: %s}\n",
		_SimMesh->numTags, _SimMesh->sNameIdx, _SimMesh->sSimVtxCount,
		_SimMesh->sIterationCount, _SimMesh->bCalcNormal ? "true" : "false",
		_SimMesh->bDispObject ? "true" : "false");
#endif

	/* Assign sim mesh pointer */
	parent.pSimMesh = _SimMesh;
}

void CSimMeshDef::GetSimLine(StTag& pTag) 
{
	StSimMesh* pSimLine = new StSimMesh;
	pSimLine->numTags = u32;
	pSimLine->sNameIdx = u32;
	pSimLine->sSimVtxCount = u32;
	pSimLine->sIterationCount = u32;
	pSimLine->bIsSimLine = true;
	pTag.pSimMesh = pSimLine;

#ifdef DEBUG_CONSOLE
	printf("\n\t\t- StSimLine {sTags: %d, sName:%d, sSimVtxCount:%d, sIterationCount:%d}\n",
		pSimLine->numTags, pSimLine->sNameIdx,
		pSimLine->sSimVtxCount, pSimLine->sIterationCount);
#endif

}


void CSimMeshDef::GetLineDef(StSimMesh& sMesh) 
{
	uint32_t numLines = u32;
	sMesh.lines.sSize = numLines;
	sMesh.lines.links.resize(numLines);

	for (int i = 0; i < numLines; i++) {
		uint32_t index = u32;
		uint32_t nodeBegin = u32;
		uint32_t nodeEnd = u32;

		/* Collect all linked nodes and append to line def*/
		NodeLink nodeLink = NodeLink{ nodeBegin, nodeEnd };
		for (int j = nodeBegin; j <= nodeEnd; j++) {
			SimNode& node = sMesh.nodePalette.at(j);
			nodeLink.link.push_back(node);
		}

		sMesh.lines.links.at(index) = nodeLink;
	}
}

bool CSimMeshDef::useOldAssignNodeSchema(const uintptr_t& address, char*& fs, int numNodes) 
{
	if (numNodes == 0) return false;

	uint32_t matX = ReadUInt32(fs);
	uint32_t matY = ReadUInt32(fs);
	uint32_t matZ = ReadUInt32(fs);
	uint32_t matW = ReadUInt32(fs);
	uint32_t nodeIdx = ReadUInt32(fs);
	fs = (char*)address;

	uint32_t val = (matW & 0xFFFF);
	return (val != 0) && (nodeIdx == 0);
}

void CSimMeshDef::AssignNode(StSimMesh& sLine)
{
	uint32_t numNodes = u32;
	uintptr_t streamPos = (uintptr_t)m_data;
	bool useOldSchema = useOldAssignNodeSchema(streamPos, m_data, numNodes);

	for (int i = 0; i < numNodes; i++)
	{
		Vector4 matrixf;
		uint32_t index;

		if (useOldSchema) {
			index = u32;
			matrixf = Vector4{ f32,f32,f32, f32 };
		}
		else {
			matrixf = Vector4{ f32,f32,f32 };
			index = u32;
		}

		SimNode sNode = m_pSimObj->getNode(index);
		sNode.vecf = matrixf;
		sLine.nodePalette.push_back(sNode);
	}
}

void CSimMeshDef::GetNodeTable()
{
	uint32_t numChildren = u32;
	uint32_t numNodes = u32;
	m_pSimObj->clear_nodes();
	m_pSimObj->pos += 0x20;

	printf("Total Nodes: %d\n", numNodes);
	m_data = (char*)m_pSimObj->pos;

	for (uint32_t i = 0; i < numNodes; i++) {
		SimNode node{ i , m_pSimObj->loadString() };
		m_pSimObj->addNode(node);
	}
}

