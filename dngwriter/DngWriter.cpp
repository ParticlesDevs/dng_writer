//
// Created by troop on 23.10.2016.
//
#include "DngTags.h"
#include <string.h>
#include <math.h>
#include "DngWriter.h"

extern "C" {
#include "lj92.h"
}


#ifdef LOG_RAW_DATA
const char *bit_rep[16] = {
        [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
        [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
        [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
        [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};
#endif


TIFF* DngWriter::openfTIFF(char *fileSavePath)
{
    TIFF *tif;
    if (!(tif = TIFFOpen (fileSavePath, "w")))
    {
        logWriter("openfTIFF:error while creating outputfile");
    }
    return tif;
}

TIFF* DngWriter::openfTIFFFD(char *fileSavePath, int fd) {
    TIFF *tif;

    logWriter("FD: %d", fd);
    if (!(tif = TIFFFdOpen (fd,fileSavePath, "w")))
    {
        logWriter("openfTIFFFD:error while creating outputfile");
    }
    return tif;
}

void DngWriter::writeIfd0(TIFF *tif) {
    int width,height, photometric;

    if (crop_width == 0 && crop_height == 0)
    {
        width = dngProfile->rawwidht;
        height = dngProfile->rawheight;
    }
    else
    {
        width = crop_width;
        height = crop_height;
    }

    if(dngProfile->rawType == DNG_10BIT_LOOSE_SHIFT
       || dngProfile->rawType == DNG_10BIT_TO_16BIT
       || dngProfile->rawType == DNG_16BIT
       || dngProfile->rawType == DNG_QUADBAYER_16BIT && dngProfile->rawType == DNG_16_TO_LOSSLESS)
        bpp = 16;
    else if (dngProfile->rawType == DNG_12BIT_SHIFT || dngProfile->rawType == DNG_16BIT_TO_12BIT)
        bpp = 12;
    else
        bpp = 10;

    if(dngProfile->rawType == DNG_16_TO_LOSSLESS)
        compression = COMPRESSION_JPEG;

    if (compression == COMPRESSION_NONE || compression == COMPRESSION_JPEG)
        photometric = PHOTOMETRIC_CFA;
    else if (compression == LOSY_JPEG)
        photometric = LINEAR_RAW;

    char cfa[4] = {0,0,0,0};
    if(0 == strcmp(dngProfile->bayerformat,"bggr")){
        cfa[0] = 2;cfa[1] = 1;cfa[2] = 1;cfa[3] = 0;}
    //TIFFSetField (tif, TIFFTAG_CFAPATTERN, "\002\001\001\0");// 0 = Red, 1 = Green, 2 = Blue, 3 = Cyan, 4 = Magenta, 5 = Yellow, 6 = White
    if(0 == strcmp(dngProfile->bayerformat , "grbg")){
        cfa[0] = 1;cfa[1] = 0;cfa[2] = 2;cfa[3] = 1;}//TIFFSetField (tif, TIFFTAG_CFAPATTERN, "\001\0\002\001");
    if(0 == strcmp(dngProfile->bayerformat , "rggb")){
        cfa[0] = 0;cfa[1] = 1;cfa[2] = 1;cfa[3] = 2;}//TIFFSetField (tif, TIFFTAG_CFAPATTERN, "\0\001\001\002");
    if(0 == strcmp(dngProfile->bayerformat , "gbrg")){
        cfa[0] = 1;cfa[1] = 2;cfa[2] = 0;cfa[3] = 1;}//TIFFSetField (tif, TIFFTAG_CFAPATTERN, "\001\002\0\001");
    if(0 == strcmp(dngProfile->bayerformat , "rgbw")){
        cfa[0] = 0;cfa[1] = 1;cfa[2] = 2;cfa[3] = 6;}//TIFFSetField (tif, TIFFTAG_CFAPATTERN, "\0\001\002\006");

    logWriter("cfa pattern %i%i%i%i", cfa[0],cfa[1],cfa[2],cfa[3]);

    short CFARepeatPatternDim[] = { 2,2 };
    float margin = 8;
    float scale[] = {1,1};
    float  defaultCropOrigin[] = {margin, margin};
    float  defaultCropSize[2];
    if (crop_width == 0 && crop_height == 0) {
        defaultCropSize[0] = width - defaultCropOrigin[0] - margin;
        defaultCropSize[1] = height - defaultCropOrigin[1] - margin;
    }
    else
    {
        defaultCropSize[0] = width - defaultCropOrigin[0] - margin;
        defaultCropSize[1] = height - defaultCropOrigin[1] - margin;
    }
    logWriter("defaultCropSize %f %f", defaultCropSize[0], defaultCropSize[1]);

    TIFFSetField(tif, TIFFTAG_DNGVERSION, "\001\004\0\0");
    TIFFSetField(tif, TIFFTAG_DNGBACKWARDVERSION, "\001\001\0\0");
    TIFFSetField (tif, TIFFTAG_SUBFILETYPE, 0);
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
    logWriter("width x height:  %i x %i", width,height);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bpp);
    logWriter("bitspersample %i", bpp);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, compression);
    logWriter("Compression %i", compression);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric);
    logWriter("PhotometricCFA %i", photometric);

    TIFFSetField(tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
    if(exifInfo != nullptr)
        TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, exifInfo->_imagedescription);
    TIFFSetField(tif, TIFFTAG_MAKE, make);
    logWriter("make %s", make);
    TIFFSetField(tif, TIFFTAG_MODEL, model);
    logWriter("model %s",model);
    if (exifInfo != nullptr)
    {
        try
        {
            if (0 == strcmp(exifInfo->_orientation, "0"))
                TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
            if (0 == strcmp(exifInfo->_orientation, "90"))
                TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_RIGHTTOP);
            if (0 == strcmp(exifInfo->_orientation, "180"))
                TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_BOTRIGHT);
            if (0 == strcmp(exifInfo->_orientation, "270"))
                TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_LEFTBOT);
            logWriter("orientation %s",exifInfo->_orientation);
        }
        catch (...)
        {
            logWriter("failed to set orientation");
        }
    }
    else
        TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField (tif, TIFFTAG_SAMPLESPERPIXEL, 1);
    logWriter("sampelsperpixel %i" ,1);
    TIFFSetField( tif, TIFFTAG_XRESOLUTION, 72.f);
    TIFFSetField( tif, TIFFTAG_YRESOLUTION, 72.f);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
    if(software != nullptr)
        TIFFSetField(tif, TIFFTAG_SOFTWARE, "DNG Writer");
    if(dateTime != nullptr)
        TIFFSetField(tif,TIFFTAG_DATETIME, dateTime);
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
    //30k
    TIFFSetField(tif, TIFFTAG_CFAREPEATPATTERNDIM, CFARepeatPatternDim);
    TIFFSetField(tif, TIFFTAG_CFAPATTERN, 4, cfa);
    TIFFSetField(tif, TIFFTAG_EP_STANDARD_ID, "\001\000\0\0");
    //50k
    if(model != nullptr){
        TIFFSetField(tif, TIFFTAG_UNIQUECAMERAMODEL, model);
        logWriter("CameraModel %s",model);
    }
    TIFFSetField( tif, TIFFTAG_CFAPLANECOLOR, 3, "\00\01\02" ); // RGB
    TIFFSetField( tif, TIFFTAG_CFALAYOUT, 1 );

    TIFFSetField (tif, TIFFTAG_BLACKLEVELREPEATDIM, CFARepeatPatternDim);
    TIFFSetField (tif, TIFFTAG_BLACKLEVEL, 4, dngProfile->blacklevel);
    logWriter("wrote blacklevel");
    TIFFSetField (tif, TIFFTAG_WHITELEVEL, 1, &dngProfile->whitelevel);
    TIFFSetField(tif,TIFFTAG_DEFAULTSCALE, scale);

    logWriter("defaultCropOrigin");
    //TIFFSetField(tif, TIFFTAG_DEFAULTCROPORIGIN, defaultCropOrigin);
    logWriter("defaultCropSize");
    //TIFFSetField(tif, TIFFTAG_DEFAULTCROPSIZE, defaultCropSize);

    TIFFSetField(tif, TIFFTAG_COLORMATRIX1, 9,customMatrix->colorMatrix1);
    TIFFSetField(tif, TIFFTAG_COLORMATRIX2, 9, customMatrix->colorMatrix2);

    logWriter("calibrationMatrix1");
    if(customMatrix->calibrationMatrix1 != nullptr)
        TIFFSetField(tif, TIFFTAG_CAMERACALIBRATION1, 9,  customMatrix->calibrationMatrix1);
    logWriter("calibrationMatrix2");
    if(customMatrix->calibrationMatrix2 != nullptr)
        TIFFSetField(tif, TIFFTAG_CAMERACALIBRATION2, 9,  customMatrix->calibrationMatrix2);
    TIFFSetField(tif, TIFFTAG_ASSHOTNEUTRAL, 3, customMatrix->neutralColorVector);

    logWriter("baselineExposure");
    double baseS = baselineExposure;
    TIFFSetField(tif,TIFFTAG_BASELINEEXPOSURE, baseS);

    //TIFFSetField(tif,TIFFTAG_BAYERGREENSPLIT, bayergreensplit);

    //STANDARD A = FIIRST 17
    //D65 21 Second According to DNG SPEC 1.4 this is the correct order
    TIFFSetField(tif, TIFFTAG_CALIBRATIONILLUMINANT1, 21);
    TIFFSetField(tif, TIFFTAG_CALIBRATIONILLUMINANT2, 17);

    if(dngProfile->activearea != nullptr)
    {
        TIFFSetField(tif,TIFFTAG_ACTIVEAREA, dngProfile->activearea);
    }
    else
    {
        int active_area[] = {0,0,height,width};
        logWriter("activearea %i %i %i %i", active_area[0],active_area[1],active_area[2],active_area[3]);
        TIFFSetField(tif,TIFFTAG_ACTIVEAREA, active_area);
    }

    logWriter("huesatmapdims");
    if(huesatmapdims != nullptr)
    {
        TIFFSetField(tif, TIFFTAG_PROFILEHUESATMAPDIMS, 3, huesatmapdims);
    }
    logWriter("huesatmapdata1");
    if(huesatmapdata1 != nullptr)
    {
        TIFFSetField(tif,TIFFTAG_PROFILEHUESATMAPDATA1, huesatmapdata1_size,huesatmapdata1);
    }
    logWriter("huesatmapdata2");
    if(huesatmapdata2 != nullptr)
    {
        TIFFSetField(tif,TIFFTAG_PROFILEHUESATMAPDATA2, huesatmapdata2_size,huesatmapdata2);
    }
    logWriter("tonecurve");
    if(tonecurve != nullptr)
    {
        TIFFSetField(tif,TIFFTAG_PROFILETONECURVE, tonecurvesize,tonecurve);
    }
    logWriter("fowardMatrix1");
    if(customMatrix->fowardMatrix1 != nullptr)
        TIFFSetField(tif, TIFFTAG_FOWARDMATRIX1, 9,  customMatrix->fowardMatrix1);
    logWriter("fowardMatrix2");
    if(customMatrix->fowardMatrix2 != nullptr)
        TIFFSetField(tif, TIFFTAG_FOWARDMATRIX2, 9,  customMatrix->fowardMatrix2);
    //51k
    /*if(opCode != nullptr){
        if(opCode->op2Size > 0){
            logWriter("Set OP2 %i", opCode->op2Size);
            TIFFSetField(tif, TIFFTAG_OPC2, opCode->op2Size, opCode->op2);
        }else{
            logWriter("opcode2 nullptr");
        }
        if(opCode->op3Size > 0){
            logWriter("Set OP3 %i", opCode->op3Size);
            TIFFSetField(tif, TIFFTAG_OPC3, opCode->op3Size, opCode->op3);
        }else{
            logWriter("opcode3 nullptr");
        }
    } else{
        logWriter("opcode nullptr");
    }*/
    logWriter("noiseMatrix");
    if(customMatrix->noiseMatrix != nullptr)
        TIFFSetField(tif, TIFFTAG_NOISEPROFILE, 6,  customMatrix->noiseMatrix);
    logWriter("baselineExposureOffset");
    if(baselineExposureOffset != 0.f)
    {
        TIFFSetField(tif,TIFFTAG_BASELINEEXPOSUREOFFSET, baselineExposureOffset);
    }
}

void DngWriter::makeGPS_IFD(TIFF *tif) {
    logWriter("GPS IFD DATA");
    if (TIFFCreateGPSDirectory(tif) != 0)
    {
        logWriter("TIFFCreateGPSDirectory() failed" );
    }
    if (!TIFFSetField( tif, GPSTAG_GPSVersionID, "\002\003\0\0"))
    {
        logWriter("Can't write GPSVersionID" );
    }
    logWriter("Wrote GPSVersionID" );

    const char* longitudeRef = "E";
    if (gpsInfo->Longitude[0] < 0) {
        longitudeRef = "W";
        gpsInfo->Longitude[0] = fabsf(gpsInfo->Longitude[0]);
    }
    if (!TIFFSetField( tif, GPSTAG_GPSLongitudeRef, longitudeRef))
    {
        logWriter("Can't write LongitudeRef" );
    }
    logWriter("LONG REF Written %c", longitudeRef);

    if (!TIFFSetField(tif, GPSTAG_GPSLongitude, gpsInfo->Longitude))
    {
        logWriter("Can't write Longitude" );
    }
    logWriter("Longitude Written");
    const char* latitudeRef = "N";
    if (gpsInfo->Latitude[0] < 0) {
        latitudeRef = "S";
        gpsInfo->Latitude[0] = fabsf(gpsInfo->Latitude[0]);
    }
    logWriter("PMETH Written");
    if (!TIFFSetField( tif, GPSTAG_GPSLatitudeRef, latitudeRef)) {
        logWriter("Can't write LAti REf" );
    }
    logWriter("LATI REF Written %c", latitudeRef);

    if (!TIFFSetField( tif, GPSTAG_GPSLatitude,gpsInfo->Latitude))
    {
        logWriter("Can't write Latitude" );
    }
    logWriter("Latitude Written");
    if (!TIFFSetField( tif, GPSTAG_GPSAltitude,gpsInfo->Altitude))
    {
        logWriter("Can't write Altitude" );
    }
    logWriter("Altitude Written");

    if (!TIFFSetField( tif, GPSTAG_GPSTimeStamp, gpsInfo->gpsTime))
    {
        logWriter("Can't write gpsTime" );
    }
    logWriter("GPSTimeStamp Written");

    if (!TIFFSetField( tif, GPSTAG_GPSDateStamp, gpsInfo->gpsDate))
    {
        logWriter("Can't write gpsTime" );
    }
    logWriter("GPSTimeDate Written");
}

void DngWriter::writeExifIfd(TIFF *tif) {

    /////////////////////////////////// EXIF IFD //////////////////////////////
    short iso[] = {static_cast<short>(exifInfo->_iso)};
    logWriter("EXIF dir created");
    if (!TIFFSetField( tif, EXIFTAG_ISOSPEEDRATINGS,1, iso)) {
        logWriter("Can't write SPECTRALSENSITIVITY" );
    }
    logWriter("iso");
    if (!TIFFSetField( tif, EXIFTAG_FLASH, exifInfo->_flash)) {
        logWriter("Can't write Flas" );
    }
    logWriter("flash");
    if (!TIFFSetField( tif, EXIFTAG_APERTUREVALUE, exifInfo->_fnumber)) {
        logWriter("Can't write Aper" );
    }
    logWriter("aperture");

    if (!TIFFSetField( tif, EXIFTAG_EXPOSURETIME,exifInfo->_exposure)) {
        logWriter("Can't write SPECTRALSENSITIVITY" );
    }
    logWriter("exposure");


    if (!TIFFSetField( tif, EXIFTAG_FOCALLENGTH, exifInfo->_focallength)) {
        logWriter("Can't write Focal" );
    }
    logWriter("focal");

    if (!TIFFSetField( tif, EXIFTAG_FNUMBER, exifInfo->_fnumber)) {
        logWriter("Can't write FNum" );
    }

    if(!TIFFSetField(tif,EXIFTAG_EXPOSUREINDEX, exifInfo->_exposureIndex))
        logWriter("Cant write expoindex");
    logWriter("fnumber");
}

//process mipi10bit to 16bit 10bit values stored
void DngWriter::processTight(TIFF *tif) {
    logWriter("IN SXXXXl0");
    int i, j, row, col, b;
    unsigned char *buffer, *dp;
    unsigned short pixel[dngProfile->rawwidht]; // array holds 16 bits per pixel

    logWriter("buffer set");
    j=0;
    if(dngProfile->rowSize == 0)
        dngProfile->rowSize =  -(-5 * dngProfile->rawwidht >> 5) << 3;
    buffer =(unsigned char *)malloc(dngProfile->rowSize);
    memset( buffer, 0, dngProfile->rowSize);
    if (buffer == nullptr)
    {
        logWriter("allocating buffer failed try again");
        buffer =(unsigned char *)malloc(dngProfile->rowSize);
    }
    logWriter("rowsize:%i", dngProfile->rowSize);

    for (row=0; row < dngProfile->rawheight; row ++)
    {
        i = 0;
        for(b = row * dngProfile->rowSize; b < row * dngProfile->rowSize + dngProfile->rowSize; b++)
            buffer[i++] = bayerBytes[b];
        for (dp=buffer, col = 0; col < dngProfile->rawwidht; dp+=5, col+= 4)
        {
            for(int i = 0; i< 4; i++)
            {
                pixel[col+i] = (dp[i] <<2) | (dp[4] >> (i << 1) & 3);
            }
        }

        if (TIFFWriteScanline(tif, pixel, row, 0) != 1) {
            logWriter("Error writing TIFF scanline.");
        }
    }
    logWriter("Write done");
    if(buffer != nullptr)
    {
        logWriter("Free Buffer");
        free(buffer);
        logWriter("Freed Buffer");
    }
    logWriter("Mem Released");
}

//shift 10bit mipi into 10bit readable raw data
void DngWriter::process10tight(TIFF *tif) {
    unsigned char *ar = bayerBytes;
    int bytesToSkip = 0;
    int realrowsize;
    int shouldberowsize;
    unsigned char* out;
    logWriter("writer-RowSize: %d  rawheight:%d ,rawwidht: %d", rawSize, dngProfile->rawheight,
         dngProfile->rawwidht);

    realrowsize = -(-5 * dngProfile->rawwidht >> 5) << 3;
    shouldberowsize = realrowsize;
    if (realrowsize % 5 > 0) {
        shouldberowsize = dngProfile->rawwidht * 10 / 8;
        bytesToSkip = realrowsize - shouldberowsize;
    }
    logWriter("realrow: %i shoudlbe: %i", realrowsize, shouldberowsize);
    logWriter("width: %i height: %i", dngProfile->rawwidht, dngProfile->rawheight);
    logWriter("bytesToSkip: %i", bytesToSkip);

    int row = shouldberowsize;
    out = new unsigned char[shouldberowsize*dngProfile->rawheight];
    if(out == nullptr)
    {
        logWriter("failed to set buffer");
        return;
    }

    int m = 0;
    for(int i =0; i< rawSize; i+=5)
    {
        if(i == row)
        {
            row += shouldberowsize + bytesToSkip;
            i+=bytesToSkip;
        }

        out[m++] = (ar[i]); // 00110001
        out[m++] =  (ar[i+4] & 0b00000011 ) <<6 | (ar[i+1] & 0b11111100)>>2; // 01 001100
        out[m++] = (ar[i+1]& 0b00000011 )<< 6 | (ar[i+4] & 0b00001100 ) <<2 | (ar[i +2] & 0b11110000 )>> 4;// 10 01 0011
        out[m++] = (ar[i+2] & 0b00001111 ) << 4 | (ar[i+4] & 0b00110000 )>> 2| (ar[i+3]& 0b11000000)>>6; // 0011 11 00
        out[m++] = (ar[i+3]& 0b00111111)<<2 | (ar[i+4]& 0b11000000)>>6;//110100 00
    }
    TIFFWriteRawStrip(tif, 0, out, dngProfile->rawheight*shouldberowsize);
    logWriter("Finalizng DNG");
    delete[] out;
}

void DngWriter::process12tight(TIFF *tif) {
    unsigned char* ar = bayerBytes;
    int bytesToSkip = 0;
    logWriter("writer-RowSize: %d  rawheight:%d ,rawwidht: %d",  rawSize,dngProfile->rawheight, dngProfile->rawwidht);
    int realrowsize = rawSize/dngProfile->rawheight;
    int shouldberowsize = dngProfile->rawwidht*12/8;
    logWriter("realrow: %i shoudlbe: %i", realrowsize, shouldberowsize);
    if (realrowsize != shouldberowsize)
        bytesToSkip = realrowsize - shouldberowsize;
    logWriter("bytesToSkip: %i", bytesToSkip);
    int row = shouldberowsize;
    unsigned char* out = new unsigned char[shouldberowsize*dngProfile->rawheight];
    int m = 0;
    for(int i =0; i< rawSize; i+=3)
    {
        if(i == row)
        {
            row += shouldberowsize +bytesToSkip;
            i+=bytesToSkip;
        }
        out[m++] = (ar[i]); // 00110001
        out[m++] = (ar[i+2] & 0b11110000 ) <<4 | (ar[i+1] & 0b11110000)>>4; // 01 001100
        out[m++] = (ar[i+1]& 0b00001111 )<< 4 | (ar[i+2] & 0b00001111 ) >>4 ;// 10 01 0011
    }
    TIFFWriteRawStrip(tif, 0, out, dngProfile->rawheight*shouldberowsize);
    logWriter("Finalizng DNG");
    delete[] out;
}

void DngWriter::processLoose(TIFF *tif) {
    int i, row, col, b;
    unsigned char *buffer, *dp;
    unsigned short pixel[dngProfile->rawwidht]; // array holds 16 bits per pixel

    uint64 colorchannel;

    dngProfile->rowSize= (dngProfile->rawwidht+5)/6 << 3;
    buffer =(unsigned char *)malloc(dngProfile->rowSize);
    memset( buffer, 0, dngProfile->rowSize);
    if (buffer == nullptr)
    {
        logWriter("allocating buffer failed try again");
        buffer =(unsigned char *)malloc(dngProfile->rowSize);
    }
    for (row=0; row < dngProfile->rawheight; row ++)
    {
        i = 0;
        for(b = row * dngProfile->rowSize; b < (row * dngProfile->rowSize) + dngProfile->rowSize; b++)
            buffer[i++] = bayerBytes[b];
        /*
         * get 5 bytes from buffer and move first 4bytes to 16bit
         * split the 5th byte and add the value to the first 4 bytes
         * */
        for (dp=buffer, col = 0; col < dngProfile->rawwidht; dp+=8, col+= 6)
        { // iterate over pixel columns

            for(int i =0; i< 8; i++)
            {
                colorchannel = (colorchannel << 8) | dp[i^7];
            }

            for(int i =0; i< 6; i++)
            {
                pixel[col+i] = (colorchannel >> i*10) & 0x3ff;
            }

        }
        if (TIFFWriteScanline (tif, pixel, row, 0) != 1) {
            logWriter("Error writing TIFF scanline.");
        }
    }
    logWriter("Free Memory processLoose");
    if(buffer != nullptr)
    {
        logWriter("Free Buffer");
        free(buffer);
        logWriter("Freed Buffer");
    }

    logWriter("Mem Released");
}

void DngWriter::processSXXX16(TIFF *tif) {
    int j, row, col;
    unsigned short pixel[dngProfile->rawwidht];
    unsigned short pixel2[dngProfile->rawwidht];
    unsigned short low, high;
    j=0;
    for (row=0; row < dngProfile->rawheight; row+=2)
    {
        for (col = 0; col < dngProfile->rawwidht; col++)
        { // iterate over pixel columns
            int pos = (dngProfile->rawwidht * row + col)*2;
            low = bayerBytes[pos];
            high =   bayerBytes[pos+1];
            pixel[col] =  high << 8 |low;
            pos = (dngProfile->rawwidht * (row+1) + col)*2;
            low = bayerBytes[pos];
            high =   bayerBytes[pos+1];
            pixel2[col] =  high << 8 |low;
        }
        if (TIFFWriteScanline (tif, pixel, row, 0) != 1) {
            logWriter("Error writing TIFF scanline.");
        }
        if (TIFFWriteScanline (tif, pixel2, row+1, 0) != 1) {
            logWriter("Error writing TIFF scanline.");
        }
    }

    logWriter("Finalizng DNG");
    logWriter("Free Memory processSXXX16");
}

//taken from https://github.com/ifb/makeDNG
void DngWriter::process16ToLossless(TIFF *tiff) {
    int height = dngProfile->rawheight;
    int width = dngProfile->rawwidht;
    int new_width = int(width / 2);
    int new_height = int(height);
    TIFFSetField( tiff, TIFFTAG_TILEWIDTH, uint32_t(new_width));
    logWriter("wrote TIFFTAG_TILEWIDTH %i", new_width);
    TIFFSetField( tiff, TIFFTAG_TILELENGTH, uint32_t(height));
    logWriter("wrote TIFFTAG_TILELENGTH %i", height);
    logWriter("width %i", width);
    int ret = 0;
    uint8_t* input = bayerBytes;
    uint8_t* encoded = nullptr;
    int encodedLength = 0;
    ret = lj92_encode( (uint16_t*)&input[0], new_width, new_height, bpp, new_width, new_width, nullptr, 0, &encoded, &encodedLength );
    TIFFWriteRawTile(tiff, 0, encoded, encodedLength );
    logWriter("endcoded tile 0: %i", encodedLength);
    free( encoded );
    ret = lj92_encode( (uint16_t*)&input[width], new_width, new_height, bpp, new_width, new_width, nullptr, 0, &encoded, &encodedLength );
    TIFFWriteRawTile(tiff, 1, encoded, encodedLength );
    logWriter("encoded tile 1: %i", encodedLength);
    free( encoded );
}

void DngWriter::processSXXX16crop(TIFF *tif) {
    logWriter("processSXXX16crop");
    int j, row, col, j2;
    unsigned short pixel[crop_width];
    unsigned short low, high;
    int w_diff = dngProfile->rawwidht - crop_width;
    int h_diff = dngProfile->rawheight - crop_height;
    int offset_x = w_diff/2;
    int offset_y = h_diff/2;
    logWriter("crop w: %i h: %i offset x: %i y: %i" ,crop_width,crop_height, offset_x,offset_y);
    j=0;
    j2 = 0;
    for (row=offset_y; row < dngProfile->rawheight-offset_y; row++)
    {
        j=0;
        for (col = offset_x; col < dngProfile->rawwidht - offset_x; col++)
        { // iterate over pixel columns
            int pos = (dngProfile->rawwidht * row + col)*2;
            low = bayerBytes[pos];
            high =   bayerBytes[pos+1];
            pixel[j++] =  high << 8 |low;
        }
        if (TIFFWriteScanline (tif, pixel, j2++, 0) != 1) {
            logWriter("Error writing TIFF scanline.");
        }
    }

    logWriter("Finalizng DNG");
    logWriter("Free Memory processSXXX16crop");
}

unsigned short DngWriter::getColor(int row, int col)
{
    int pos = (dngProfile->rawwidht * row + col)*2;
    unsigned short low = bayerBytes[pos];
    unsigned short high =   bayerBytes[pos+1];
    return high << 8 | low;
}

void DngWriter::quadBayer16bit(TIFF *tif) {

    int row, col;
    unsigned short pixel[dngProfile->rawwidht];
    unsigned short pixel2[dngProfile->rawwidht];
    unsigned short pixel3[dngProfile->rawwidht];
    unsigned short pixel4[dngProfile->rawwidht];
    unsigned short r1,r2,r3,r4;
    unsigned short g1,g2,g3,g4;
    unsigned short gg1,gg2,gg3,gg4;
    unsigned short b1,b2,b3,b4;

    for (row=0; row < dngProfile->rawheight; row+=4)
    {
        for (col = 0; col < dngProfile->rawwidht; col+=4)
        { // iterate over pixel columns
            r1 = getColor(row,col);
            r2 = getColor(row, col+1);
            r3 = getColor(row+1,col);
            r4 = getColor(row+1,col+1);
            g1 = getColor(row,col+2);
            g2 = getColor(row,col+3);
            g3 = getColor(row+1,col+2);
            g4 = getColor(row+1,col+3);

            gg1 = getColor(row+2,col);
            gg2 = getColor(row+2,col+1);
            gg3 = getColor(row+3,col);
            gg4 = getColor(row+3,col+1);
            b1 = getColor(row+2,col+2);
            b2 = getColor(row+2,col+3);
            b3 = getColor(row+3,col+2);
            b4 = getColor(row+3,col+3);

            pixel[col] = r1;
            pixel[col+1] = g1;
            pixel[col+2] = r2;
            pixel[col+3] = g2;
            pixel3[col] = r3;
            pixel3[col+1] = g3;
            pixel3[col+2] = r4;
            pixel3[col+3] = g4;

            pixel2[col] = gg1;
            pixel2[col+1] = b1;
            pixel2[col+2] = gg2;
            pixel2[col+3] = b2;
            pixel4[col] = gg3;
            pixel4[col+1] = b3;
            pixel4[col+2] = gg4;
            pixel4[col+3] = b4;

        }
        if (TIFFWriteScanline (tif, pixel, row, 0) != 1) {
            logWriter("Error writing TIFF scanline.");
        }
        if (TIFFWriteScanline (tif, pixel2, row+1, 0) != 1) {
            logWriter("Error writing TIFF scanline.");
        }
        if (TIFFWriteScanline (tif, pixel3, row+2, 0) != 1) {
            logWriter("Error writing TIFF scanline.");
        }
        if (TIFFWriteScanline (tif, pixel4, row+3, 0) != 1) {
            logWriter("Error writing TIFF scanline.");
        }
    }

    logWriter("Finalizng DNG");
    logWriter("Free Memory quadBayer16bit");
}

void DngWriter::process16to10(TIFF *tif) {
    long j;
    int rowsizeInBytes= dngProfile->rawwidht*10/8;
    long finalsize = rowsizeInBytes * dngProfile->rawheight;
    unsigned char* byts= bayerBytes;
    unsigned char* pixel = new unsigned char[finalsize];
    unsigned char B_ar[2];
    unsigned char G1_ar[2];
    unsigned char G2_ar[2];
    unsigned char R_ar[2];
    j=0;
    for (long i = 0; i < finalsize; i +=5)
    {

        B_ar[0] = byts[j];
        B_ar[1] = byts[j+1];
#ifdef LOG_RAW_DATA
        logWriter("P:%i B0:%s%s,B1:%s%s", i, bit_rep[B_ar[0] >> 4], bit_rep[B_ar[0] & 0x0F], bit_rep[B_ar[1] >> 4], bit_rep[B_ar[1] & 0x0F]);
#endif

        G1_ar[0] = byts[j+2];
        G1_ar[1] = byts[j+3];
#ifdef LOG_RAW_DATA
        logWriter("P:%i G10:%s%s,G11:%s%s", i, bit_rep[G1_ar[0] >> 4], bit_rep[G1_ar[0] & 0x0F], bit_rep[G1_ar[1] >> 4], bit_rep[G1_ar[1] & 0x0F]);
#endif
        G2_ar[0] = byts[j+4];
        G2_ar[1] = byts[j+5];
#ifdef LOG_RAW_DATA
        logWriter("P:%i G20:%s%s,G21:%s%s", i, bit_rep[G2_ar[0] >> 4], bit_rep[G2_ar[0] & 0x0F], bit_rep[G2_ar[1] >> 4], bit_rep[G2_ar[1] & 0x0F]);
#endif

        R_ar[0] = byts[j+6];
        R_ar[1] = byts[j+7];
#ifdef LOG_RAW_DATA
        logWriter("P:%i R0:%s%s,R1:%s%s", i, bit_rep[R_ar[0] >> 4], bit_rep[R_ar[0] & 0x0F], bit_rep[R_ar[1] >> 4], bit_rep[R_ar[1] & 0x0F]);
#endif
        j+=8;

        //00000011 1111111      H11 111111
        //00000011 1111111      11 H11 1111
        //00000011 1111111      1111 H11 11
        //00000011 1111111      111111 H11
        //00000011 1111111      11111111
        pixel[i] = (B_ar[1] & 0b00000011) << 6 | (B_ar[0] & 0b11111100) >> 2;//H11 111111
        pixel[i+1] =  (B_ar[0] & 0b00000011 ) << 6 | (G1_ar[1] & 0b00000011) << 4 | (G1_ar[0] & 0b11110000) >> 4;//11 H22 2222
        pixel[i+2] =  (G1_ar[0] & 0b00001111 ) << 4 | (G2_ar[1] & 0b00000011) << 2 | (G2_ar[0] & 0b11000000) >> 6; //2222 H33 33
        pixel[i+3] = (G2_ar[0] & 0b00111111 ) << 2 | (R_ar[1] & 0b00000011);//333333 H44
        pixel[i+4] = R_ar[0]; //44444444
    }
    TIFFWriteRawStrip(tif, 0, pixel, dngProfile->rawheight*rowsizeInBytes);
    logWriter("Finalizng DNG");

    logWriter("Free Memory process16to10");
    delete[] pixel;
}

void DngWriter::process16to12(TIFF *tif) {
    long j;
    int rowsizeInBytes= dngProfile->rawwidht*12/8;
    long finalsize = rowsizeInBytes * dngProfile->rawheight;
    unsigned char* byts= bayerBytes;
    unsigned char* pixel = new unsigned char[finalsize];
    unsigned char B_ar[3];
    unsigned char G1_ar[3];
    unsigned char G2_ar[3];
    unsigned char R_ar[3];
    j=0;
    for (long i = 0; i < finalsize; i +=6)
    {

        B_ar[0] = byts[j];
        B_ar[1] = byts[j+1];
#ifdef LOG_RAW_DATA
        logWriter("P:%i B0:%s%s,B1:%s%s", i, bit_rep[B_ar[0] >> 4], bit_rep[B_ar[0] & 0x0F], bit_rep[B_ar[1] >> 4], bit_rep[B_ar[1] & 0x0F]);
#endif

        G1_ar[0] = byts[j+2];
        G1_ar[1] = byts[j+3];
#ifdef LOG_RAW_DATA
        logWriter("P:%i G10:%s%s,G11:%s%s", i, bit_rep[G1_ar[0] >> 4], bit_rep[G1_ar[0] & 0x0F], bit_rep[G1_ar[1] >> 4], bit_rep[G1_ar[1] & 0x0F]);
#endif
        G2_ar[0] = byts[j+4];
        G2_ar[1] = byts[j+5];
#ifdef LOG_RAW_DATA
        logWriter("P:%i G20:%s%s,G21:%s%s", i, bit_rep[G2_ar[0] >> 4], bit_rep[G2_ar[0] & 0x0F], bit_rep[G2_ar[1] >> 4], bit_rep[G2_ar[1] & 0x0F]);
#endif

        R_ar[0] = byts[j+6];
        R_ar[1] = byts[j+7];
#ifdef LOG_RAW_DATA
        logWriter("P:%i R0:%s%s,R1:%s%s", i, bit_rep[R_ar[0] >> 4], bit_rep[R_ar[0] & 0x0F], bit_rep[R_ar[1] >> 4], bit_rep[R_ar[1] & 0x0F]);
#endif
        j+=8;

        //00001111 1111111      H1111 1111
        //00001111 1111111      1111 H1111
        //00001111 1111111      1111 1111
        //00001111 1111111      H1111 1111
        //00001111 1111111      1111 H1111
        //00001111 1111111      1111 1111

        pixel[i] = (B_ar[1] & 0b00001111) << 4 | (B_ar[0] & 0b11110000) >> 4;//B1111 1111
        pixel[i+1] =  (B_ar[0] & 0b00001111 ) << 4 | (G1_ar[1] & 0b00001111);//1111 G2222
        pixel[i+2] =  G1_ar[0];//2222 2222
        pixel[i+3] = (G2_ar[1] & 0b00001111 ) << 4 | (G2_ar[0] & 0b11110000)>>4;//3333 3333
        pixel[i+4] = (G2_ar[0] & 0b00001111 ) << 4 | (R_ar[1] &  0b00001111); //3333 4444
        pixel[i+5] = R_ar[0]; //4444 4444
    }
    TIFFWriteRawStrip(tif, 0, pixel, dngProfile->rawheight*rowsizeInBytes);
    logWriter("Finalizng DNG");

    logWriter("Free Memory process16to12");
    delete[] pixel;
}


void DngWriter::writeRawBuffer(TIFF *tif) {
    //**********************************************************************************

    if (compression == COMPRESSION_JPEG)
    {
        process16ToLossless(tif);
    }
    else if(dngProfile->rawType == DNG_10BIT_TIGHT_SHIFT)
    {
        logWriter("Processing tight RAW data...");
        process10tight(tif);
        logWriter("Done tight RAW data...");
    }
    else if (dngProfile->rawType == DNG_10BIT_LOOSE_SHIFT)
    {
        logWriter("Processing loose RAW data...");
        processLoose(tif);
        logWriter("Done loose RAW data...");
    }
    else if (dngProfile->rawType == DNG_16BIT_TO_10BIT) {
        logWriter("process16to10(tif);");
        process16to10(tif);
    }
    else if (dngProfile->rawType == DNG_10BIT_TO_16BIT) {
        logWriter("processTight(tif);");
        processTight(tif);
    }
    else if (dngProfile->rawType == DNG_12BIT_SHIFT) {
        logWriter("process12tight");
        process12tight(tif);
    }
    else if (dngProfile->rawType == DNG_16BIT_TO_12BIT) {
        logWriter("process16to12(tif);");
        process16to12(tif);
    }
    else if (dngProfile->rawType == DNG_16BIT)
    {
        if (crop_width == 0 && crop_height == 0)
            processSXXX16(tif);
        else
            processSXXX16crop(tif);
    }
    else if (dngProfile->rawType == DNG_QUADBAYER_16BIT)
        quadBayer16bit(tif);
    else
        logWriter("rawType is not implented");
}


void DngWriter::clear() {
    logWriter("delete Opcode2");
    opCode = nullptr;
    logWriter("delete bayerbytes");
    /*if (bayerBytes != nullptr){
        delete [] bayerBytes;
        rawSize = nullptr;
        bayerBytes = nullptr;
    }*/
    logWriter("delete filesavepath");
    if(fileSavePath != nullptr)
    {
        if(!hasFileDes)
            delete[] fileSavePath;
        fileSavePath = nullptr;
    }
    logWriter("delete exif");
    if(exifInfo != nullptr)
        exifInfo = nullptr;
    logWriter("delete dngprofile");
    if(dngProfile != nullptr)
        dngProfile = nullptr;
    logWriter("delete customMatrix");
    if(customMatrix != nullptr)
        customMatrix = nullptr;

    fileDes = 0;
    thumbheight = 0;
    thumwidth = 0;
}
int (*logWriterLocal)(const char* format, ...);
int errorHandler(const char* module, const char* fmt, va_list ap)
{
    char buf[1024];
    logWriterLocal("Module: %s: ", module);
    vsprintf(buf, fmt, ap);
    logWriterLocal("Value: %s", buf);
    return 0;
}

void DngWriter::WriteDNG() {
    _XTIFFInitialize();
    logWriterLocal = logWriter;
    TIFFSetWarningHandler(reinterpret_cast<TIFFErrorHandler>(errorHandler));
    TIFFSetErrorHandler(reinterpret_cast<TIFFErrorHandler>(errorHandler));
    logWriter("init ext tags");
    logWriter("init ext tags done");
    TIFF *tif;
    logWriter("has file description: %b", hasFileDes);
    if(hasFileDes == true)
    {
        tif = openfTIFFFD("", fileDes);
    }
    else
        tif = openfTIFF(fileSavePath);

    logWriter("writeIfd0");
    writeIfd0(tif);

    if(opCode != nullptr)
    {
        if(opCode->op2Size > 0)
        {
            logWriter("Set OP2 %i", opCode->op2Size);
            TIFFSetField(tif, TIFFTAG_OPC2, opCode->op2Size, opCode->op2);
        }
        else
        {
            logWriter("opcode2 nullptr");
        }
        if(opCode->op3Size > 0)
        {
            logWriter("Set OP3 %i", opCode->op3Size);
            TIFFSetField(tif, TIFFTAG_OPC3, opCode->op3Size, opCode->op3);
        }
        else
        {
            logWriter("opcode3 nullptr");
        }
    } else
    {
        logWriter("opcode nullptr");
    }
    logWriter("writeRawBuffer");
    writeRawBuffer(tif);

    TIFFWriteDirectory(tif);
    logWriter("set exif");
    if(exifInfo != nullptr)
    {
        uint64 exif_offset = 0;
        TIFFCreateEXIFDirectory(tif);
        writeExifIfd(tif);
        TIFFWriteCustomDirectory(tif, &exif_offset);
        TIFFSetDirectory(tif, 0);
        TIFFSetField (tif, TIFFTAG_EXIFIFD, exif_offset);
        TIFFRewriteDirectory(tif);
    }

    if(gpsInfo != nullptr)
    {
        uint64 gps_offset = 0;
        logWriter("makeGPSIFD");
        makeGPS_IFD(tif);
        logWriter("TIFFWriteCustomDirectory");
        TIFFWriteCustomDirectory(tif, &gps_offset);
        // set GPSIFD tag
        logWriter("TIFFSetDirectory");
        TIFFSetDirectory(tif, 0);
        logWriter("setgpsoffset");
        TIFFSetField (tif, TIFFTAG_GPSIFD, gps_offset);

    }

    TIFFClose(tif);


    logWriter("DONE");
}

