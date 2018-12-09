#pragma once

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "GLFWUtil.h"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

class Mesh {
	int elementCount;

	unsigned int dVBO, dVAO;

public:
	unsigned int VBO, VAO;

	std::vector<float> vertices;

	static std::vector<float> generateDebugMesh(std::vector<float> vrt)
	{
		std::vector<float> ot;
		for (int i = 0; i < (vrt.size() / 6) / 3; i++)
		{
			glm::vec3 v0 = glm::vec3(vrt[i * 18 + 0], vrt[i * 18 + 1], vrt[i * 18 + 2]);
			glm::vec3 n0 = glm::vec3(vrt[i * 18 + 3], vrt[i * 18 + 4], vrt[i * 18 + 5]);
			glm::vec3 v1 = glm::vec3(vrt[i * 18 + 6], vrt[i * 18 + 7], vrt[i * 18 + 8]);
			glm::vec3 v2 = glm::vec3(vrt[i * 18 + 12], vrt[i * 18 + 13], vrt[i * 18 + 14]);

			//n0 = glm::cross((v1 - v0), (v2 - v0));
			//n0 /= glm::length(n0);

			glm::vec3 c = (v0 + v1 + v2) / 3.0f;

			glm::vec3 d = c + (n0 * 0.2f);

			ot.push_back(c.x);
			ot.push_back(c.y);
			ot.push_back(c.z);
			ot.push_back(0);
			ot.push_back(0);
			ot.push_back(1);
			ot.push_back(c.x);
			ot.push_back(c.y);
			ot.push_back(c.z);
			ot.push_back(0);
			ot.push_back(0);
			ot.push_back(1);

			ot.push_back(d.x);
			ot.push_back(d.y);
			ot.push_back(d.z);
			ot.push_back(0);
			ot.push_back(0);
			ot.push_back(1);
		}

		return ot;
	}

	Mesh() {
		glGenVertexArrays(1, &this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

		//Debug information mesh
		glGenVertexArrays(1, &this->dVAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->dVBO);
	}

	Mesh(std::vector<float> vertices) {
		this->vertices = vertices;
		this->elementCount = vertices.size() / 6;

		// first, configure the cube's VAO (and VBO)
		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &this->VBO);

		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

		glBindVertexArray(this->VAO);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		//Normal vector
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);





		std::vector<float> dbg = Mesh::generateDebugMesh(vertices);

		//Debug information mesh
		glGenVertexArrays(1, &this->dVAO);
		glGenBuffers(1, &this->dVBO);

		glBindBuffer(GL_ARRAY_BUFFER, this->dVBO);
		glBufferData(GL_ARRAY_BUFFER, dbg.size() * sizeof(float), &dbg[0], GL_STATIC_DRAW);

		glBindVertexArray(this->dVAO);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		//Normal vector
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	~Mesh() {
		glDeleteVertexArrays(1, &this->VAO);
		glDeleteBuffers(1, &this->VBO);

		glDeleteVertexArrays(1, &this->dVAO);
		glDeleteBuffers(1, &this->dVBO);
	}

	void Draw() {
		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, this->elementCount);
	}

	void DrawDebug() {
		glBindVertexArray(this->dVAO);
		glDrawArrays(GL_TRIANGLES, 0, this->elementCount);
	}
};