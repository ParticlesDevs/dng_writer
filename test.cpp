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
    dngprofile->rawwidht = 4000;
    dngprofile->rawheight = 3000;
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

    float ncv[] = {1,1,1};
    matrix->neutralColorVector = ncv;
    float idm[] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    matrix->colorMatrix1 = idm;
    matrix->colorMatrix2 = idm;
    matrix->fowardMatrix1 = idm;
    matrix->fowardMatrix2 = idm;
    matrix->calibrationMatrix1 = idm;
    matrix->calibrationMatrix2 = idm;

    size_t width = dngprofile->rawwidht;
    size_t height = dngprofile->rawheight;
    auto outBuff = new uint16_t[width*height];
    for(int i =0; i<height; i++){
        for(int j =0; j<width; j++){
            outBuff[i*width+j] = uint16_t(65535.f*(float(i)/float(height - 1) + float(j)/float(width - 1))/2.f);
        }
    }

    DngWriter *dngw = new DngWriter(printfln);
    dngw->compression = COMPRESSION_JPEG;
    dngw->dngProfile = dngprofile;
    dngw->customMatrix = matrix;
    dngw->bayerBytes = reinterpret_cast<unsigned char*>(outBuff);
    dngw->rawSize = dngprofile->rawwidht*dngprofile->rawheight*2;
    dngw->fileSavePath = (char*)"./test.dng";
    dngw->make = "make";
    dngw->model = "model";
    dngw->software = "software";

    dngw->WriteDNG();
}