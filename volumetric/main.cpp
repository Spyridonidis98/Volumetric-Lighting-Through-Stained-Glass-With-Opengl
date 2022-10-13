// Include C++ headers
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Shader loading utilities and other
#include <common/shader.h>
#include <common/util.h>
#include <common/camera.h>
#include <common/model.h>
#include <common/texture.h>

using namespace std;
using namespace glm;

// Function prototypes
void initialize();
void createContext();
void mainLoop();
void free();

#define W_WIDTH 1024 
#define W_HEIGHT 768 
#define TITLE "god rays"

// Global variables
GLFWwindow* window;
Camera* camera;

GLuint diffuseTexture[3], specularTexture[3], normalTexture[3]; 
GLuint suzanneVAO, sphereVAO, roomVAO, quadVAO, window1VAO, window2VAO, window3VAO, window1gVAO, window2gVAO, window3gVAO, window1sVAO, window2sVAO;

GLuint time[3], ambient[3], diffuse[3], specular[3], shininess[3], lightpow[3];
GLuint lightcolor[3];

vec3  Ks ;
vec3  Kd ;
vec3  Ka ;
float Ns  ;

std::vector<vec3> roomVertices, suzanneVertices, sphereVertices, quadVertices, window1Vertices, window2Vertices, window3Vertices, window1gVertices, window2gVertices, window3gVertices, window1sVertices, window2sVertices;
std::vector<vec2> floorUVs;

float light_pos_x= -3.2, light_pos_y= 14.300, light_pos_z = 26.90;
float lightcolor_r = 1, lightcolor_g = 1, lightcolor_b = 1;
float lightpower = 600;
int window_light1 = 1, window_light2 = 1;
float suzanne_posx = 0;
float horizontal = 1;
//light pos x=-3.199997 y=10.300009 z=17.900036
//x = 3.399998 y = 14.600017 z = 26.300032
class Shader
{
    public: GLuint ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath)
    {

        ID = loadShaders(
            vertexPath,
            fragmentPath);
    }

    void use()
    {
        glUseProgram(ID);
    }

    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec2(const std::string& name, float x, float y) const
    {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec3(const std::string& name, float x, float y, float z) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec4(const std::string& name, float x, float y, float z, float w)
    {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string& name, const glm::mat2& mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string& name, const glm::mat3& mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setTexture(const std::string& name, const GLuint& texture, const int textureId) {
        ;
        glActiveTexture(textureId);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(ID, name.c_str()), textureId-33984);
    }
};


void computeTangentBasis(
    // inputs
    std::vector<glm::vec3>& vertices,
    std::vector<glm::vec2>& uvs,
    std::vector<glm::vec3>& normals,
    // outputs
    std::vector<glm::vec3>& tangents,
    std::vector<glm::vec3>& bitangents
) 
{
    for (int i = 0; i < vertices.size(); i += 3) {

        // Shortcuts for vertices
        glm::vec3& v0 = vertices[i + 0];
        glm::vec3& v1 = vertices[i + 1];
        glm::vec3& v2 = vertices[i + 2];

        // Shortcuts for UVs
        glm::vec2& uv0 = uvs[i + 0];
        glm::vec2& uv1 = uvs[i + 1];
        glm::vec2& uv2 = uvs[i + 2];

        // Edges of the triangle : position delta
        glm::vec3 deltaPos1 = v1 - v0;
        glm::vec3 deltaPos2 = v2 - v0;

        // UV delta
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
        glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

        // Set the same tangent for all three vertices of the triangle.
        // They will be merged later, in vboindexer.cpp
        tangents.push_back(tangent);
        tangents.push_back(tangent);
        tangents.push_back(tangent);

        // Same thing for bitangents
        bitangents.push_back(bitangent);
        bitangents.push_back(bitangent);
        bitangents.push_back(bitangent);

    }
}

class OBJ2 {
    public: std::vector<vec3> objVertices, objNormals, objTangents, objBitangents;
    public: std::vector<vec2> objUVs;
    public: GLuint diffuseTexture, specularTexture, normalTexture;
    public: GLuint objVAO, objVerticiesVBO, objUVVBO, objNormalsVBO, objTangentsVBO, objBitangentsVBO;
    OBJ2(const char* name) {
        loadOBJWithTiny(name, objVertices, objUVs, objNormals);
        computeTangentBasis(objVertices, objUVs, objNormals, objTangents, objBitangents);
        
        glGenVertexArrays(1, &objVAO);
        glBindVertexArray(objVAO);

        // vertex VBO
        glGenBuffers(1, &objVerticiesVBO);
        glBindBuffer(GL_ARRAY_BUFFER, objVerticiesVBO);
        glBufferData(GL_ARRAY_BUFFER, objVertices.size() * sizeof(glm::vec3),
            &objVertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        // normals VBO
        glGenBuffers(1, &objNormalsVBO);
        glBindBuffer(GL_ARRAY_BUFFER, objNormalsVBO);
        glBufferData(GL_ARRAY_BUFFER, objNormals.size() * sizeof(glm::vec3),
            &objNormals[0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);

        // uvs VBO
        glGenBuffers(1, &objUVVBO);
        glBindBuffer(GL_ARRAY_BUFFER, objUVVBO);
        glBufferData(GL_ARRAY_BUFFER, objUVs.size() * sizeof(glm::vec2),
            &objUVs[0], GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);

        //tangens 
        glGenBuffers(1, &objTangentsVBO);
        glBindBuffer(GL_ARRAY_BUFFER, objTangentsVBO);
        glBufferData(GL_ARRAY_BUFFER, objTangents.size() * sizeof(glm::vec3),
            &objTangents[0], GL_STATIC_DRAW);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(3);

        //bitangens
        glGenBuffers(1, &objBitangentsVBO);
        glBindBuffer(GL_ARRAY_BUFFER, objBitangentsVBO);
        glBufferData(GL_ARRAY_BUFFER, objBitangents.size() * sizeof(glm::vec3),
            &objBitangents[0], GL_STATIC_DRAW);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(4);

    }

    void bind() {
        glBindVertexArray(objVAO);
    }

};


void createContext()
{

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // load texture maps
    diffuseTexture[0] = loadSOIL("textures/suzanne_diffuse.bmp");
    specularTexture[0] = loadSOIL("textures/suzanne_specular.bmp");
    normalTexture[0] = loadSOIL("textures/NormalMap2.png");
    diffuseTexture[1] = loadSOIL("textures/TexturesCom_Tiles_SmallHoned_1K_albedo.png");
    specularTexture[1] = loadSOIL("textures/TexturesCom_Tiles_SmallHoned_1K_ao.png");
    normalTexture[1] = loadSOIL("textures/NormalMap.png");
    diffuseTexture[2] = loadSOIL("textures/white.jpg");
    normalTexture[2] = loadSOIL("textures/nonormals.png");
    
    OBJ2 suzanne("objects/suzanne.obj");
    suzanneVAO = suzanne.objVAO;
    suzanneVertices =  suzanne.objVertices;
    
    OBJ2 room("objects/Room6.obj");
    roomVAO = room.objVAO;
    roomVertices = room.objVertices;
    //square
    OBJ2 sphere("objects/sphere.obj");
    sphereVAO = sphere.objVAO;
    sphereVertices = sphere.objVertices;
    
    OBJ2 quad("objects/quad.obj");
    quadVAO = quad.objVAO;
    quadVertices = quad.objVertices;

    OBJ2 window1("objects/window1n.obj");
    window1VAO = window1.objVAO;
    window1Vertices = window1.objVertices;

    OBJ2 window2("objects/window2n.obj");
    window2VAO = window2.objVAO;
    window2Vertices = window2.objVertices;

    OBJ2 window3("objects/window3n.obj");
    window3VAO = window3.objVAO;
    window3Vertices = window3.objVertices;

    OBJ2 window1g("objects/window1g.obj");
    window1gVAO = window1g.objVAO;
    window1gVertices = window1g.objVertices;

    OBJ2 window2g("objects/window2g.obj");
    window2gVAO = window2g.objVAO;
    window2gVertices = window2g.objVertices;

    OBJ2 window3g("objects/window3g.obj");
    window3gVAO = window3g.objVAO;
    window3gVertices = window3g.objVertices;

    OBJ2 window1s("objects/window1.obj");
    window1sVAO = window1s.objVAO;
    window1sVertices = window1s.objVertices;

    OBJ2 window2s("objects/window2.obj");
    window2sVAO = window2s.objVAO;
    window2sVertices = window2s.objVertices;
}

void free()
{
    //glDeleteBuffers(1, &triangleVerticesVBO);
    //glDeleteBuffers(1, &triangleNormalsVBO);
    

    //glDeleteBuffers(1, &objVerticiesVBO);
    //glDeleteBuffers(1, &objUVVBO);
    //glDeleteBuffers(1, &objNormalsVBO);
    //glDeleteVertexArrays(1, &objVAO);

    glDeleteVertexArrays(1, &suzanneVAO);
    glDeleteVertexArrays(1, &roomVAO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteVertexArrays(1, &window1VAO);

    glDeleteTextures(3, &diffuseTexture[0]);
    glDeleteTextures(3, &specularTexture[0]);
    glDeleteTextures(2, &normalTexture[0]);
    //glDeleteProgram(shaderProgram);
    //glDeleteProgram(shaderProgram2);
    glfwTerminate();
}

void renderScene(Shader& shader, int s, GLuint &FBtexture, GLuint& FBtexture2, GLuint& FBtexture3) {
    shader.use();

    vec3 lightPos = vec3(light_pos_x, light_pos_y, light_pos_z-0.1);
    mat4 modelMatrix[5] = { translate(mat4(), vec3(2+suzanne_posx, 3, 5.5)), translate(mat4(), vec3(0, 4, 0)), translate(mat4(), vec3(3, 0, 0)), translate(mat4(), vec3(0, 2, 0)), translate(mat4(), vec3(3, 2, 0)) };
    Ns = 38.4;
    camera->update();

    
    
    mat4 lightp = ortho(-25.0f, 25.0f, -25.0f, 25.0f, 14.0f , 42.0f + lightpower - 400);
    mat4 lightv = lookAt(lightPos, glm::vec3(0.0f , 0.0f , 0.0f), glm::vec3(0.0, 1.0, 0.0));
    mat4 projectionMatrix;
    mat4 viewMatrix;
    
    if(s == 0){
        
        projectionMatrix = lightp;
        viewMatrix = lightv;
        
        
        
        glBindVertexArray(suzanneVAO);
         for (int i = 0; i < 1; i++) {
             shader.setMat4("P", projectionMatrix);
             shader.setMat4("V", viewMatrix);
             shader.setMat4("M", modelMatrix[i]);
             glDrawArrays(GL_TRIANGLES, 0, suzanneVertices.size());
         }

         

         //floor
         glBindVertexArray(roomVAO);
         mat4 floorModelMatrix = translate(mat4(), vec3(2, -2, 0)) * rotate(mat4(), radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
         shader.setMat4("P", projectionMatrix);
         shader.setMat4("V", viewMatrix);
         shader.setMat4("M", floorModelMatrix);
         glDrawArrays(GL_TRIANGLES, 0, roomVertices.size());

         
         glBindVertexArray(window1VAO);
         mat4 window1Modelmatrix = translate(mat4(), vec3(2, -2, 0));
         shader.setMat4("P", projectionMatrix);
         shader.setMat4("V", viewMatrix);
         shader.setMat4("M", window1Modelmatrix);
         glDrawArrays(GL_TRIANGLES, 0, window1Vertices.size());
         
         if (window_light1 == 0) {
             glBindVertexArray(window1sVAO);
             mat4 window1sModelmatrix = translate(mat4(), vec3(2, -2, 0));
             shader.setMat4("P", projectionMatrix);
             shader.setMat4("V", viewMatrix);
             shader.setMat4("M", window1sModelmatrix);
             glDrawArrays(GL_TRIANGLES, 0, window1sVertices.size());
         }
         if (window_light2 == 0) {
             glBindVertexArray(window2sVAO);
             mat4 window2sModelmatrix = translate(mat4(), vec3(2, -2, 0));
             shader.setMat4("P", projectionMatrix);
             shader.setMat4("V", viewMatrix);
             shader.setMat4("M", window2sModelmatrix);
             glDrawArrays(GL_TRIANGLES, 0, window1sVertices.size());

         }
         


    }
  
    else if (s == 1) {
        projectionMatrix = lightp;
        viewMatrix = lightv;
        
        glBindVertexArray(suzanneVAO);
        for (int i = 0; i < 1; i++) {
            shader.setMat4("P", projectionMatrix);
            shader.setMat4("V", viewMatrix);
            shader.setMat4("M", modelMatrix[i]);
            glDrawArrays(GL_TRIANGLES, 0, suzanneVertices.size());
        }

        //floor
        glBindVertexArray(roomVAO);
        mat4 floorModelMatrix = translate(mat4(), vec3(2, -2, 0)) * rotate(mat4(), radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        shader.setMat4("P", projectionMatrix);
        shader.setMat4("V", viewMatrix);
        shader.setMat4("M", floorModelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, roomVertices.size());

        glBindVertexArray(window2VAO);
        mat4 window1Modelmatrix = translate(mat4(), vec3(2, -2, 0));
        shader.setMat4("P", projectionMatrix);
        shader.setMat4("V", viewMatrix);
        shader.setMat4("M", window1Modelmatrix);
        glDrawArrays(GL_TRIANGLES, 0, window2Vertices.size());

        if (window_light1 == 0) {
            glBindVertexArray(window1sVAO);
            mat4 window1sModelmatrix = translate(mat4(), vec3(2, -2, 0));
            shader.setMat4("P", projectionMatrix);
            shader.setMat4("V", viewMatrix);
            shader.setMat4("M", window1sModelmatrix);
            glDrawArrays(GL_TRIANGLES, 0, window1sVertices.size());
        }
        if (window_light2 == 0) {
            glBindVertexArray(window2sVAO);
            mat4 window2sModelmatrix = translate(mat4(), vec3(2, -2, 0));
            shader.setMat4("P", projectionMatrix);
            shader.setMat4("V", viewMatrix);
            shader.setMat4("M", window2sModelmatrix);
            glDrawArrays(GL_TRIANGLES, 0, window1sVertices.size());
        }
    }

    else if (s == 2) {
        projectionMatrix = lightp;
        viewMatrix = lightv;

        glBindVertexArray(suzanneVAO);
        for (int i = 0; i < 1; i++) {
            shader.setMat4("P", projectionMatrix);
            shader.setMat4("V", viewMatrix);
            shader.setMat4("M", modelMatrix[i]);
            glDrawArrays(GL_TRIANGLES, 0, suzanneVertices.size());
        }

        //floor
        glBindVertexArray(roomVAO);
        mat4 floorModelMatrix = translate(mat4(), vec3(2, -2, 0)) * rotate(mat4(), radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        shader.setMat4("P", projectionMatrix);
        shader.setMat4("V", viewMatrix);
        shader.setMat4("M", floorModelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, roomVertices.size());

        glBindVertexArray(window3VAO);
        mat4 window1Modelmatrix = translate(mat4(), vec3(2, -2, 0));
        shader.setMat4("P", projectionMatrix);
        shader.setMat4("V", viewMatrix);
        shader.setMat4("M", window1Modelmatrix);
        glDrawArrays(GL_TRIANGLES, 0, window3Vertices.size());

        if (window_light1 == 0) {
            glBindVertexArray(window1sVAO);
            mat4 window1sModelmatrix = translate(mat4(), vec3(2, -2, 0));
            shader.setMat4("P", projectionMatrix);
            shader.setMat4("V", viewMatrix);
            shader.setMat4("M", window1Modelmatrix);
            glDrawArrays(GL_TRIANGLES, 0, window1sVertices.size());
        }
        if (window_light2 == 0) {
            glBindVertexArray(window2sVAO);
            mat4 window2sModelmatrix = translate(mat4(), vec3(2, -2, 0));
            shader.setMat4("P", projectionMatrix);
            shader.setMat4("V", viewMatrix);
            shader.setMat4("M", window2sModelmatrix);
            glDrawArrays(GL_TRIANGLES, 0, window2sVertices.size());
        }
    }

    else if(s == 3){
         glBindVertexArray(suzanneVAO);
         projectionMatrix = camera->projectionMatrix;
         viewMatrix = camera->viewMatrix;
         vec3 lightPos_cameraspace = vec3(viewMatrix * vec4(lightPos, 1.0f));
         mat4 lightSpaceMatrix = lightp * lightv;//P*V
         vec3 camerapos = camera->position;
         for (int i = 0; i < 1; i++) {
             shader.setMat4("P", projectionMatrix);
             shader.setMat4("V", viewMatrix);
             shader.setMat4("M", modelMatrix[i]);
             shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
             shader.setTexture("diffuseColorSampler", diffuseTexture[0], GL_TEXTURE0);
             shader.setTexture("specularColorSampler", specularTexture[0], GL_TEXTURE1);
             shader.setTexture("NormalMap", normalTexture[0], GL_TEXTURE2);
             shader.setTexture("shadowMap", FBtexture, GL_TEXTURE3);
             shader.setTexture("shadowMap2", FBtexture2, GL_TEXTURE4);
             shader.setTexture("shadowMap3", FBtexture3, GL_TEXTURE5);
             shader.setVec3("light_position_cameraspace", lightPos_cameraspace.x, lightPos_cameraspace.y, lightPos_cameraspace.z);
             shader.setVec3("lightcolor", lightcolor_r, lightcolor_g, lightcolor_b);
             shader.setFloat("light_power", lightpower);
             shader.setFloat("Ns", Ns);
             shader.setVec3("camera_pos", camerapos);
             glDrawArrays(GL_TRIANGLES, 0, suzanneVertices.size());
         }

         

         //floor
         glBindVertexArray(roomVAO);
         mat4 floorModelMatrix = translate(mat4(), vec3(2, -2, 0)) * rotate(mat4(), radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
         shader.setMat4("P", projectionMatrix);
         shader.setMat4("V", viewMatrix);
         shader.setMat4("M", floorModelMatrix);
         shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
         shader.setTexture("diffuseColorSampler", diffuseTexture[2], GL_TEXTURE0);
         shader.setTexture("specularColorSampler", specularTexture[0], GL_TEXTURE1);
         shader.setTexture("NormalMap", normalTexture[2], GL_TEXTURE2);
         
         shader.setTexture("shadowMap", FBtexture, GL_TEXTURE3);
         shader.setTexture("shadowMap2", FBtexture2, GL_TEXTURE4);
         shader.setTexture("shadowMap3", FBtexture3, GL_TEXTURE5);
         shader.setVec3("light_position_cameraspace", lightPos_cameraspace.x, lightPos_cameraspace.y, lightPos_cameraspace.z);
         shader.setVec3("lightcolor", lightcolor_r, lightcolor_g, lightcolor_b);
         shader.setFloat("light_power", lightpower);
         shader.setFloat("Ns", Ns);
         shader.setVec3("camera_pos", camerapos);
         glDrawArrays(GL_TRIANGLES, 0, roomVertices.size());

        
         
    }
    
    else if (s == 4) {
        projectionMatrix = camera->projectionMatrix;
        viewMatrix = camera->viewMatrix;
        vec3 camerapos = camera->position;
        mat4 lightSpaceMatrix = lightp * lightv;//P*V

        //*
        glBindVertexArray(window1gVAO);
        mat4 window1Modelmatrix = translate(mat4(), vec3(2, -2, 0));
        shader.setFloat("is_sun", 0);
        shader.setMat4("P", projectionMatrix);
        shader.setMat4("V", viewMatrix);
        shader.setMat4("M", window1Modelmatrix);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shader.setTexture("shadowMap", FBtexture, GL_TEXTURE0);
        shader.setTexture("shadowMap2", FBtexture2, GL_TEXTURE1);
        shader.setTexture("shadowMap3", FBtexture3, GL_TEXTURE2);
        shader.setVec3("camera_pos", camerapos);
        glDrawArrays(GL_TRIANGLES, 0, window1gVertices.size());
        //*/
        glBindVertexArray(window2gVAO);
        mat4 window2Modelmatrix = translate(mat4(), vec3(2, -2, 0));
        shader.setFloat("is_sun", 1);
        shader.setMat4("P", projectionMatrix);
        shader.setMat4("V", viewMatrix);
        shader.setMat4("M", window2Modelmatrix);
        glDrawArrays(GL_TRIANGLES, 0, window1gVertices.size());
        
        glBindVertexArray(window3gVAO);
        mat4 window3Modelmatrix = translate(mat4(), vec3(2, -2, 0));
        shader.setFloat("is_sun", 2);
        shader.setMat4("P", projectionMatrix);
        shader.setMat4("V", viewMatrix);
        shader.setMat4("M", window3Modelmatrix);
        glDrawArrays(GL_TRIANGLES, 0, window3gVertices.size());

        
    }

    else if (s == 5) {
        glBindVertexArray(quadVAO);
        shader.setTexture("screenTexture", FBtexture, GL_TEXTURE0);
        shader.setFloat("horizontal", 1);
        glDrawArrays(GL_TRIANGLES, 0, quadVertices.size());
    }

    else if (s == 6) {
    glBindVertexArray(quadVAO);
    shader.setTexture("screenTexture", FBtexture, GL_TEXTURE0);
    shader.setFloat("horizontal", 0);
    glDrawArrays(GL_TRIANGLES, 0, quadVertices.size());
    }

    else {
        projectionMatrix = camera->projectionMatrix;
        viewMatrix = camera->viewMatrix;
        
        mat4 cubeModelMatrix = translate(mat4(), vec3(light_pos_x, light_pos_y, light_pos_z)) * scale(mat4(), vec3(0.2f, 0.2f, 0.2f));
        vec4 light_b = projectionMatrix * viewMatrix * vec4(vec3(light_pos_x, light_pos_y, light_pos_z),1.0f);//*vec4(lightPos, 1.0f);
        vec2 light_screenspace = vec2(light_b.x, light_b.y)/ light_b.w;
        light_screenspace = light_screenspace * 0.5f + 0.5f ;
        mat4 lightSpaceMatrix = lightp * lightv;//P*V
        //*
        mat4 IV = inverse(viewMatrix);

        glBindVertexArray(quadVAO);
        shader.setVec2("light_screenspace", light_screenspace.x, light_screenspace.y);
        shader.setTexture("screenTexture", FBtexture, GL_TEXTURE0);
        shader.setTexture("screenTexture2", FBtexture3, GL_TEXTURE1);
        shader.setTexture("shadowMap", FBtexture2, GL_TEXTURE2);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shader.setMat4("IV", IV);
        glDrawArrays(GL_TRIANGLES, 0, quadVertices.size());//*/
    }
}

void mainLoop()
{
    Shader quadShader("quad.vertexshader","quad.fragmentshader");
    Shader depthShader("depth.vertexshader", "depth.fragmentshader");
    Shader sceneShader("scene.vertexshader", "scene.fragmentshader");
    Shader windowShader("window.vertexshader", "window.fragmentshader");
    Shader bloomShader("bloom.vertexshader", "bloom.fragmentshader");
    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 4024, SHADOW_HEIGHT = 4024;
    //*
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    //glDrawBuffer(GL_NONE);
    //glReadBuffer(GL_NONE);
    //*/
    
    unsigned int depthMapFBO2;
    glGenFramebuffers(1, &depthMapFBO2);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO2);
    unsigned int depthMap2;
    glGenTextures(1, &depthMap2);
    glBindTexture(GL_TEXTURE_2D, depthMap2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap2, 0);
    //
    unsigned int depthMapFBO3;
    glGenFramebuffers(1, &depthMapFBO3);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO3);
    unsigned int depthMap3;
    glGenTextures(1, &depthMap3);
    glBindTexture(GL_TEXTURE_2D, depthMap3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap3, 0);
    
    //*/
    // framebuffer configuration
    // -------------------------
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, W_WIDTH, W_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    //
    unsigned int textureColorbuffer2;
    glGenTextures(1, &textureColorbuffer2);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, W_WIDTH, W_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textureColorbuffer2, 0);
    //
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    //
    unsigned int rbo2;
    glGenRenderbuffers(1, &rbo2);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, W_WIDTH, W_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo2); // now actually attach it
    
    
    unsigned int pingpongFBO[2];
    unsigned int pingpongBuffer[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffer);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, W_WIDTH, W_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0
        );
    }

  
    
    do
    {
               
        
        //shadow maps 
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        renderScene(depthShader,0, depthMap, depthMap, depthMap);//no input
        
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO2);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        renderScene(depthShader, 1, depthMap, depthMap, depthMap);//no input

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO3);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        renderScene(depthShader, 2, depthMap, depthMap, depthMap);//no input


        //render scene
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, W_WIDTH, W_HEIGHT);
        renderScene(sceneShader, 3, depthMap, depthMap2, depthMap3);//3 shadow maps
        renderScene(windowShader, 4, depthMap, depthMap2, depthMap3);//3 shadow maps 


        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, W_WIDTH, W_HEIGHT);
        renderScene(bloomShader, 5, textureColorbuffer2, depthMap, depthMap);// 1 input

        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[1]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, W_WIDTH, W_HEIGHT);
        renderScene(bloomShader, 6, pingpongBuffer[0], depthMap, depthMap);// 1 input
        for (int i = 0; i < 4; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, W_WIDTH, W_HEIGHT);
            renderScene(bloomShader, 5, pingpongBuffer[1], depthMap, depthMap);// 1 input

            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[1]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, W_WIDTH, W_HEIGHT);
            renderScene(bloomShader, 6, pingpongBuffer[0], depthMap, depthMap);// 1 input
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, W_WIDTH, W_HEIGHT);
        renderScene(quadShader, 7, textureColorbuffer, depthMap, pingpongBuffer[1]);
        //*/
        


        
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);
}



void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_T) {
        light_pos_z -= 0.1;
    }
    if (key == GLFW_KEY_G) {
        light_pos_z += 0.1;
    }
    
    if (key == GLFW_KEY_H) {
        light_pos_x += 0.1;
    }

    if (key == GLFW_KEY_F) {
        light_pos_x -= 0.1;
    }
    if (key == GLFW_KEY_R) {
        light_pos_y += 0.1;
    }
    if (key == GLFW_KEY_Y) {
        light_pos_y -= 0.1;
    }
    
    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        if (window_light1 == 0)window_light1 = 1;
        else if (window_light1 == 1)window_light1 = 0;
    }

    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        if (window_light2 == 0)window_light2 = 1;
        else if (window_light2 == 1)window_light2 = 0;
    }
    if (key == GLFW_KEY_C) {
        suzanne_posx += 0.1;
    }
    if (key == GLFW_KEY_V) {
        suzanne_posx -= 0.1;
    }
    
    if (key == GLFW_KEY_J) {
        lightpower += 0.5;
    }
    if (key == GLFW_KEY_M) {
        if(lightpower>0)lightpower -= 0.5;
    }
}

void initialize()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        throw runtime_error("Failed to initialize GLFW\n");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(W_WIDTH, W_HEIGHT, TITLE, NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        throw runtime_error(string(string("Failed to open GLFW window.") +
            " If you have an Intel GPU, they are not 3.3 compatible." +
            "Try the 2.1 version.\n"));
    }
    glfwMakeContextCurrent(window);

    // Start GLEW extension handler
    glewExperimental = GL_TRUE;

    // Initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        throw runtime_error("Failed to initialize GLEW\n");
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, W_WIDTH / 2, W_HEIGHT / 2);

    // Gray background color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glfwSetKeyCallback(window, pollKeyboard);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // enable textures
    glEnable(GL_TEXTURE_2D);

    // Log
    logGLParameters();

    // Create camera
    camera = new Camera(window);
}

int main(void)
{
    try
    {
        initialize();
        createContext();
        mainLoop();
        free();
    }
    catch (exception& ex)
    {
        cout << ex.what() << endl;
        getchar();
        free();
        return -1;
    }

    return 0;
}
