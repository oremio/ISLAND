#include <iostream>
#include <sstream>
#include <fstream>

#include <SOIL.h>

#include "resource_manager.h"

// Instantiate static variables
std::map<std::string, Texture2D> ResourceManager::Textures;
std::map<std::string, Shader> ResourceManager::Shaders;

Shader ResourceManager::loadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, std::string name)
{
    Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    return Shaders[name];
}

Shader& ResourceManager::getShader(std::string name)
{
    return Shaders[name];
}

Texture2D ResourceManager::loadTexture(const char* file, bool alpha, std::string name, bool gammaCorrection)
{
    Textures[name] = loadTextureFromFile(file, alpha, gammaCorrection);
    return Textures[name];
}

Texture2D& ResourceManager::getTexture(std::string name)
{
    return Textures[name];
}
void ResourceManager::clear()
{
    // (properly) delete all shaders
    // shaders编译完之后就会自动删除
    for (auto iter : Shaders)
        glDeleteProgram(iter.second.ID);
    for (auto iter : Textures)
        glDeleteTextures(1, &iter.second.ID);
}

Shader ResourceManager::loadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    try {
        // open files
        std::ifstream vertexShaderFile(vShaderFile);
        std::ifstream fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        // close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // if geometry shader path is present, also load a geometry shader
        if (gShaderFile != nullptr)
        {
            std::ifstream geometryShaderFile(gShaderFile);
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::exception e)
    {
        std::cout << "ERROR::SHADER: Failed to read shader files" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    const char* gShaderCode = geometryCode.c_str();
    // 2. now create shader object from source code
    Shader shader;
    shader.compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr, std::string(vShaderFile).substr(0, std::string(vShaderFile).find_last_of('/')));
    return shader;
}

Texture2D ResourceManager::loadTextureFromFile(const char* file, bool alpha, bool gammaCorrection)
{
    // create texture object
    Texture2D texture;
    if (!gammaCorrection)
        texture.Internal_Format = GL_RGB;
    if (alpha)
    {
        texture.Internal_Format = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
        texture.Image_Format = GL_RGBA;
    }
    // Load image
    int width, height;
    unsigned char* image = SOIL_load_image(file, &width, &height, 0, texture.Image_Format == GL_RGBA ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB);
    // Now generate texture
    texture.generate(width, height, image);
    // And finally free image data
    SOIL_free_image_data(image);
    return texture;
}