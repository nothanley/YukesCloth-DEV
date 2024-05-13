#include "csimmesh.h"
#include "cgamecloth.h"
#include "simmesh_rcn.h"
#include "SimMeshJson.h"
using JSON = nlohmann::json;

CJsonSimMesh::CJsonSimMesh(const char* filePath) {
	this->m_sJsonPath = filePath;
    ReadJson(m_Json, m_sJsonPath.c_str());
    LoadSimMesh();
}

bool hasSimNode(const SimNode& node, const std::vector<SimNode>& palette) {
    for (auto& element : palette) {
        if (element.name == node.name)
            return true;
    }
    return false;
}

void
CJsonSimMesh::ReadJson(JSON& json, const char* filePath) {
    std::ifstream i(filePath);
    i >> json;
    i.close();

    this->ValidateJsonType();
}

void
CJsonSimMesh::ValidateJsonType() {

    try {
        float jsonFormat = m_Json["General"]["JSIM Version"];
    }
    catch (...) {
        throw("Invalid json sim container.");
    }

}

StTag* 
CJsonSimMesh::BuildNewTag(const uint32_t& type, const char* name = "") {
    StTag* pTag = new StTag;
    pTag->eType = type;
    pTag->sTagName = name;
    pTag->sTagCount = 0;
    return pTag;
}

void
CJsonSimMesh::Load_AssignSubObj(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_AssignSubObj);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;
    bool isSourceSimMesh = (meshType != SIM_MESH_SOURCE);

    sMesh->sMeshIndex = -1;
    sMesh->sObjIndex = -1;
    m_pSMesh->sIterationCount = m_Json["Model"]["Iteration Count"];
    sMesh->bCalcNormal = isSourceSimMesh;
    sMesh->bDispObject = isSourceSimMesh;
    Load_AssignSubObjVtx(pTag);

    pTagParent->children.push_back(pTag);
}

void
CJsonSimMesh::Load_AssignSubObjVtx(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_AssignSubObjVtx);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;

    std::vector<uint32_t> sJsonVerts = m_Json["Mesh"]["SubObj Verts"];
    sMesh->sObjVerts = sJsonVerts;

    pTagParent->children.push_back(pTag);
}

void
CJsonSimMesh::Load_AssignSimVtx(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_AssignSimVtx);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;

    std::vector<uint32_t> sJsonVerts = m_Json["Mesh"]["Sim Verts"];
    sMesh->simVerts = sJsonVerts;

    pTagParent->children.push_back(pTag);
}

void
CJsonSimMesh::Load_RCN(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_RCN);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;

    auto jsonRcn = m_Json["Mesh"]["RCN"];
    sMesh->rcn.numFaceNrmVtxIndices = jsonRcn["Faces"]["Size"];
    sMesh->rcn.numVtxNrmFaceLinks = sMesh->sSimVtxCount;
    sMesh->rcn.parameters = std::vector<uint32_t>{ 
                                sMesh->rcn.numFaceNrmVtxIndices,
                                sMesh->rcn.numVtxNrmFaceLinks, 1, 0 };

    Load_RCNSubObj(pTag);
    pTagParent->children.push_back(pTag);
}


void
CJsonSimMesh::Load_RCNSubObj(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_RCNSubObj);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;

    auto jsonRcn = m_Json["Mesh"]["RCN"];
    sMesh->rcn.numFaceNrmVtxIndices = jsonRcn["Faces"]["Size"];
    sMesh->rcn.numVtxNrmFaceLinks = sMesh->sSimVtxCount;

    /* Create face index buffer */
    for (int i = 0; i < sMesh->rcn.numFaceNrmVtxIndices; i++)
    {
        MeshPolygon face;
        std::vector<uint32_t> faceIndices = jsonRcn["Faces"][std::to_string(i)];
        uint32_t numCorners = faceIndices.size();
        sMesh->rcn.degCounts.push_back(numCorners);

        for (auto& index : faceIndices) 
        {
            sMesh->rcn.faceNrmVtxIndices.push_back(index);
            face.points.push_back(index);
        }

        sMesh->rcn.faces.push_back(face);
    }

    /* Create face link table */
    for (int i = 0; i < sMesh->rcn.numVtxNrmFaceLinks; i++)
    {
        std::vector<uint32_t> links = jsonRcn["Links"][std::to_string(i)];
        uint32_t numFaces = links.size();
        sMesh->rcn.linkCounts.push_back(numFaces);
        float perVertexFaceWeight = 1.0f / (float)numFaces;

        for (auto& faceIndex : links) {
            SkinVertex vertexFaceNrm{ faceIndex, perVertexFaceWeight };
            sMesh->rcn.vtxNrmFaceLinks.push_back(vertexFaceNrm);
        }
    }

    pTagParent->children.push_back(pTag);
}


void
CJsonSimMesh::Load_SkinVerts(StSimMesh* sMesh) {
    for (int i = 0; i < sMesh->sSimVtxCount; i++) {
        auto vMat = m_Json["Mesh"]["Vertex"][std::to_string(i)];
        auto vNrm = m_Json["Mesh"]["Vertex Normal"][std::to_string(i)];

        Vector4 vertex{ vMat[0], vMat[1], vMat[2], 1.0 };
        Vector4 normal{ vNrm[0], vNrm[1], vNrm[2], 0.0 };

        sMesh->skin.vertices.push_back(vertex);
        sMesh->skin.normals.push_back(normal);
    }
}

void
CJsonSimMesh::Load_SkinWeights(StSimMesh* sMesh, uint32_t& maxBlendWeights) {
    for (int i = 0; i < m_pSMesh->sSimVtxCount; i++)
    {
        auto vertexSkin = m_Json["Mesh"]["Skin"][std::to_string(i)];
        std::vector<std::string> palette = vertexSkin["Palette"];

        MeshWeight blendWeight;
        blendWeight.numWeights = palette.size();

        for (auto& item : palette)
        {
            SimNode node{ static_cast<uint32_t>(-1) , item };
            if (!hasSimNode(node, sMesh->skin.nodePalette))
                sMesh->skin.nodePalette.push_back(node);

            blendWeight.bones.push_back(item);
            blendWeight.weights.push_back(vertexSkin[item]);
        }

        uint32_t numWeights = palette.size();
        maxBlendWeights = (numWeights > maxBlendWeights) ? numWeights : maxBlendWeights;
        sMesh->skin.blendWeights.push_back(blendWeight);
    }
}
void
CJsonSimMesh::Load_Skin(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_Skin);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    uint32_t maxBlendWeights = 0;
    pTag->pSimMesh = sMesh;

    Load_SkinVerts(sMesh);
    Load_SkinWeights(sMesh, maxBlendWeights);
    pTagParent->children.push_back(pTag);
}

void
CJsonSimMesh::Load_SimLinkSrc(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_SimLinkSrc);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;
    auto sJsonLinks = m_Json["Mesh"]["SrcLink"];

    /* Defines all source triangles used by target meshes for deformation data */
    for (auto& link : sJsonLinks) {
        Triangle mTriangle{ link[0], link[1], link[2] };
        sMesh->sourceEdges.push_back(mTriangle);
    }

    pTagParent->children.push_back(pTag);
}

void
CJsonSimMesh::Load_SimLinkTar(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_SimLinkTar);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;
    LinkTarget meshTarget;

    auto sJsonTarget = m_Json["Mesh"]["SimLink-Target"];
    meshTarget.source = sJsonTarget["Source"];
    uint32_t numFaces = sJsonTarget["Source Faces"];
    meshTarget.totalWeights = 4;

    /* Create face index buffer */
    for (uint32_t i = 0; i < numFaces; i++) {
        meshTarget.indices.push_back(i);
    }

    /* Create vtx index palette */
    for (uint32_t i = 0; i < sMesh->sSimVtxCount; i++) {
        meshTarget.palette.push_back(i);
    }

    /* Define source face vertex influence */
    for (uint32_t i = 0; i < sMesh->sSimVtxCount; i++) {
        MeshWeight blendWeight;
        auto vtxSourceBlend = sJsonTarget[std::to_string(i)];

        for (int j = 0; j < meshTarget.totalWeights; j++) {
            blendWeight.numWeights = 4;
            int faceIndex = vtxSourceBlend[j][0];
            float influence = vtxSourceBlend[j][1];

            blendWeight.indices.push_back(faceIndex);
            blendWeight.weights.push_back(influence);
        }

        meshTarget.weights.push_back(blendWeight);
    }

    sMesh->target = meshTarget;
    pTagParent->children.push_back(pTag);
}

void
CJsonSimMesh::Load_Pattern(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_Pattern);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;
    sMesh->bSimPattern = -1;

    /* Gets all sim mesh property stacks */
    Load_Stacks(pTag);

    pTagParent->children.push_back(pTag);
}

void
CJsonSimMesh::Load_SkinCalc(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_SkinCalc);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;

    std::vector<uint32_t> sJsonVerts = m_Json["Mesh"]["SkinCalc"];

    for (auto& vtxIndex : sJsonVerts) {
        Vector4 skinMatrix = sMesh->skin.vertices.at(vtxIndex);
        sMesh->skinCalc.push_back(skinMatrix);
    }

    pTagParent->children.push_back(pTag);
}

void
CJsonSimMesh::Load_SkinPaste(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_SkinPaste);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;

    std::vector<uint32_t> sJsonVerts = m_Json["Mesh"]["SkinPaste"];

    for (auto& index : sJsonVerts) {
        sMesh->skinPaste.push_back(index);
    }

    pTagParent->children.push_back(pTag);
}

void
CJsonSimMesh::Load_OldVtxSave(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_OldVtxSave);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;

    std::vector<uint32_t> sJsonVerts = m_Json["Mesh"]["SaveVerts"];

    for (auto& index : sJsonVerts) {
        sMesh->saveVerts.push_back(index);
    }

    pTagParent->children.push_back(pTag);
}

void
CJsonSimMesh::Load_Force(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_Force);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;

    float gForceY = m_Json["Model"]["Force Y"];
    sMesh->force.parameters = std::vector<float>{ gForceY, 0.985, 0.0, 0.05 };
    std::vector<uint32_t> sJsonVerts = m_Json["Mesh"]["Force"];


    for (auto& index : sJsonVerts) {
        SkinVertex gForceVertex{ index, 1.0 };

        sMesh->force.data.push_back(gForceVertex);
    }

    pTagParent->children.push_back(pTag);
}

void
CJsonSimMesh::Load_ColVerts(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_ColVtx);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;
    auto meshColVerts = m_Json["Mesh"]["ColVerts"];

    CollisionVerts colVerts;
    colVerts.tagSize   = -1;
    colVerts.unkFlag   = -1;
    colVerts.unkVal    = 1;
    colVerts.numVerts  = sMesh->sSimVtxCount;
    colVerts.unkFlagB  = -1;

    for (int i = 0; i < colVerts.numVerts; i++) {
        std::vector<int> indices = meshColVerts[std::to_string(i)];
        for (size_t j = 0; j < indices.size(); j += 2) {
            Points edge;
            edge.x.index = indices[j];
            edge.y.index = indices[j + 1];

            edge.x.weight = 0.1;
            edge.y.weight = 1.0;
            colVerts.items.push_back(edge);
        }
    }

    std::vector<uint32_t> paletteIndices = meshColVerts["Palette"];
    colVerts.indices = paletteIndices;
    colVerts.numItems = colVerts.items.size();
    pTagParent->children.push_back(pTag);

    pTag->vtxCol_index = sMesh->vtxColGroups.size();
    sMesh->vtxColGroups.push_back(colVerts);
}

void
CJsonSimMesh::Load_Stacks(StTag* pTagParent) {
    StTag* pTag = BuildNewTag(enTagType_SimMesh_Stacks);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;

    Load_SkinCalc(pTag);
    Load_SkinPaste(pTag);
    Load_OldVtxSave(pTag);

    if (meshType == SIM_MESH_SOURCE) {
        Load_Force(pTag);
    }

    Load_Constraints(pTag);

    if (meshType == SIM_MESH_TARGET) {
        Load_SimLinkTar(pTag);
    }

    pTagParent->children.push_back(pTag);
}


void
CJsonSimMesh::Load_Constraints(StTag* pTagParent) {
    std::vector<std::string> constraints = m_Json["Model"]["Constraints"];

    for (const auto& type : constraints) {
        auto it = constraintLoaders.find(type);
        if (it != constraintLoaders.end()) {
            it->second(pTagParent);
        }
    }
}


void
CJsonSimMesh::LoadSimMesh() {
    m_pTagHead = BuildNewTag(enTagType_SimMesh, "Mesh");
    m_pSMesh = new StSimMesh;

    m_pSMesh->sModelName = "Character Model";
    m_pSMesh->sObjName = m_Json["Model"]["SubObj Name"];
    m_pSMesh->sSimVtxCount = m_Json["Model"]["Sim Verts"];
    m_pTagHead->pSimMesh = m_pSMesh;
    this->meshType = m_Json["Model"]["Mesh Type"];

    /* Get all child nodes and mesh data */
    Load_AssignSubObj(m_pTagHead);
    Load_AssignSimVtx(m_pTagHead);
    Load_RCN(m_pTagHead);
    Load_Skin(m_pTagHead);

    if (meshType == SIM_MESH_SOURCE) 
    {
        Load_SimLinkSrc(m_pTagHead);
    }

    /* Get all mesh constraints */
    Load_Pattern(m_pTagHead);

    printf("\n\t{YCL Log} Json mesh conversion success!\n\n");
}



void
CJsonSimMesh::Load_CtStdLink(StTag* pTagParent) {
    if (meshType != SIM_MESH_SOURCE) return;

    StTag* pTag = BuildNewTag(enTagType_SimMesh_CtStdLink);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;

    SimConstraint constraint{ "Standard", enTagType_SimMesh_CtStdLink };
    uint32_t numLinks = m_Json["Constraint-Standard"]["Size"];
    constraint.parameter = 0; /* todo: this is probably an index */

    auto edgeLinks = m_Json["Constraint-Standard"]["Links"];
    for (int i = 0; i < numLinks; i++)
    {
        auto indices = edgeLinks["Indices"][std::to_string(i)];
        auto weights = edgeLinks["Weights"][std::to_string(i)];

        EdgeConstraint edgeLink;
        edgeLink.vertices.x.index = indices.at(0);
        edgeLink.vertices.y.index = indices.at(1);

        edgeLink.vertices.x.weight = weights.at(0);
        edgeLink.vertices.y.weight = weights.at(1);
        edgeLink.vertices.z.weight = weights.at(2);;
        constraint.data.push_back(edgeLink);
    }

    sMesh->constraints.push_back(constraint);
    pTagParent->children.push_back(pTag);
}

void
CJsonSimMesh::Load_CtBendLink(StTag* pTagParent) {
    if (meshType != SIM_MESH_SOURCE) return;

    StTag* pTag = BuildNewTag(enTagType_SimMesh_CtBendLink);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;

    SimConstraint constraint{ "Bend", enTagType_SimMesh_CtBendLink };
    uint32_t numLinks = m_Json["Constraint-Bend"]["Size"];
    constraint.parameter = 1; /* todo: this is probably an index */

    auto edgeLinks = m_Json["Constraint-Bend"]["Links"];
    for (int i = 0; i < numLinks; i++)
    {
        auto indices = edgeLinks["Indices"][std::to_string(i)];
        auto weights = edgeLinks["Weights"][std::to_string(i)];

        EdgeConstraint edgeLink;
        edgeLink.vertices.x.index = indices.at(0);
        edgeLink.vertices.y.index = indices.at(1);

        edgeLink.vertices.x.weight = weights.at(0);
        edgeLink.vertices.y.weight = weights.at(1);
        edgeLink.vertices.z.weight = weights.at(2);;
        constraint.data.push_back(edgeLink);
    }

    sMesh->constraints.push_back(constraint);
    pTagParent->children.push_back(pTag);
}

void
CJsonSimMesh::Load_CtStretchLink(StTag* pTagParent) {
    if (meshType != SIM_MESH_SOURCE) return;

    StTag* pTag = BuildNewTag(enTagType_SimMesh_CtStretchLink);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;

    SimConstraint constraint{ "Stretch", enTagType_SimMesh_CtStretchLink };
    uint32_t numLinks = m_Json["Constraint-Stretch"]["Size"];
    constraint.parameter = 1; /* todo: this is probably an index */

    auto edgeLinks = m_Json["Constraint-Stretch"]["Links"];
    for (int i = 0; i < numLinks; i++)
    {
        auto indices = edgeLinks["Indices"][std::to_string(i)];
        auto weights = edgeLinks["Weights"][std::to_string(i)];

        EdgeConstraint edgeLink;
        edgeLink.vertices.x.index = indices.at(0);
        edgeLink.vertices.y.index = indices.at(1);

        edgeLink.vertices.x.weight = weights.at(0);
        edgeLink.vertices.y.weight = weights.at(1);
        edgeLink.vertices.z.weight = weights.at(2);;
        constraint.data.push_back(edgeLink);
    }

    sMesh->constraints.push_back(constraint);
    pTagParent->children.push_back(pTag);
}

void
CJsonSimMesh::Load_CtFixation(StTag* pTagParent) {
    if (meshType == SIM_MESH_SOURCE) {
        Load_ColVerts(pTagParent);
    }

    StTag* pTag = BuildNewTag(enTagType_SimMesh_CtFixation);
    StSimMesh* sMesh = pTagParent->pSimMesh;
    pTag->pSimMesh = sMesh;

    SimConstraint constraint{ "Fixation", enTagType_SimMesh_CtFixation };
    uint32_t numLinks = m_Json["Constraint-Fixation"]["Size"];
    constraint.parameter = 0; /* todo: this is probably an index */

    auto edgeLinks = m_Json["Constraint-Fixation"]["Links"];
    for (int i = 0; i < numLinks; i++)
    {
        auto indices = edgeLinks["Indices"][std::to_string(i)];
        auto weights = edgeLinks["Weights"][std::to_string(i)];

        EdgeConstraint edgeLink;
        edgeLink.vertices.x.index = indices.at(0);
        edgeLink.vertices.x.weight = weights.at(0);
        constraint.data.push_back(edgeLink);
    }

    sMesh->constraints.push_back(constraint);
    pTagParent->children.push_back(pTag);
}
