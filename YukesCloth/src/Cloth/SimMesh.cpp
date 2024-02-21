#include "SimMesh.h"
#include "BinaryIO.h"
#include "SimObj.h"
#include<algorithm>
using namespace BinaryIO;

#define _U32  ReadUInt32(*pSimObj->m_pDataStream)
#define _S32  ReadSInt32(*pSimObj->m_pDataStream)
#define _U16  ReadUShort(*pSimObj->m_pDataStream)
#define _U8  ReadByte(*pSimObj->m_pDataStream)
#define _BOOL ReadBool(*pSimObj->m_pDataStream)
#define _FLOAT ReadFloat(*pSimObj->m_pDataStream)

void
CSimMeshData::GetSimMesh(StTag& parent, const CSimObj* pSimObj) {

	/* Initialize and get Sim Mesh stream */
	StSimMesh* _SimMesh = new StSimMesh{ _U32, _U32, _U32, _U32, (bool)_U32, (bool)_U32 };

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

void
CSimMeshData::AssignSubObj(StSimMesh& sMesh, const CSimObj* pSimObj) {
	uint32_t numTags = _U32;
	sMesh.sMeshIndex = _U32;
	sMesh.sObjIndex = _U32;

#ifdef DEBUG_CONSOLE
	printf("\n\t\t- StSimMesh_AssignSubObj {sTags: %d, sModelName:%d, sObjName:%d }\n",
		numTags, sMesh.sMeshIndex, sMesh.sObjIndex);
#endif
}

void
CSimMeshData::AssignSubObjVtx(StSimMesh& sMesh, const CSimObj* pSimObj) {
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x10);
	uint32_t numVerts = _U32;

	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x20);
	for (uint32_t i = 0; i < numVerts; i++) {
		sMesh.sObjVerts.push_back(_U32);
	}

}

void
CSimMeshData::AssignSimVtx(StSimMesh& sMesh, const CSimObj* pSimObj) {
	uint32_t unkVal0 = _U32;
	uint32_t unkVal1 = _U32;
	uint32_t numSimVerts = _U32;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x20);

	for (uint32_t i = 0; i < numSimVerts; i++) {
		uint32_t nullValue = _U32;
		sMesh.simVerts.push_back(_U32);
	}
}

void
CSimMeshData::GetSkinData(StSimMesh& sMesh, const CSimObj* pSimObj) 
{
	/* This buffer is arranged with respect to the simVtx table, 
		correct the output to match the table */
	uint32_t numBones = _U32;
	uint32_t numVerts = _U32;
	uint32_t maxBlendWeights = _U32;
	uint32_t numOther = _U32;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x20);

	/* Get vertex coordinates */
	for (int i = 0; i < numVerts; i++) {
		Vector4 vertex{ _FLOAT, _FLOAT, _FLOAT, _FLOAT };
		sMesh.skin.matrices.push_back(vertex);
	}

	/* Get vertex normals */
	for (int i = 0; i < numVerts; i++) {
		Vector4 normal{ _FLOAT, _FLOAT, _FLOAT, _FLOAT };
		sMesh.skin.attributes.push_back(normal);
	}
	uintptr_t skinBufferAddress = pSimObj->m_pDataStream->tellg();

	/* Get blendweight palette */
	std::vector<SimNode> skinPalette;
	for (int i = 0; i < numBones; i++) {
		int nodeIndex = _U32;
		SimNode node = pSimObj->m_NodeTable.at(nodeIndex);
		skinPalette.push_back(node);
	}

	/* Get blendweights */
	for (int i = 0; i < numVerts; i++) {
		MeshWeight blendWeight;
		blendWeight.numWeights = maxBlendWeights;

		for (int i = 0; i < maxBlendWeights; i++) {
			if (i >= blendWeight.numWeights) 
			{
				_U16; _U16; _FLOAT;
			}
			else {
				blendWeight.numWeights = _U16;
				blendWeight.bones.push_back(skinPalette.at(_U16).name);
				blendWeight.weights.push_back(_FLOAT);
			}
		}
		sMesh.skin.blendWeights.push_back(blendWeight);
	}

	sMesh.skin.nodePalette = skinPalette;
}

void
CSimMeshData::Link_DefineSourceMesh(StSimMesh& sMesh, const CSimObj* pSimObj) {
	uint32_t numTriangles = _U32;

	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x20);

	/* Define source edges */
	for (int i = 0; i < numTriangles; i++) {
		sMesh.sourceEdges.push_back(Triangle{ _U16, _U16, _U32 });
	}

}

void
CSimMeshData::GetSimMeshPattern(StSimMesh& sMesh, const CSimObj* pSimObj) {
    if (!&sMesh) {
		printf("Could not parse skin pattern - Missing sim mesh destination.\n");
		return;
	}

	uint32_t numNodes = _U32;
	sMesh.bSimPattern = _S32;
}

void
CSimMeshData::GetSimMeshStacks(StSimMesh& sMesh, const CSimObj* pSimObj) {
    uint32_t numProperties = _U32;
}

void
CSimMeshData::GetSkinCalc(StSimMesh& sMesh, const CSimObj* pSimObj) {
	uint32_t numSkinVerts = _U32;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x10);
	
	for (int i = 0; i < numSkinVerts; i++)
	{
		uint32_t vertIdx = _U32;
        Vector4 skinMatrix = sMesh.skin.matrices.at(vertIdx);
        sMesh.skinCalc.push_back(skinMatrix);
	}
}

void
CSimMeshData::GetSkinPaste(StSimMesh& sMesh, const CSimObj* pSimObj) {
	uint32_t numPasteVerts = _U32;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x10);

	for (int i = 0; i < numPasteVerts; i++)
	{
		uint32_t skinIdx = _U32;
		sMesh.skinPaste.push_back(skinIdx);
	}
}

void
CSimMeshData::SaveOldVtxs(StSimMesh& sMesh, const CSimObj* pSimObj) {
	uint32_t numVerts = _U32;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x10);

	for (int i = 0; i < numVerts; i++)
	{
		sMesh.saveVerts.push_back(_U32);
	}
}

void
CSimMeshData::GetForce(StSimMesh& sMesh, const CSimObj* pSimObj) {
	uint32_t numVerts = _U32;
    sMesh.force.parameters = std::vector<float>{ _FLOAT, _FLOAT, _FLOAT, _FLOAT };
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x30);

	for (int i = 0; i < numVerts; i++)
	{
		sMesh.force.data.push_back(SkinVertex{_U32,_FLOAT});
	}
}

float GetVectorDistance(Vector4& vec_a, Vector4& vec_b) {
	float a = (vec_a.x - vec_b.x);
	float b = (vec_a.y - vec_b.y);
	float c = (vec_a.z - vec_b.z);

	return sqrtf((a * a) + (b * b) + (c * c));
}

void
CSimMeshData::GetConstraintStretchLink(StSimMesh& sMesh, const CSimObj* pSimObj) {
	uint32_t numLinks = _U32;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x10);
	SimConstraint stretchConstraint{ "Stretch", enTagType_SimMesh_CtStretchLink };

	for (int i = 0; i < numLinks; i++)
	{
		EdgeConstraint edgeLink;
		edgeLink.vertices.x.index = _U32;
		edgeLink.vertices.y.index = _U32;

		edgeLink.vertices.x.weight = _FLOAT;
		edgeLink.vertices.y.weight = _FLOAT;;

		stretchConstraint.data.push_back(edgeLink);
	}

	sMesh.constraints.push_back(stretchConstraint);
}


void
CSimMeshData::GetConstraintStandardLink(StSimMesh& sMesh, const CSimObj* pSimObj) {

	SimConstraint constraint{ "Standard", enTagType_SimMesh_CtStdLink };
	uint32_t numLinks = _U32;
	constraint.parameter = _U32;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x20);

	for (int i = 0; i < numLinks; i++)
	{
		EdgeConstraint edgeLink;
		edgeLink.vertices.x.index = _U16;
		edgeLink.vertices.y.index = _U16;

		edgeLink.vertices.x.weight = _FLOAT;
		edgeLink.vertices.y.weight = _FLOAT;
		edgeLink.vertices.z.weight = _FLOAT;;

		constraint.data.push_back(edgeLink);
	}

	sMesh.constraints.push_back(constraint);
}

void
CSimMeshData::GetConstraintBendLink(StSimMesh& sMesh, const CSimObj* pSimObj) {
	SimConstraint constraint{ "Bend", enTagType_SimMesh_CtBendLink };
	uint32_t numLinks = _U32;
	constraint.parameter = _U32;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x20);

	for (int i = 0; i < numLinks; i++)
	{
		EdgeConstraint edgeLink;
		edgeLink.vertices.x.index = _U16;
		edgeLink.vertices.y.index = _U16;

		edgeLink.vertices.x.weight = _FLOAT;
		edgeLink.vertices.y.weight = _FLOAT;;
		edgeLink.vertices.z.weight = _FLOAT;;
		constraint.data.push_back(edgeLink);
	}

	sMesh.constraints.push_back(constraint);
}

void
CSimMeshData::GetBendStiffness(StSimMesh& sMesh, const CSimObj* pSimObj) {
	SimConstraint constraint{ "BendStiffness", enTagType_SimMesh_BendingStiffness };
	uint32_t numTris = _U32;
	constraint.parameter = _U32;
	float unkFloat = _FLOAT;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x20);

	for (int i = 0; i < numTris; i++){
		FaceConstraint face;

		face.x.index = _U16;
		face.y.index = _U16;
		face.z.index = _U32;

		face.weights.push_back(_FLOAT);
		face.weights.push_back(_FLOAT);
		face.weights.push_back(_FLOAT);
		face.weights.push_back(_FLOAT);

		constraint.faceData.push_back(face);
	}

	sMesh.constraints.push_back(constraint);
}


void
CSimMeshData::GetCollisionVerts(StTag& tag, StSimMesh& sMesh, const CSimObj* pSimObj) {
	sMesh.colVtx.tagSize = tag.sSize;
	sMesh.colVtx.unkFlag = _S32;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x30);

	sMesh.colVtx.unkVal = _S32;
	sMesh.colVtx.numItems = _S32;
	sMesh.colVtx.numVerts = _S32;
	sMesh.colVtx.unkFlagB = _S32;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + tag.sSize);

	for (int i = 0; i < sMesh.colVtx.numItems; i++)
	{
		Points segment;
		segment.x.index = _U32;
		segment.y.index = _U32;

		segment.x.weight = _FLOAT;
		segment.y.weight = _FLOAT;

		sMesh.colVtx.items.push_back(segment);
	}

	for (int i = 0; i < sMesh.colVtx.numVerts; i++)
	{
		sMesh.colVtx.indices.push_back(_U32);
	}

}

void
CSimMeshData::GetConstraintFixation(StSimMesh& sMesh, const CSimObj* pSimObj) {
	uint32_t numVerts = _U32;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x10);
	SimConstraint constraint{ "Fixation", enTagType_SimMesh_CtFixation };

	for (int i = 0; i < numVerts; i++)
	{
		EdgeConstraint edge;
		edge.vertices.x.index = _U32;
		edge.vertices.x.weight = _FLOAT;
		constraint.data.push_back(edge);
	}

	sMesh.constraints.push_back(constraint);
}

void
CSimMeshData::GetSimLine(StTag& pTag, const CSimObj* pSimObj) {
    StSimMesh* pSimLine = new StSimMesh;
    pSimLine->numTags = _U32;
    pSimLine->sNameIdx = _U32;
    pSimLine->sSimVtxCount = _U32;
    pSimLine->sIterationCount = _U32;
    pSimLine->bIsSimLine = true;
    pTag.pSimMesh = pSimLine;

#ifdef DEBUG_CONSOLE
    printf("\n\t\t- StSimLine {sTags: %d, sName:%d, sSimVtxCount:%d, sIterationCount:%d}\n",
           pSimLine->numTags, pSimLine->sNameIdx,
           pSimLine->sSimVtxCount, pSimLine->sIterationCount);
#endif
}

void
CSimMeshData::GetStringTable(CSimObj* pSimObj){
    pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos+0x8);
    uint32_t numNodes = _U32;
    uint32_t numStrings = _U32;

    printf("Total Strings: %d\n", numStrings);
	pSimObj->m_iStreamPos += 0x20;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos);

    for (int i = 0; i < numStrings; i++){
		std::string item = GetString(pSimObj);
        pSimObj->m_sStringTable.push_back(item);
    }
}

std::string
CSimMeshData::GetString(CSimObj* pSimObj) {
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos);
	uint32_t eTagType = _U32;
	uint32_t sSize = _U32;
	uint32_t sTotalSize = _U32;
	uint32_t numChildNodes = _U32;

	int charBufferSize = sTotalSize - 0x10;
	std::string item = ReadString(*pSimObj->m_pDataStream, charBufferSize);

	pSimObj->m_iStreamPos += sTotalSize;
	return item;
}


void
CSimMeshData::GetLineDef(StSimMesh& sMesh, CSimObj* pSimObj) {
	uint32_t numLines = _U32;
	sMesh.lines.sSize = numLines;
	sMesh.lines.links.resize(numLines);

	for (int i = 0; i < numLines; i++) {
		uint32_t index = _U32;
		uint32_t nodeBegin  = _U32;
		uint32_t nodeEnd = _U32;
		
		/* Collect all linked nodes and append to line def*/
		NodeLink nodeLink = NodeLink{ nodeBegin, nodeEnd };
		for (int j = nodeBegin; j <= nodeEnd; j++) {
			SimNode& node  = sMesh.nodePalette.at(j);
			nodeLink.link.push_back(node);
		}

		sMesh.lines.links.at(index) = nodeLink;
	}
}

bool
useOldAssignNodeSchema(const uintptr_t& address, std::ifstream* fs, int numNodes) {
	if (numNodes == 0) return false;

	uint32_t matX = ReadUInt32(*fs);
	uint32_t matY = ReadUInt32(*fs);
	uint32_t matZ = ReadUInt32(*fs);
	uint32_t matW = ReadUInt32(*fs);
	uint32_t nodeIdx = ReadUInt32(*fs);
	fs->seekg(address);

	uint32_t val = (matW & 0xFFFF);
	return (val != 0) && (nodeIdx == 0);
}

void
CSimMeshData::AssignNode(StSimMesh& sLine, CSimObj* pSimObj)
{
	uint32_t numNodes = _U32;
	uintptr_t streamPos = pSimObj->m_pDataStream->tellg();
	bool useOldSchema = useOldAssignNodeSchema(streamPos, pSimObj->m_pDataStream, numNodes);

	for (int i = 0; i < numNodes; i++)
	{
		Vector4 matrixf;
		uint32_t index;

		if (useOldSchema) {
			index = _U32;
			matrixf = Vector4{ _FLOAT,_FLOAT,_FLOAT, _FLOAT };
		}
		else {
			matrixf = Vector4{ _FLOAT,_FLOAT,_FLOAT };
			index = _U32;
		}

		SimNode sNode = pSimObj->m_NodeTable.at(index);
		sNode.vecf = matrixf;
		sLine.nodePalette.push_back(sNode);
	}
}


void 
CSimMeshData::GetNodeTable(CSimObj* pSimObj) {
	uint32_t numChildren = _U32;
	uint32_t numNodes = _U32;
	pSimObj->m_NodeTable.resize(numNodes);

	printf("Total Nodes: %d\n", numNodes);
	pSimObj->m_iStreamPos += 0x20;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos);

    for (uint32_t i = 0; i < numNodes; i++) {
        SimNode node{ i , GetString(pSimObj) };
		pSimObj->m_NodeTable.at(i) = node;
	}
}


StSimMesh* GetMeshSourceObj(const std::string& meshName, CSimObj* pSimObj) {
	StTag* pTag = pSimObj->m_pStHead;

	for (auto& child : pTag->children) {
		StSimMesh* pMesh = child->pSimMesh;
		if (pMesh) 
		{
			int stringIndex = pMesh->sNameIdx;
			std::string childObjName = pSimObj->m_sStringTable.at(stringIndex);
			if (childObjName == meshName) {
				return pMesh;
			}
		}
	}
	return nullptr;
}


void
CSimMeshData::GetLinkTar(StSimMesh& sLine, CSimObj* pSimObj) 
{
	LinkTarget meshTarget;
	meshTarget.source = ReadString(*pSimObj->m_pDataStream, 0x80);
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0x8C);

	StSimMesh* pSourceMesh = GetMeshSourceObj(meshTarget.source, pSimObj);
	if (!pSourceMesh) {
		throw("\nCould not locate source mesh for target: %s", sLine.sObjName);
	}

	uint32_t numIdxs = _U32;
	uint32_t paletteSize = _U32;
	meshTarget.totalWeights = _U32;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos + 0xA0);

	/* Index and create a table of every used source edge from the sim mesh */
	for (uint32_t i = 0; i < numIdxs; i++) {
		int index = _U32;
		Triangle srcTriangle = pSourceMesh->sourceEdges.at(index);

		meshTarget.sourceTris.push_back(srcTriangle);
		meshTarget.indices.push_back(index);
	}

	/* Index and create a table of all vertex coords for the target mesh */
	for (uint32_t i = 0; i < paletteSize; i++) {
		int index = _U32;
		Vector4 vertexMat = sLine.skin.matrices.at(index);

		meshTarget.vertexTable.push_back(vertexMat);
		meshTarget.palette.push_back(index);
	}

	/* Iterate and define the influence of the source triangle on the specified vertex */
	for (uint32_t i = 0; i < paletteSize; i++) {
		MeshWeight blendWeight;
		for (int j = 0; j < meshTarget.totalWeights; j++) {
			blendWeight.numWeights = _U16;
			int srcIndex = _U16;
			float influence = _FLOAT;

			blendWeight.indices.push_back(srcIndex);
			blendWeight.weights.push_back(influence);

		}

		meshTarget.weights.push_back(blendWeight);
	}

	sLine.target = meshTarget;
}

void
CSimMeshData::GetColIDTbl(CSimObj* pSimObj)
{
	uint32_t numChildren = _U32;
	uint32_t numColliders = _U32;
	pSimObj->m_iStreamPos = uintptr_t(pSimObj->m_pDataStream->tellg()) + 0xC;
	pSimObj->m_pDataStream->seekg(pSimObj->m_iStreamPos);
	
	for (int i = 0; i < numColliders; i++) {
		std::string name = GetString(pSimObj);
		pSimObj->collisionID = name;
	}
}

void
CSimMeshData::GetCols(CSimObj* pSimObj) {
	uint32_t numChildren = _U32;
	int32_t unkVal = _S32;
}


void
CSimMeshData::GetColPack(CSimObj* pSimObj) {
	uint32_t numCapsules = _U32;
	uint32_t pad = _U32;
	uint32_t packType = _U32;
}

void
CSimMeshData::GetCapsuleStandard(CSimObj* pSimObj, StTag& tag)
{
	CollisionVolume capsule;
	capsule.indexA = _U32;
	capsule.indexB = _U32;

	capsule.weights = std::vector<float>
	{  _FLOAT, _FLOAT, _FLOAT, _FLOAT,
	   _FLOAT, _FLOAT, _FLOAT, _FLOAT };

	capsule.id = _U32;
	if (capsule.indexB >= pSimObj->m_NodeTable.size()) {
		printf("Invalid capsule join. Could not get node table index.\n");
		capsule.indexB = pSimObj->m_NodeTable.size()-1;
	}

	capsule.name = pSimObj->m_sStringTable.at(capsule.indexA);
	capsule.joint = pSimObj->m_NodeTable.at(capsule.indexB).name;

	pSimObj->m_ColTable.push_back(capsule);
	tag.col = capsule;
}

void
CSimMeshData::GetCapsuleTapered(CSimObj* pSimObj, StTag& tag) {
	CollisionVolume capsule;
	capsule.indexA = _U32;
	capsule.indexB = _U32;

	capsule.weights = std::vector<float>
	{ _FLOAT, _FLOAT, _FLOAT, _FLOAT,
	   _FLOAT, _FLOAT, _FLOAT, _FLOAT };

	capsule.radius = _FLOAT;
	capsule.id = _U32;
	if (capsule.indexB >= pSimObj->m_NodeTable.size()) {
		printf("Invalid capsule join. Could not get node table index.\n");
		capsule.indexB = pSimObj->m_NodeTable.size() - 1;
	}

	capsule.name = pSimObj->m_sStringTable.at(capsule.indexA);
	capsule.joint = pSimObj->m_NodeTable.at(capsule.indexB).name;

	pSimObj->m_ColTable.push_back(capsule);
	tag.col = capsule;
}
