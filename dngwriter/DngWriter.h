//
// Created by troop on 23.10.2016.
//
#pragma once

//#define LOG_RAW_DATA


//shift 10bit tight data into readable bitorder
#define DNG_10BIT_TIGHT_SHIFT 0
//shift 10bit loose data into readable bitorder
#define DNG_10BIT_LOOSE_SHIFT 1
//drops the 6 first bit from pure 16bit data(mtk soc, Camera2 RAW_SENSOR)
#define DNG_16BIT_TO_10BIT 2
//convert and shift 10bit tight data into 16bit pure
#define DNG_10BIT_TO_16BIT 3
//shift 12bit data into readable bitorder
#define DNG_12BIT_SHIFT 4

#define DNG_16BIT_TO_12BIT 5
#define DNG_16BIT 6
#define DNG_QUADBAYER_16BIT 7
#define DNG_16_TO_LOSSLESS 8

#include "tiffio.h"
#include "tiffiop.h"
#include "ExifInfo.h"
#include "GpsInfo.h"
#include "DngProfile.h"
#include "CustomMatrix.h"
#include "OpCode.h"
#include "tif_dir.h"
#include <assert.h>
#include <stdlib.h>


#define LOSY_JPEG 34892
#define LINEAR_RAW 34892

//typedef unsigned long long uint64;
class DngWriter
{

private:
    int bpp = 10;
    int (*logWriter)(const char* format, ...);
    //open tiff from filepath
    TIFF *openfTIFF(char* fileSavePath);
    //open tiff from filedescriptor
    TIFF *openfTIFFFD(char* fileSavePath, int fd);
    void writeIfd0(TIFF *tif);
    void makeGPS_IFD(TIFF *tif);
    void writeExifIfd(TIFF *tif);
    void processTight(TIFF *tif);
    void process10tight(TIFF *tif);
    void process12tight(TIFF *tif);
    void processLoose(TIFF *tif);
    void processSXXX16(TIFF *tif);
    void processSXXX16crop(TIFF *tif);
    void process16to10(TIFF *tif);
    void process16to12(TIFF *tif);
    void writeRawBuffer(TIFF *tif);
    void quadBayer16bit(TIFF *tif);
    void process16ToLossless(TIFF *tiff);
    unsigned short getColor(int row, int col);

public:
    ExifInfo *exifInfo;
    GpsInfo *gpsInfo;
    DngProfile * dngProfile;
    CustomMatrix * customMatrix;
    OpCode * opCode = nullptr;
    const char* make = nullptr;
    const char* model = nullptr;
    const char* dateTime = nullptr;
    const char* software = nullptr;


    long rawSize{};

    char *fileSavePath;
    long fileLength;
    unsigned char* bayerBytes;

    float *tonecurve;
    int tonecurvesize;
    float *huesatmapdata1;
    int huesatmapdata1_size;
    float *huesatmapdata2;
    int huesatmapdata2_size;
    float baselineExposure{};
    float baselineExposureOffset{};
    unsigned int bayergreensplit{};

    int *huesatmapdims;


    int fileDes;
    bool hasFileDes;

    int crop_width;
    int crop_height;
    int compression = COMPRESSION_NONE;

    int thumbheight{}, thumwidth{};
    unsigned char* _thumbData{};

    explicit DngWriter(int printf(const char* format, ...))
    {
        logWriter = printf;
        exifInfo = nullptr;
        gpsInfo = nullptr;
        dngProfile = nullptr;
        customMatrix = nullptr;
        opCode = nullptr;
        fileDes = -1;
        hasFileDes = false;
        tonecurve = nullptr;
        fileSavePath = nullptr;
        fileLength = 0;
        bayerBytes = nullptr;
        tonecurvesize = 0;
        huesatmapdata1 = nullptr;
        huesatmapdata1_size = 0;
        huesatmapdata2 = nullptr;
        huesatmapdata2_size = 0;
        huesatmapdims = nullptr;
        crop_width = 0;
        crop_height = 0;
    }

    void WriteDNG();
    void clear();


};
