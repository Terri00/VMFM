#pragma once
#include <GLFW\glfw3.h>

class util_keyHandler {
private:
	GLFWwindow * windowHandle;
public:
	bool getKeyDown(int key);

	util_keyHandler(GLFWwindow* window);
};