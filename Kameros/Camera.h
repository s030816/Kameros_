#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"

/**
 * @todo write docs
 */
class Camera
{
public:
    static GLfloat vertices[];
    static GLfloat vcolors[];
    static size_t vsize;
    static size_t vcsize;
    glm::vec3 position;
    glm::vec3 rotation;
    
    Camera(glm::vec3 pos, glm::vec3 rot) : position(pos), rotation(rot){}
};

#endif // CAMERA_H
