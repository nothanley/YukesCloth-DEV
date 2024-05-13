// YukesCloth.cpp : This file contains the 'main' function. Program execution begins and ends there. //

#include <iostream>
#include "console_runner.hpp"

int main(int argc, char* argv[])
{
    //std::string path("C:\\Users\\wauke\\Desktop\\1025_Attire_2023.ycl");

    if (argc != 2) {
        runconv::run_dir_conv();
    }
    else 
    {
        printf("\n{YCL Conv} Processing Input...\n");
        std::string path(argv[1]);

        try {
            runconv::run_cloth_conv(path);
        }
        catch (...) {
            printf("\n{YCL Conv} Failed to read YCL contents.");
        }
    }
   

    printf("\n\nAll operations complete.\n");
    system("pause");
}


