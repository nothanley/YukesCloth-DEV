#include <string>
#include <Json.hpp>
#pragma once

using JSON = nlohmann::json;
class StTag;
class StSimMesh;
class SimConstraint;

class ClothJson
{

public:
        ClothJson(StTag* pTag, const char* filePath);

public:
        void save();
        void UpdateTag();
        SimConstraint GetJson_Constraint(StSimMesh *pSimMesh, const SimConstraint &source);

private:
        void SaveJsonToFile();
        void SaveJsonTag_Constraint();
        void CreateJsonVertexTables(StSimMesh* pSimMesh);
        void CreateConstraintJson(StSimMesh* pSimMesh, SimConstraint* pConstraint);
        SimConstraint* GetSimConstraint(StSimMesh* pSimMesh, const std::string& target);
        uint32_t UnpackVertexIndex(StSimMesh* pSimMesh, const uint32_t& index);
        uint32_t PackVertexIndex(StSimMesh* pSimMesh, const uint32_t& index);

private:
        StTag* m_pTag = nullptr;
        std::string m_JsonPath;
        JSON m_Json;
};

