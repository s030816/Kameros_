#include "Renderer.h"

Renderer::Renderer()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    shader = new Shader();
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    make_vbo(Camera::vertices,0,Camera::vsize);
    make_vbo(Camera::vcolors,1, Camera::vcsize);
}

Renderer::~Renderer()
{
    for(auto& vbo : this->vbos)
    {
        GLuint i = vbo;
        glDeleteBuffers(1, &i);
    }
    glDeleteVertexArrays(1, &VAO);
    delete shader;
}

GLuint Renderer::make_vbo(GLfloat *data, int layout, size_t size)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    
    glVertexAttribPointer(layout, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(layout);
    this->vbos.emplace_back(buffer);
    return buffer;
}

void Renderer::render()
{
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader->shaderProgram);
        
        // transformaciju matricos
        glm::mat4 model         = glm::mat4(1.0f);
        glm::mat4 view          = glm::mat4(1.0f);
        glm::mat4 projection    = glm::mat4(1.0f);
        
        unsigned int modelLoc = glGetUniformLocation(shader->shaderProgram, "model");
        unsigned int viewLoc  = glGetUniformLocation(shader->shaderProgram, "view");
        unsigned int projecLoc  = glGetUniformLocation(shader->shaderProgram, "projection");
        
        // Piramidziu piesimas

        for (auto cam : cams)
        {
            glm::mat4 model = glm::mat4(1.0f);
            // Pirmine versija kuri suka piramides apie savo asi uzkomentuota
            /*
            model = glm::translate(model, cam.position);

            // pitch X, -yaw Z, -roll Y
            model = glm::rotate(model, glm::radians(cam.rotation[0]), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-cam.rotation[2]), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-cam.rotation[1]), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(cam_scale, cam_scale, cam_scale));
            */
            model = glm::translate(model, cam.position);
            glm::vec3 xNorm(1.0, 0.0f, 0.0);
            glm::vec3 yNorm(0.0, 1.0f, 0.0);
            glm::vec3 zNorm(0.0, 0.0f, 1.0);
 
            model = glm::rotate(model, glm::radians(cam.rotation[0]), xNorm); 
            yNorm = glm::rotate(yNorm, glm::radians(-cam.rotation[0]), xNorm);
            model = glm::rotate(model, glm::radians(-cam.rotation[1]), yNorm); 
            zNorm = glm::rotate(zNorm, glm::radians(cam.rotation[1]), yNorm);
            model = glm::rotate(model, glm::radians(cam.rotation[2]), zNorm); 
            
            model = glm::scale(model, glm::vec3(cam_scale, cam_scale, cam_scale));


            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(Camera::vsize/sizeof(GLfloat)/3 - 10));
            glDrawArrays(GL_LINES, static_cast<GLint>(Camera::vsize/sizeof(GLfloat)/3 - 10), 6);
        }
        // Tinklelio piesimas
        auto tinklelis = [&](float x, float y, int offset)
        {
            glm::mat4 pagrindas = glm::mat4(1.0f);
            pagrindas = glm::translate(pagrindas, glm::vec3(x, -2.0f, y));
            pagrindas = glm::scale(pagrindas, glm::vec3(SCALE_RANGE, SCALE_RANGE, SCALE_RANGE));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(pagrindas));
            glDrawArrays(GL_LINES, static_cast<GLint>(Camera::vsize / sizeof(GLfloat) / 3 - offset), 2);
        };
        for (float i = 0.0f; i < static_cast<float>(SCALE_RANGE); i += 4.0f)
        {
            tinklelis(i, 0.0f,2);
            tinklelis(0.0f, i , 4);
        }


        // Erdves piesimas
        model = glm::rotate(model, glm::radians(cam_pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(cam_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        view  = glm::translate(view, glm::vec3(cam_x, cam_y, cam_z)) * model;
        
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 150.0f);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projecLoc, 1, GL_FALSE, &projection[0][0]);

        glBindVertexArray(this->VAO);
}

void Renderer::add_cameras(std::vector<CAMERA_DATA>& cams_raw)
{
    for (const auto& i : cams_raw)
    {
        // y,altitude,x
        this->cams.emplace_back
        (
            Camera
            (
                glm::vec3
                (
                    i.gps[GPS_EXPORT_INDEX::longtitude],
                    i.gps[GPS_EXPORT_INDEX::altitude],
                    i.gps[GPS_EXPORT_INDEX::latitude]
                ),
                glm::vec3(i.pitch,i.yaw,i.roll)
            )
        );
    }
}

