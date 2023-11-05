//
// Created by troop on 03.03.2018.
//

#include "CustomMatrix.h"
#include "JniUtils.h"
#include <stdlib.h>
class DNGWriter {
    CustomMatrix *matrix;
public:
    void CustomMatrixInit() {
        matrix = new CustomMatrix();
    }
   void CustomMatrixClear(JNIEnv * env , jobject thiz, jobject javaHandler)
    {
        CustomMatrix *matrix = (CustomMatrix *) env->GetDirectBufferAddress(javaHandler);
        matrix -> clear();
        delete matrix;
    }
};



    JNIEXPORT void JNICALL Java_freed_dng_CustomMatrix_clear(JNIEnv * env , jobject thiz, jobject javaHandler)
    {
        CustomMatrix *matrix = (CustomMatrix *) env->GetDirectBufferAddress(javaHandler);
        matrix -> clear();
        delete matrix;
    }

    JNIEXPORT void JNICALL Java_freed_dng_CustomMatrix_setMatrixes(JNIEnv * env , jobject thiz, jobject javaHandler, jfloatArray colorMatrix1,
        jfloatArray colorMatrix2,
        jfloatArray neutralColor,
        jfloatArray fowardMatrix1,
        jfloatArray fowardMatrix2,
        jfloatArray reductionMatrix1,
        jfloatArray reductionMatrix2,
        jdoubleArray noiseMatrix)
    {
        CustomMatrix *matrix = (CustomMatrix *) env->GetDirectBufferAddress(javaHandler);
        matrix->colorMatrix1 = copyfloatArray(env,colorMatrix1);
        //writer->colorMatrix1 = env->GetFloatArrayElements(colorMatrix1, 0);
        matrix->colorMatrix2 = copyfloatArray(env,colorMatrix2);
        matrix->neutralColorMatrix = copyfloatArray(env,neutralColor);
        if(fowardMatrix1 != NULL)
            matrix->fowardMatrix1 = copyfloatArray(env,fowardMatrix1);
        if(fowardMatrix2 != NULL)
            matrix->fowardMatrix2 =copyfloatArray(env,fowardMatrix2);
        if(reductionMatrix1 != NULL)
            matrix->reductionMatrix1 =copyfloatArray(env,reductionMatrix1);
        if(reductionMatrix2 != NULL)
            matrix->reductionMatrix2 =copyfloatArray(env,reductionMatrix2);
        if(noiseMatrix != NULL){
            int size = env->GetArrayLength((jarray)noiseMatrix);
            matrix->noiseMatrix = new double[size];
            jdouble * mat =env->GetDoubleArrayElements(noiseMatrix, 0);
            memcpy(matrix->noiseMatrix,mat, size * sizeof(jdouble));
            env->ReleaseDoubleArrayElements(noiseMatrix,mat,JNI_ABORT);
        }
    }