#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <regex>

#include "GLFWUtil.h"

#include "Shader.h"
#include "Mesh.h"

#include "GameObject.h"

#include "ValveKeyValue.h"
#include "VMF.h"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "Plane.h"
#include "ConvexPolytopes.h"

#include "BoundsSelector.h"
#include "TextFont.h"

#define SCALE_SOURCE_GL 0.01f

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, util_keyHandler keys);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void updateSelectionCoords(glm::vec2 coords, int pointid);
glm::vec2 projectClickToWorld(glm::vec2 mousecoords);

int window_width = 1080;
int window_height = 720;

int editing_side = 0;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool isClicking = false;

bool rmb_down = false;
bool lmb_down = false;

double mousex;
double mousey;
double _mousex;
double _mousey;

float M_ORTHO_SIZE = SCALE_SOURCE_GL * 4;
int SV_PERSPECTIVE = 0;
glm::vec3 cam_pos(0.0f, SCALE_SOURCE_GL * 2048, 0.0f);

GameObject viewPickerObject;

BoundsSelector* bs_test;
vmf::vmf* _map_a;
vmf::vmf* _map_b;


int main(int argc, char* argv[]) {

	std::vector<std::string> maps = { "de_arpegio_bu07re01.vmf", "de_nausea15r4.vmf" };
	//std::vector<std::string> maps = { };

	for (int i = 1; i < argc; ++i) {
		char* _arg = argv[i];
		std::string arg = _arg;

		if (split(arg, '.').back() == "vmf") {
			maps.push_back(arg);
		}
	}

	if (maps.size() < 2) {
		std::cout << "Drop 2 vmfs onto the exe" << std::endl; system("PAUSE"); return 0;
	}

	std::vector<float> m_cube_verts = {
		// positions          // normals          rds
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	};

#pragma region Initialisation

	std::cout << "VMF merge test run 1.0.0-r-a" << std::endl;

	//Initialize OpenGL
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //We are using version 3.3 of openGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	//Creating the window
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "HarryEngine", NULL, NULL);

	//Check if window open
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate(); return -1;
	}
	glfwMakeContextCurrent(window);

	//Settingn up glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl; return -1;
	}

	//Viewport
	glViewport(0, 0, window_width, window_height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Mouse
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

#pragma endregion

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	//The unlit shader thing
	Shader shader_unlit("additivecolor.vs", "additivecolor.fs");
	Shader shader_lit("lit.vs", "lit.fs");


	//Mesh handling -----------------------------------------------------------------------------

	Mesh cube(m_cube_verts);
	viewPickerObject = GameObject(&cube, glm::vec3(0, 0, 4));


	util_keyHandler keys(window);

	vmf::vmf* map_a;

	try { map_a = new vmf::vmf(maps[0]); }
	catch (...) { return 0; }

	_map_a = map_a;
	map_a->ComputeGLMeshes();

	vmf::vmf* map_b;
	try { map_b = new vmf::vmf(maps[1]); }
	catch (...) { return 0; }

	_map_b = map_b;
	map_b->ComputeGLMeshes();


	bs_test = new BoundsSelector(glm::vec2(0, SCALE_SOURCE_GL * 512.0f), glm::vec2(SCALE_SOURCE_GL * 512.0f, 0));

	map_a->MarkSelected(bs_test);

	TextFont::init(); //Setup textfonts

	TextFont text("Hello World!");
	text.size = glm::vec2(1.0f / window_width, 1.0f / window_height) * 2.0f;
	text.alpha = 1.0f;
	text.color = glm::vec3(0.75f, 0.75f, 0.75f);
	text.screenPosition = glm::vec2(0, (1.0f / window_height) * 15.0f);

	text.SetText("\
Middle Mouse: Pan\n\
Scroll:       Zoom\n\
Left Click:   Set Point A\n\
Right Click:  Set Point B\n\
Tab:          Invert Selection\n\
Enter:        Save merged");

	//The main loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Input
		processInput(window, keys);

		//Rendering commands
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
		glDepthMask(0);

		glm::mat4 model = glm::mat4();

		shader_unlit.use();
		shader_unlit.setFloat("alpha", 0.1f);
		shader_unlit.setMatrix("projection", glm::ortho(-M_ORTHO_SIZE * (float)window_width, M_ORTHO_SIZE * (float)window_width, (float)window_height * -M_ORTHO_SIZE, (float)window_height * M_ORTHO_SIZE, 0.01f, 100.0f));

		glm::mat4 viewMatrix = glm::lookAt(cam_pos, cam_pos + glm::vec3(0, -1.0f, 0), glm::vec3(0, 0, 1));
		shader_unlit.setMatrix("view", viewMatrix);

		model = glm::mat4();
		shader_unlit.setMatrix("model", model);

#pragma region map_a
		shader_unlit.setVec3("color", 0.1f, 0.1f, 0.1f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glClear(GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < map_a->solids.size(); i++) {
			if (map_a->solids[i].faces[0].texture == "TOOLS/TOOLSSKIP") continue;
			if (map_a->solids[i].faces[0].texture == "TOOLS/TOOLSCLIP") continue;
			if (map_a->solids[i].faces[0].texture == "TOOLS/TOOLSSKYBOX") continue;
			if (map_a->solids[i].hidden) continue;

			map_a->solids[i].mesh->Draw();
		}

		//Draw lines
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		for (int i = 0; i < map_a->solids.size(); i++) {
			if (map_a->solids[i].faces[0].texture == "TOOLS/TOOLSSKIP") continue;
			if (map_a->solids[i].faces[0].texture == "TOOLS/TOOLSCLIP") continue;
			if (map_a->solids[i].faces[0].texture == "TOOLS/TOOLSSKYBOX") continue;
			if (map_a->solids[i].hidden) continue;

			map_a->solids[i].mesh->Draw();
		}

		//Draw entity origins
		for (int i = 0; i < map_a->entities.size(); i++) {
			if (map_a->entities[i].hidden) continue;

			model = glm::mat4();
			model = glm::translate(model, map_a->entities[i].origin);
			model = glm::scale(model, glm::vec3(0.25f));
			shader_unlit.setMatrix("model", model);

			cube.Draw();
		}
#pragma endregion

#pragma region map_b
		model = glm::mat4();
		shader_unlit.setMatrix("model", model);

		shader_unlit.setVec3("color", 0.25f, 0.0f, 0.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glClear(GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < map_b->solids.size(); i++) {
			if (map_b->solids[i].faces[0].texture == "TOOLS/TOOLSSKIP") continue;
			if (map_b->solids[i].faces[0].texture == "TOOLS/TOOLSCLIP") continue;
			if (map_b->solids[i].faces[0].texture == "TOOLS/TOOLSSKYBOX") continue;
			if (map_b->solids[i].hidden) continue;

			map_b->solids[i].mesh->Draw();
		}

		//Draw lines
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		for (int i = 0; i < map_b->solids.size(); i++) {
			if (map_b->solids[i].faces[0].texture == "TOOLS/TOOLSSKIP") continue;
			if (map_b->solids[i].faces[0].texture == "TOOLS/TOOLSCLIP") continue;
			if (map_b->solids[i].faces[0].texture == "TOOLS/TOOLSSKYBOX") continue;
			if (map_b->solids[i].hidden) continue;


			map_b->solids[i].mesh->Draw();
		}

		//Draw entity origins
		for (int i = 0; i < map_b->entities.size(); i++) {
			if (map_b->entities[i].hidden) continue;

			model = glm::mat4();
			model = glm::translate(model, map_b->entities[i].origin);
			model = glm::scale(model, glm::vec3(0.25f));
			shader_unlit.setMatrix("model", model);

			cube.Draw();
		}
#pragma endregion

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader_unlit.setFloat("alpha", 1.0f);
		shader_unlit.setVec3("color", 1.0f, 0.0f, 0.0f);

		model = glm::mat4();
		shader_unlit.setMatrix("model", model);

		bs_test->SelectionOverlay->Draw();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		shader_unlit.setVec3("color", 0.0f, 1.0f, 0.0f);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(bs_test->A.x, 0.0f, bs_test->A.y));
		model = glm::scale(model, glm::vec3(0.5f));
		shader_unlit.setMatrix("model", model);
		cube.Draw();

		shader_unlit.setVec3("color", 0.0f, 0.0f, 1.0f);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(bs_test->B.x, 0.0f, bs_test->B.y));
		model = glm::scale(model, glm::vec3(0.5f));
		shader_unlit.setMatrix("model", model);
		cube.Draw();

		// UI Elements
		text.Draw();

		//Check and call events, swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//Exit safely
	glfwTerminate();
	return 0;
}

//Automatically readjust to the new size we just received
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	window_width = width;
	window_height = height;
}

bool commit_lock = false;
bool selection_invert = false;
bool flip_lock = false;

void processInput(GLFWwindow* window, util_keyHandler keys)
{
	//if (keys.getKeyDown(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
		if (!commit_lock) {
			_map_a->Commit_Merge(_map_b);
			commit_lock = true;

			char filename[MAX_PATH];
		}
	}

	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
		if (!flip_lock) {
			flip_lock = true;

			selection_invert = !selection_invert;
			updateSelectionCoords(glm::vec2(0), -1); //Force update
		}
	}
	else flip_lock = false;
}

glm::vec2 projectClickToWorld(glm::vec2 mousecoords)
{
	mousecoords -= glm::vec2(window_width / 2.0f, window_height / 2.0f);
	mousecoords *= (M_ORTHO_SIZE * 2);
	mousecoords *= -1.0f;
	mousecoords += glm::vec2(cam_pos.x, cam_pos.z);

	return mousecoords;
}

void updateSelectionCoords(glm::vec2 coords, int pointid)
{
	if (pointid == 0)
		bs_test->SetA(coords);
	else if(pointid == 1)
		bs_test->SetB(coords);

	bs_test->UpdateMeshes();
	_map_a->MarkSelected(bs_test, selection_invert);
	_map_b->MarkSelected(bs_test, !selection_invert);
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	_mousex = mousex; _mousey = mousey;
	mousex = xpos; mousey = ypos;

	if (isClicking) {
		cam_pos += glm::vec3(1.0, 0.0, 0.0) * (float)(mousex - _mousex) * (M_ORTHO_SIZE * 2);
		cam_pos += glm::vec3(0.0, 0.0, 1.0) * (float)(mousey - _mousey) * (M_ORTHO_SIZE * 2);

		glfwSetWindowTitle(window, (std::to_string((int)(-cam_pos.x * (1.0f / M_ORTHO_SIZE))) + " " + std::to_string((int)(cam_pos.z * (1.0f / M_ORTHO_SIZE)))).c_str());
	}

	if (rmb_down) updateSelectionCoords(projectClickToWorld(glm::vec2(mousex, mousey)), 0);
	if (lmb_down) updateSelectionCoords(projectClickToWorld(glm::vec2(mousex, mousey)), 1);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	M_ORTHO_SIZE += (float)-yoffset * 0.0025f;
	if (M_ORTHO_SIZE < 0.001f) M_ORTHO_SIZE = 0.001f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_HAND_CURSOR);
		isClicking = true;
	}
	else
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		isClicking = false;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		updateSelectionCoords(projectClickToWorld(glm::vec2(mousex, mousey)), 0); rmb_down = true;
	}
	else {
		rmb_down = false;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		updateSelectionCoords(projectClickToWorld(glm::vec2(mousex, mousey)), 1); lmb_down = true;
	}
	else {
		lmb_down = false;
	}
}