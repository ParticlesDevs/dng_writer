//
// Created by troop on 01.03.2018.
//

#ifndef FREEDCAM_EXIFINFO_H
#define FREEDCAM_EXIFINFO_H

class ExifInfo
{
public:
    int _iso, _flash;
    double _exposure;
    char* _imagedescription;
    char* _dateTime;
    char* _orientation;
    double _fnumber, _focallength;
    float _exposureIndex;

    void clear()
    {
        _imagedescription = nullptr;
        _dateTime = nullptr;
        _orientation = nullptr;
    };

    ExifInfo()
    {
        clear();
    };
};

#endif //FREEDCAM_EXIFINFO_H


