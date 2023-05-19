#ifndef COMMON_H
#define COMMON_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const unsigned int SCALE_RANGE = 40;

extern GLfloat cam_x;
extern GLfloat cam_z;
extern GLfloat cam_y;

extern GLfloat cam_pitch;
extern GLfloat cam_yaw;
extern int cam_scale;

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

#endif // SHADER_H
