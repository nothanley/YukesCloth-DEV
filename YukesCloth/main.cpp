// YukesCloth.cpp : This file contains the 'main' function. Program execution begins and ends there. //
#include "YukesCloth"
#include <Encoder/Import/SimMeshJson.h>
#include <iostream>

int main()
{
    CClothContainer yclfile(
        "C:\\Users\\wauke\\Desktop\\hair_00170-24.ycl");

    yclfile.open();

    //CClothSave::SaveToDisk(
        //"C:\\Users\\wauke\\Desktop\\out_file.ycl", 
        //_yclContainer.m_pClothSimObj);

    //CJsonSimMesh jsonMesh("C:\\Users\\wauke\\Desktop\\plane_source.jsim");
}
