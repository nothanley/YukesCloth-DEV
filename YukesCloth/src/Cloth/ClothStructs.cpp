#include "ClothStructs.h"
#include <string>
#include <fstream>
#include <BinaryIO.h>
#include <iostream>

using namespace BinaryIO;


void
yclutils::debugPrintFloats(std::vector<Vector4>* floatArray) {
	std::string filePath = "";
	std::ofstream outputFile(filePath.c_str(), ios_base::binary);

	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open the file for writing." << std::endl;
		return;
	}

	for (const auto& vec : *floatArray) {
		WriteFloat(&outputFile, vec.x);
		WriteFloat(&outputFile, vec.y);
		WriteFloat(&outputFile, vec.z);
		WriteFloat(&outputFile, vec.w);
	}

	outputFile.close();
}

bool 
yclutils::hasIndex(const std::vector<int>& vec, int target){
	auto it = std::find(vec.begin(), vec.end(), target);
	return it != vec.end();
}

std::string
yclutils::GetNodeName(const uint32_t& tagEnum){
    if (tagEnum > 57)
        return "";

    std::vector<std::string> enumStrings = {
        "Root",
        "Cols",
        "ColPack",
        "Capsule_Standard",
        "Capsule_Tapered",
        "SimMesh",
        "SimMesh_AssignSubObj",
        "SimMesh_AssignSubObjVtx",
        "SimMesh_AssignSimVtx",
        "SimMesh_Stacks",
        "SimMesh_SkinCalc",
        "SimMesh_SkinPaste",
        "SimMesh_Force",
        "SimMesh_CtStdLink",
        "SimMesh_CtBendLink",
        "SimMesh_CtLocalRange",
        "SimMesh_CtFixation",
        "SimMesh_SimLinkTar",
        "SimMesh_ColVtx",
        "SimMesh_RCN",
        "SimMesh_RCNSubObj",
        "SimMesh_Skin",
        "SimMesh_SimLinkSrc",
        "StrTbl",
        "String",
        "NodeTbl",
        "SimMesh_OldVtxSave",
        "SimLine",
        "SimLine_LineDef",
        "SimLine_AssignNode",
        "BOGUS_30",
        "SimMesh_Pattern",
        "ColIDTbl",
        "SimMesh_CtStretchLink",
        "Box_S",
        "Box_D",
        "ColIDInfo",
        "SimMesh_ColVtxFast",
        "ColMeshLink_Model",
        "ColMeshLink_Object",
        "ColMeshLink_SubObj",
        "ColMeshLink_Col",
        "ColMeshLink_Capsule",
        "SimMesh_CtNodeFixation",
        "SimMesh_AnimNodeFixation",
        "SimMesh_Wind",
        "SimMesh_TarNodeLink",
        "SimMesh_SrcNodeLink",
        "SimMesh_CtLinkFixation",
        "SimMesh_CtCancelFixation",
        "SimMesh_SelfLinkCapsule",
        "ColNameTbl",
        "Box_R",
        "SimMesh_Friction",
        "SimMesh_CtLinkFixationST",
        "SimMesh_CtEdgeLength",
        "SimMesh_BendingStiffness"    };

    return enumStrings.at(tagEnum);
}





