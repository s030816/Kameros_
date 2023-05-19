#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
// Windows Header Files
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <ctype.h>
#include <vector>


#define EXIFPARSER __declspec(dllexport)


constexpr auto JPEG_SOI = 0xD8FF;
constexpr auto JPEG_APP0 = 0xE0FF;
constexpr auto JPEG_APP1 = 0xE1FF;
constexpr auto EXIF_II = 0x4949;
constexpr auto EXIF_MM = 0x4D4D;
constexpr auto GPS_IFD = 0x8825;

struct CAMERA_DATA
{
    double roll;
    double yaw;
    double pitch;
    double gps[3];
};

enum GPS_EXPORT_INDEX : uint8_t
{
    altitude = 0,
    latitude = 1,
    longtitude = 2
};

extern "C" EXIFPARSER CAMERA_DATA get_file_data(const wchar_t* path);

struct JpegSegment
{
    uint8_t* data{ NULL };
    uint16_t s_size{ 0 };
    ~JpegSegment()
    {
        if (data) free(data);
    }
};

struct GPS_DATA
{
    double latitude{ 0.0 };
    double longtitude{ 0.0 };
    double altitude{ 0.0 };
};

enum class GPS_TAG : uint16_t
{
    latituderef = 1,
    latitude = 2,
    longtituderef = 3,
    longtitude = 4,
    altituderef = 5,
    altitude = 6
};

struct IFD
{
    uint16_t tag_id{ 0 };
    uint16_t val_type{ 0 };
    uint32_t num_of_val{ 0 };
    uint32_t offset{ 0 };
    IFD() = default;
    IFD(uint16_t tid, uint16_t vt, uint32_t n, uint32_t o) :
        tag_id(tid), val_type(vt), num_of_val(n), offset(o) {}
};