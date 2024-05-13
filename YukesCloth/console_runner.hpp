#include "YukesCloth"
#include <Encoder/Import/SimMeshJson.h>
#include <iostream>
#include "Container/clothfile.h"
#include "clothsave.h"
#include "common.h"
#include "windows.h"

namespace fs = std::filesystem;

namespace runconv 
{
    static std::vector<std::string> get_all_file_paths(const std::string& directory, const char* extension)
    {
        std::vector<std::string> yclPaths;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == extension) {
                yclPaths.push_back(entry.path().string());
            }
        }
        return yclPaths;
    }

    static std::string get_executable_directory() {
        char buffer[MAX_PATH];
        GetModuleFileName(NULL, buffer, MAX_PATH);
        std::string executable_path(buffer);
        size_t last_slash = executable_path.find_last_of('\\');
        return executable_path.substr(0, last_slash);
    }

    static void run_cloth_conv(const std::string& path)
    {
        CClothContainer yclfile(path.c_str());
        yclfile.open();
        auto yClothObj = yclfile.clothObj();

        if (yClothObj->getFileVersion() == YUKES_CLOTH_24) {
            printf("\n{YCL Conv} File is already formatted.\n");
            return;
        }

        std::string outPath = SysCommon::replace_substring(path, ".ycl", "_reformat.ycl");
        CClothSave::SaveToDisk(outPath.c_str(), yclfile.clothObj());
        printf("\n{YCL Conv} File reformat saved to '%s\n'", outPath.c_str());
    }

    static void run_dir_conv() 
    {
        std::string folder_path = get_executable_directory();

        if (folder_path.empty()){
            printf("\n{YCL Conv} Could not resolve working directory path.\n");
            return;
        }

        auto file_paths = get_all_file_paths(folder_path, ".ycl");
        int numPaths = file_paths.size();

        if (numPaths == 0){
            printf("\n{YCL Conv} Could not locate .ycl files in current directory.\n");
            return;
        }

        for (int i = 0; i < file_paths.size(); i++){
            auto& path = file_paths.at(i);
            printf("\n\n{YCL Conv} Processing file (%d/%d):\n", i+1, numPaths);

            try {
                run_cloth_conv(path);
            }
            catch (...) {
                printf("\n{YCL Conv} Failed to read YCL contents.\n");
            }
        }

    }

}