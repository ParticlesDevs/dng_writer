//
// Created by troop on 03.03.2018.
//
#ifndef FREEDCAM_CUSTOMMATRIX_H
#define FREEDCAM_CUSTOMMATRIX_H

class CustomMatrix
{
public:
    float *colorMatrix1;
    float *colorMatrix2;
    float *neutralColorVector;
    float *fowardMatrix1;
    float *fowardMatrix2;
    float *calibrationMatrix1;
    float *calibrationMatrix2;
    double *noiseMatrix;

    CustomMatrix()
    {
        colorMatrix1 = nullptr;
        colorMatrix2 = nullptr;
        neutralColorVector = nullptr;
        fowardMatrix1 = nullptr;
        fowardMatrix2 = nullptr;
        calibrationMatrix1 = nullptr;
        calibrationMatrix2 = nullptr;
        noiseMatrix = nullptr;
    }

    void clear()
    {
        if(colorMatrix1 != nullptr)
        {
            delete[] colorMatrix1;
            colorMatrix1 = nullptr;
        }
        if(colorMatrix2 != nullptr)
        {
            delete[] colorMatrix2;
            colorMatrix2 = nullptr;
        }
        if(neutralColorVector != nullptr)
        {
            delete[] neutralColorVector;
            neutralColorVector = nullptr;
        }
        if(fowardMatrix1 != nullptr)
        {
            delete[] fowardMatrix1;
            fowardMatrix1 = nullptr;
        }
        if(fowardMatrix2 != nullptr)
        {
            delete[] fowardMatrix2;
            fowardMatrix2 = nullptr;
        }
        if(calibrationMatrix1 != nullptr)
        {
            delete[] calibrationMatrix1;
            calibrationMatrix1 = nullptr;
        }
        if(calibrationMatrix2 != nullptr)
        {
            delete[] calibrationMatrix2;
            calibrationMatrix2 = nullptr;
        }
        if(calibrationMatrix2 != nullptr)
        {
            delete[] calibrationMatrix2;
            calibrationMatrix2 = nullptr;
        }
        if(noiseMatrix != nullptr)
        {
            delete[] noiseMatrix;
            noiseMatrix = nullptr;
        }
    }
};

#endif //FREEDCAM_CUSTOMMATRIX_H
