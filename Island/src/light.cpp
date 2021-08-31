#include "Light.h"

void Light::setShader(Shader& shader, std::string Name, GLboolean UseShader) {
	if (UseShader)
		shader.use();
	shader.setVector3f((Name + ".direction").c_str(), m_direction);
	shader.setVector3f((Name + ".lightColor").c_str(), m_lightColor);
	shader.setFloat((Name + ".ambientStrength").c_str(), m_ambientStrength);
	shader.setVector3f((Name + ".ambientColor").c_str(), m_ambientColor);
}