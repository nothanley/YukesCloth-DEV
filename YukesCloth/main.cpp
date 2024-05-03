// YukesCloth.cpp : This file contains the 'main' function. Program execution begins and ends there. //
#include "YukesCloth"
#include <Encoder/Import/SimMeshJson.h>
#include <iostream>
#include "Container/clothfile.h"
#include "clothsave.h"

int main()
{
    CClothContainer yclfile(
        //"C:\\Users\\wauke\\Desktop\\hair_00170-24.ycl");
        "C:\\Users\\wauke\\Desktop\\1031_Attire.ycl");

    yclfile.open();

    CClothSave::SaveToDisk(
        "C:\\Users\\wauke\\Desktop\\out_file.ycl", 
        yclfile.getClothObj() );

    //CJsonSimMesh jsonMesh("C:\\Users\\wauke\\Desktop\\plane_source.jsim");
}


