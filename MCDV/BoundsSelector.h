#pragma once
#include <iostream>
#include <fstream>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <sstream>
#include <vector>
#include <map>
#include <regex>

#include "Util.h"
#include "ValveKeyValue.h"
#include "Plane.h"
#include "Mesh.h"
#include "ConvexPolytopes.h"

#include <chrono>


class BoundsSelector
{
public:
	glm::vec2 NW, SE;
	glm::vec2 A, B;

	Mesh* SelectionOverlay;

	BoundsSelector(glm::vec2 A, glm::vec2 B) {
		this->A = A;
		this->B = B;
		this->UpdatePoints();
		Mesh* nm = new Mesh;
		this->SelectionOverlay = nm;
		this->UpdateMeshes();
	}

	void UpdateMeshes()
	{
		// Generate new selection mesh overlay quad thingie
		std::vector<float> newVerts = {
			NW.x, 0, NW.y, 0, 1, 0,
			SE.x, 0, SE.y, 0, 1, 0,
			NW.x, 0, SE.y, 0, 1, 0,

			NW.x, 0, NW.y, 0, 1, 0,
			SE.x, 0, NW.y, 0, 1, 0,
			SE.x, 0, SE.y, 0, 1, 0
		};

		delete this->SelectionOverlay;
		this->SelectionOverlay = new Mesh(newVerts);
	}

	void SetA(glm::vec2 A)
	{
		this->A = A;
		this->UpdatePoints();
	}

	void SetB(glm::vec2 B)
	{
		this->B = B;
		this->UpdatePoints();
	}

	void UpdatePoints()
	{
		//Fix weird bounds
		this->NW = glm::vec2(glm::min(A.x, B.x), glm::max(A.y, B.y));
		this->SE = glm::vec2(glm::max(A.x, B.x), glm::min(A.y, B.y));
	}

	~BoundsSelector()
	{
		delete this->SelectionOverlay;
		this->SelectionOverlay = NULL;
	}
};