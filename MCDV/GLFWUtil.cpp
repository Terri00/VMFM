#include "GLFWUtil.h"


util_keyHandler::util_keyHandler(GLFWwindow* window)
{
	this->windowHandle = window;
}

bool util_keyHandler::getKeyDown(int key)
{
	if (glfwGetKey(this->windowHandle, key) == GLFW_PRESS)
		return true;
}