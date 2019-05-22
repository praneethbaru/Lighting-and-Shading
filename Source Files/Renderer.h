/**********************************************************************************
Renderer.h is the header file that includes the declaration of members and methods
************************************************************************************/
//To include the file only once during compilation
#pragma once

#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <nanogui/nanogui.h>

#include "Shader.h"
#include "Camera.h"
#include "Object.h"
#include "Lighting.h"

class Renderer
{
public:
	GLFWwindow* m_window;
	static Camera* m_camera;
	static Lighting* m_lightings;
	static nanogui::Screen* m_nanogui_screen;
	std::vector<Object> obj_list;
	glm::vec4 background_color = glm::vec4(0.1,0.1,0.1,0.1);
	bool is_scean_reset = true;
	std::string model_name;
	GLfloat delta_time = 0.0;
	GLfloat last_frame = 0.0;
	static bool keys[1024];

public:
	//Constructor
	Renderer();
	//Destructor
	~Renderer();

	void nanogui_init(GLFWwindow* window);
	void run();
	void load_models(std::string modelpath);
	void draw_object(Shader& shader, Object& object);

	void point_rot_x();
	void point_rot_y();
	void point_rot_z();

};

