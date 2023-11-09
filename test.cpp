//
// Created by eszdman on 08.11.23.
//
#include <iostream>
#include <cstdio>
#include "DngWriter.h"

using namespace std;

int printfln(const char* format, ...){
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\n");
    va_end(args);
    return 0;
}

int main(){
    DngProfile* dngprofile = new DngProfile();
    CustomMatrix* matrix = new CustomMatrix();
    dngprofile->blacklevel = new float[4];
    for (size_t i = 0; i < 4; i++)
    {
        dngprofile->blacklevel[i] = 0.f;
    }
    dngprofile->whitelevel = 65535;
    dngprofile->rawwidht = 512;
    dngprofile->rawheight = 512;
    dngprofile->rowSize = 0;

    /*char cfaar[4];
    cfaar[0] = raw.imgdata.idata.cdesc[raw.COLOR(0, 0)];
    cfaar[1] = raw.imgdata.idata.cdesc[raw.COLOR(0, 1)];
    cfaar[2] = raw.imgdata.idata.cdesc[raw.COLOR(1, 0)];
    cfaar[3] = raw.imgdata.idata.cdesc[raw.COLOR(1, 1)];*/

    string cfa = "BGGR";
    if (cfa == "BGGR")
    {
        dngprofile->bayerformat = "bggr";
    }
    else if (cfa == "RGGB")
    {
        dngprofile->bayerformat = "rggb";
    }
    else if (cfa == "GRBG")
    {
        dngprofile->bayerformat = "grbg";
    }
    else
    {
        dngprofile->bayerformat = "gbrg";
    }

    dngprofile->rawType = DNG_16BIT;


    /*matrix->colorMatrix1 = new float[9];
    matrix->colorMatrix2 = new float[9];
    matrix->neutralColorVector = new float[3];
    matrix->fowardMatrix1 = new float[9];
    matrix->fowardMatrix2 = new float[9];
    matrix->calibrationMatrix1 = new float[9];
    matrix->calibrationMatrix2 = new float[9];*/

    /*copyMatrix(matrix->colorMatrix1, raw.imgdata.color.dng_color[0].colormatrix);
    copyMatrix(matrix->colorMatrix2, raw.imgdata.color.dng_color[1].colormatrix);
    copyMatrix(matrix->neutralColorVector, raw.imgdata.color.cam_mul);
    copyMatrix(matrix->fowardMatrix1, raw.imgdata.color.dng_color[0].forwardmatrix);
    copyMatrix(matrix->fowardMatrix2, raw.imgdata.color.dng_color[1].forwardmatrix);
    copyMatrix(matrix->calibrationMatrix1, raw.imgdata.color.dng_color[0].calibration);
    copyMatrix(matrix->calibrationMatrix2, raw.imgdata.color.dng_color[1].calibration);
    cout << "copy colormatrix end:" << filename << "\n";*/
    float ncv[] = {1,1,1};
    matrix->neutralColorVector = ncv;
    float idm[] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    matrix->colorMatrix1 = idm;
    matrix->colorMatrix2 = idm;
    matrix->fowardMatrix1 = idm;
    matrix->fowardMatrix2 = idm;
    matrix->calibrationMatrix1 = idm;
    matrix->calibrationMatrix2 = idm;

    size_t width = 512;
    size_t height = 512;
    auto outBuff = new uint16_t[width*height];
    for(int i =0; i<width; i++){
        for(int j =0; j<height; j++){
            outBuff[i*height+j] = uint16_t(65535.f*(float(i)/float(width - 1) + float(j)/float(height - 1))/2.f);
        }
    }

    DngWriter *dngw = new DngWriter(printfln);

    dngw->dngProfile = dngprofile;
    dngw->customMatrix = matrix;
    dngw->bayerBytes = reinterpret_cast<unsigned char*>(outBuff);
    dngw->rawSize = 512*512*2;
    dngw->fileSavePath = (char*)"./test.dng";
    dngw->_make = "make";
    dngw->_model = "model";

    dngw->WriteDNG();
}