#pragma once

#include <string>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>


class Shader
{
public:
	unsigned int programID;

	//Constructor
	Shader(std::string vertexPath, std::string fragmentPath);
	Shader(std::string shaderName);
	~Shader();

	//Set active
	void use();

	//Util functions
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setMatrix(const std::string &name, glm::mat4 matrix) const;
	void setVec3(const std::string &name, glm::vec3 vector) const;

	void setVec3(const std::string &name, float v1, float v2, float v3) const;

	unsigned int getUniformLocation(const std::string &name) const;
};

