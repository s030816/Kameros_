#ifndef RENDERER_H
#define RENDERER_H

#include "common.h"
#include "Shader.h"
#include "Camera.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"

/**
 * @todo write docs
 */
class Renderer
{
private:
    std::vector<GLuint> vbos;
    std::vector<Camera> cams;
    Shader *shader{nullptr};
    GLuint VAO;
    
public:
    /**
     * Default constructor
     */
    Renderer();

    /**
     * Destructor
     */
    ~Renderer();
    
    GLuint make_vbo(GLfloat *data, int layout, size_t size);
    void render();
    void add_cameras(std::vector<CAMERA_DATA>& cams_raw);
};

#endif // RENDERER_H
