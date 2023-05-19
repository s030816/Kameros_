#ifndef SHADER_H
#define SHADER_H

#include "common.h"

/**
 * @todo write docs
 */
class Shader
{
private:
    int success;
    char infoLog[512];
    unsigned int vertexShader;
    unsigned int fragmentShader;
    
    void compile_vshader();
    void compile_fshader();
    void link_shaders();
    
public:
    unsigned int shaderProgram;
    /**
     * Default constructor
     */
    Shader();

    /**
     * Destructor
     */
    ~Shader();
    
    
};

#endif // SHADER_H
