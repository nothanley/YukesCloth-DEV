#include "clothjson.h"
#include "YukesSimMesh"
#include <YukesCloth>
#include <QDebug>

ClothJson::ClothJson(StTag* pTag, const char *filePath)
{
    m_pTag = pTag;
    m_JsonPath = filePath;
}


void
ClothJson::save(){
    this->m_Json.clear();

    switch (m_pTag->eType)
    {
        default:
            SaveJsonTag_Constraint();
            break;
    }
}

SimConstraint*
ClothJson::GetSimConstraint(StSimMesh* pSimMesh, const std::string& target){
    for (int i = 0; i < pSimMesh->constraints.size(); i++)
    {
        SimConstraint* meshConstraint = &pSimMesh->constraints.at(i);
        if (meshConstraint->name == target)
            return meshConstraint;
    }

    return nullptr;
}

uint32_t
ClothJson::PackVertexIndex(StSimMesh* pSimMesh, const uint32_t& index){
    for (int i = 0; i < pSimMesh->simVerts.size();i++){
        uint32_t simIdx = pSimMesh->simVerts.at(i);
        if (simIdx == index)
            return i;
    }

    /* Vertex index not found therefore we append it to our existing list */
    qDebug() << "Vertex index not found: " << index;
    pSimMesh->simVerts.push_back(index);
    uint32_t newIndex = pSimMesh->simVerts.size();
    return newIndex;
}


uint32_t
ClothJson::UnpackVertexIndex(StSimMesh* pSimMesh, const uint32_t& index){
    uint32_t unpackedIdx = index;
    unpackedIdx = pSimMesh->simVerts.at(unpackedIdx);
    return unpackedIdx;
}

void
ClothJson::SaveJsonToFile(){
    std::ofstream file(m_JsonPath, std::ios::out);
    file << std::setw(4) << m_Json;
    file.close();
}

void
ClothJson::CreateConstraintJson(StSimMesh* pSimMesh, SimConstraint* pConstraint){
    std::string constraintName = "Constraint-"+pConstraint->name;
    m_Json[constraintName.c_str()]["Size"] = pConstraint->data.size();

    /* Iterate through constraint data */
    for (int i = 0; i < pConstraint->data.size(); i++){
        auto link = pConstraint->data.at(i);

        std::vector<uint32_t> indices{ UnpackVertexIndex(pSimMesh,link.vertices.x.index),
                                       UnpackVertexIndex(pSimMesh,link.vertices.y.index) };

        std::vector<float> weights{ link.vertices.x.weight,
                                    link.vertices.y.weight,
                                    link.vertices.z.weight};

        m_Json[constraintName.c_str()]["Links"]["Indices"][std::to_string(i).c_str()] = indices;
        m_Json[constraintName.c_str()]["Links"]["Weights"][std::to_string(i).c_str()] = weights;
    }
}


void
ClothJson::CreateJsonVertexTables(StSimMesh* pSimMesh){

#ifdef DEBUG_EDITOR

    std::vector<uint32_t> index_table;
    for (int i = 0; i < pSimMesh->skinPaste.size(); i++){
        auto index = pSimMesh->skinPaste[i];
        index = UnpackVertexIndex(pSimMesh,index);
        index_table.push_back(index);
    }
    m_Json["Mesh"]["SkinPaste"] = index_table;

    /* Append vtx save table */
    std::vector<uint32_t> vtx_table;
    for (int i = 0; i < pSimMesh->saveVerts.size(); i++){
        auto index = pSimMesh->saveVerts[i];
        index = UnpackVertexIndex(pSimMesh,index);
        vtx_table.push_back(index);
    }

    m_Json["Mesh"]["SaveVert"] = vtx_table;

    /* Append vtx skin calc table */
    std::vector<uint32_t> skinc_table;
    for (int i = 0; i < pSimMesh->skin_calc_indices.size(); i++){
        auto index = pSimMesh->skin_calc_indices[i];
        index = UnpackVertexIndex(pSimMesh,index);
        skinc_table.push_back(index);
    }
    m_Json["Mesh"]["SkinCalc"] = skinc_table;

    /* Append vtx force table */
    std::vector<uint32_t> force_table;
    for (int i = 0; i < pSimMesh->force.data.size(); i++){
        auto vertex = pSimMesh->force.data[i];
        uint32_t index = UnpackVertexIndex(pSimMesh,vertex.index);
        force_table.push_back(index);
    }
    m_Json["Mesh"]["Force"] = force_table;

    /* Append vtx save table */
    std::vector<uint32_t> src_table;
    for (int i = 0; i < pSimMesh->sourceEdges.size(); i++){
        auto edge = pSimMesh->sourceEdges[i];
        auto pointA = UnpackVertexIndex(pSimMesh,edge.point0);
        auto pointB = UnpackVertexIndex(pSimMesh,edge.point1);
        auto pointC = UnpackVertexIndex(pSimMesh,edge.point2);
        src_table.push_back(pointA);
        src_table.push_back(pointB);
        src_table.push_back(pointC);
    }
    m_Json["Mesh"]["SrcLink"] = src_table;

    /* Append vtx save table */
    std::vector<uint32_t> col_vtx_table;
    CollisionVerts& cols = pSimMesh->vtxColGroups.front();
    for (int i = 0; i < cols.items.size(); i++){
        auto edge   = cols.items[i];
        auto pointA = UnpackVertexIndex(pSimMesh,edge.x.index);
        auto pointB = UnpackVertexIndex(pSimMesh,edge.y.index);
        col_vtx_table.push_back(pointA);
        col_vtx_table.push_back(pointB);
    }
    m_Json["Mesh"]["ColVerts"] = col_vtx_table;

#endif
}

bool
isStringInVector(const std::string& str, const std::vector<std::string>& vec) {
    auto it = std::find(vec.begin(), vec.end(), str);
    return it != vec.end();
}

bool
isValidConstraint(const std::string& target){
    std::vector<std::string> whiteList{"Standard","Bend","Fixation","Stretch"};
    return isStringInVector(target,whiteList);
}

void
ClothJson::SaveJsonTag_Constraint(){
    if (!m_pTag->pSimMesh){
        printf("No sim mesh found!\n");
        return; }

    /* Aquire target constraint */
    StSimMesh* pSimMesh = m_pTag->pSimMesh;

    /* Init json stream */
    m_Json["Model"]["SubObj Name"] = pSimMesh->sObjName;
    m_Json["Model"]["Model Name"] = pSimMesh->sModelName;
    m_Json["Model"]["Sim Verts"] = pSimMesh->sObjVerts.size();

    /* Save constraint types */
    std::vector<std::string> constraintList;
    for (auto& constraint : pSimMesh->constraints)
        if (isValidConstraint(constraint.name))
            constraintList.push_back(constraint.name);

    m_Json["Model"]["Constraints"] = constraintList;

    /* Add all constraint data + save to file */
    for (auto& constraint : pSimMesh->constraints)
    {
        if (isValidConstraint(constraint.name))
            CreateConstraintJson(pSimMesh, &constraint);
    }

    /* Add Vertex tables to json */
    CreateJsonVertexTables(pSimMesh);

    /* Save File */
    SaveJsonToFile();
}



void
ReadJson(JSON& json, const char* filePath){
    std::ifstream i(filePath);
    i >> json;
    i.close();
}

SimConstraint
ClothJson::GetJson_Constraint(StSimMesh* pSimMesh, const SimConstraint& source){
    SimConstraint constraint{source.name,source.type};
    constraint.parameter = source.parameter;

    std::string constraintName = "Constraint-"+constraint.name;
    uint32_t sSize = m_Json[constraintName.c_str()]["Size"];
    constraint.data.resize(sSize);

    /* Iterate through constraint data */
    for (int i = 0; i < sSize; i++){
        EdgeConstraint edge;
        std::vector<uint32_t> indices = m_Json[constraintName.c_str()]
                                        ["Links"]["Indices"][std::to_string(i).c_str()];

        std::vector<float> weights = m_Json[constraintName.c_str()]
                                        ["Links"]["Weights"][std::to_string(i).c_str()];

        /* Get vertex index using simvtx table */
        edge.vertices.x.index = PackVertexIndex( pSimMesh, indices[0] );
        edge.vertices.y.index = PackVertexIndex( pSimMesh, indices[1] );

        /* Reassign all float values */
        edge.vertices.x.weight = weights[0];
        edge.vertices.y.weight = weights[1];
        edge.vertices.z.weight = weights[2];

        /* Ensure order for fixation buffer */
        if (constraint.name == "Fixation")
        {
            uint32_t vertIdx = edge.vertices.x.index;
            constraint.data.at(vertIdx) = edge;
        }
        else{
            constraint.data.at(i) = edge;
        }

    }

    return constraint;
}



void
ClothJson::UpdateTag(){

    /* Aquire target constraint */
    StSimMesh* pSimMesh = m_pTag->pSimMesh;
    ReadJson(m_Json, m_JsonPath.c_str());
    std::vector<std::string> contents = m_Json["Model"]["Constraints"];

    /* Update mesh data with json */
    for (int i = 0; i < pSimMesh->constraints.size(); i++){
        SimConstraint meshData = pSimMesh->constraints[i];
        bool hasJsonConstraint = isStringInVector(meshData.name,contents);

        /* add data if json contains dataset */
        if (hasJsonConstraint && isValidConstraint(meshData.name)){
            SimConstraint data = GetJson_Constraint(pSimMesh, meshData);
            pSimMesh->constraints[i] = data;
//            qDebug() << "Swapped buffer: " << meshData.name.c_str();
        }
    }


}
















