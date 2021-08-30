#include <iostream>

#include "stb_image.h"

#include "skybox.h"

Skybox::Skybox(Shader* shaderSkybox) : m_shaderSkybox(shaderSkybox) {
	GLfloat skyboxVertices[] = {
		// positions
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	setTexUnit();
}

Skybox::~Skybox() {
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
}

void Skybox::loadDiurnalSkybox(
	std::string right,
	std::string left,
	std::string top,
	std::string bottom,
	std::string back,
	std::string front
) {
	loadSkybox(
		right,
		left,
		top,
		bottom,
		back,
		front,
		true
	);
}

void Skybox::loadNocturnalSkybox(
	std::string right,
	std::string left,
	std::string top,
	std::string bottom,
	std::string back,
	std::string front
) {
	loadSkybox(
		right,
		left,
		top,
		bottom,
		back,
		front,
		false
	);
}

void Skybox::render(const glm::mat4& view, const glm::mat4& projection, const float& coeDiurnal) {
	glDepthFunc(GL_LEQUAL);

	m_shaderSkybox->use();

	m_shaderSkybox->setMatrix4("projection", projection);
	m_shaderSkybox->setMatrix4("view", glm::mat4(glm::mat3(view)));

	m_shaderSkybox->setFloat("coeDiurnal", coeDiurnal);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_diurnalTexID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_nocturnalTexID);

	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}

void Skybox::loadSkybox(
	std::string right,
	std::string left,
	std::string top,
	std::string bottom,
	std::string back,
	std::string front,
	bool const& isDiurnal
) {
	std::vector<std::string> paths;
	paths.push_back(right);
	paths.push_back(left);
	paths.push_back(top);
	paths.push_back(bottom);
	paths.push_back(back);
	paths.push_back(front);

	if (isDiurnal) {
		glGenTextures(1, &m_diurnalTexID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_diurnalTexID);
	}
	else {
		glGenTextures(1, &m_nocturnalTexID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_nocturnalTexID);
	}

	int width, height, nrChannels;
	for (unsigned int i = 0; i < paths.size(); i++) {
		unsigned char *data = stbi_load(paths[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << paths[i].c_str() << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Skybox::setTexUnit() {
	m_shaderSkybox->use();
	m_shaderSkybox->setInteger("skyboxDiurnal", 0);
	m_shaderSkybox->setInteger("skyboxNocturnal", 1);
}