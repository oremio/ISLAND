#include "fog.h"

void Fog::setShader(Shader& shader, std::string Name, GLboolean UseShader) {
    if (UseShader)
        shader.use();
    shader.setFloat((Name + ".Density").c_str(), m_Density);
    shader.setVector3f((Name + ".Color").c_str(), m_Color);
}