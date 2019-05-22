#include "Renderer.h"

Camera* Renderer::m_camera = new Camera();
Lighting* Renderer::m_lightings = new Lighting();

nanogui::Screen* Renderer::m_nanogui_screen = nullptr;

std::string model_path = "./objs/cyborg.obj";
float ivar = 90.0f, render_index;
float camera_angle;
bool Renderer::keys[1024], reload = true;
bool bool_dir_light_status = true, bool_point_light_status = false;
bool bool_point_rot_x=false, bool_point_rot_y=false, bool_point_rot_z=false;
float a,b, z_near=1.0f, z_far=10.0f;
GLfloat obj_shininess = 32.0f;

glm::vec3 object_center;
glm::vec3 point_light_position;

glm::mat4 mat;

nanogui::Color colval(0.5f, 0.5f, 0.5f, 1.f);
nanogui::Color col_dir_light_ambient(0.5f, 0.5f, 0.5f, 1.f);
nanogui::Color col_dir_light_diffuse(0.5f, 0.5f, 0.5f, 1.f);
nanogui::Color col_dir_light_specular(0.5f, 0.5f, 0.5f, 1.f);

nanogui::Color col_point_light_ambient(0.5f, 0.5f, 0.5f, 1.f);
nanogui::Color col_point_light_diffuse(0.5f, 0.5f, 0.5f, 1.f);
nanogui::Color col_point_light_specular(0.5f, 0.5f, 0.5f, 1.f);

enum test_enum {
	Item1=0,
	Item2=1,
	Item3=2
};

test_enum render_val = Item3;
test_enum cull_val = Item1;
test_enum shader_val = Item1;
test_enum depth_val = Item1;

Renderer::Renderer()
{
}

Renderer::~Renderer()
{	
}

void Renderer::nanogui_init(GLFWwindow* window)
{
	m_nanogui_screen = new nanogui::Screen();
	m_nanogui_screen->initialize(window, true);
	glViewport(0, 0, m_camera->width, m_camera->height);


	//window 1
	nanogui::FormHelper *gui_1 = new nanogui::FormHelper(m_nanogui_screen);
	nanogui::ref<nanogui::Window> nanoguiWindow_1 = gui_1->addWindow(Eigen::Vector2i(0, 0), "Nanogui control bar_1");

	gui_1->addGroup("POSITION");
	gui_1->addVariable("X", m_camera->position[0])->setSpinnable(true);
	gui_1->addVariable("Y", m_camera->position[1])->setSpinnable(true);
	gui_1->addVariable("Z", m_camera->position[2])->setSpinnable(true);

	gui_1->addGroup("ROTATE");
	gui_1->addVariable("Value", ivar)->setSpinnable(true);

	gui_1->addButton("Rotate Right +", []() {
		m_camera->rotate_y(-camera_angle);
	});

	gui_1->addButton("Rotate Right -", []() {
		m_camera->rotate_y(camera_angle);
	});

	gui_1->addButton("Rotate Up +", []() {
		m_camera->rotate_x(camera_angle);
	});

	gui_1->addButton("Rotate Up -", []() {
		m_camera->rotate_x(-camera_angle);
	});

	gui_1->addButton("Rotate Front +", []() {
		m_camera->rotate_z(camera_angle);
	});

	gui_1->addButton("Rotate Front -", []() {
		m_camera->rotate_z(-camera_angle);
	});

	gui_1->addGroup("CONFIGURATION");
	gui_1->addVariable("Z-Near", z_near)->setSpinnable(true);
	gui_1->addVariable("Z-Far", z_far)->setSpinnable(true);
	gui_1->addVariable("Render Type", render_val, true)->setItems({ "Point", "Line", "Triangle" });
	gui_1->addVariable("Culling", cull_val, true)->setItems({ "CCW", "CW" });
	gui_1->addVariable("Shading", shader_val, true)->setItems({ "Smooth", "Flat" });
	gui_1->addVariable("Depth", depth_val, true)->setItems({ "Less", "Always" });
   static auto string_widget = gui_1->addVariable("string", model_path);
	gui_1->addButton("Reload Model", []() {
		reload = true;
	});
	gui_1->addButton("Reset", [this]() 
	{
		m_camera->reset();
	});

	//window 2

	nanogui::FormHelper *gui_2 = new nanogui::FormHelper(m_nanogui_screen);
	nanogui::ref<nanogui::Window> nanoguiWindow_2 = gui_2->addWindow(Eigen::Vector2i(250, 0), "Nanogui control bar_2");

	gui_2->addGroup("Lighting");
	gui_2->addVariable("Object Color", colval);
	gui_2->addVariable("Object shininess", obj_shininess)->setSpinnable(true);
	gui_2->addVariable("Direction light status", bool_dir_light_status);
	gui_2->addVariable("Direction Light ambient Color", col_dir_light_ambient);
	gui_2->addVariable("Direction Light diffuse Color", col_dir_light_diffuse);
	gui_2->addVariable("Direction Light specular Color", col_dir_light_specular);


	gui_2->addVariable("Point Light status", bool_point_light_status);
	gui_2->addVariable("Point Light ambient Color", col_point_light_ambient);
	gui_2->addVariable("Point Light diffuse Color", col_point_light_diffuse);
	gui_2->addVariable("Point Light specular Color", col_point_light_specular);
	gui_2->addVariable("Point Light Rotate X", bool_point_rot_x);
	gui_2->addVariable("Point Light Rotate Y", bool_point_rot_y);
	gui_2->addVariable("Point Light Rotate Z", bool_point_rot_z);

	gui_2->addButton("Reset Point Light", []() {
		//reset point light code
		point_light_position = m_camera->position;
	});

	m_nanogui_screen->setVisible(true);
	m_nanogui_screen->performLayout();

	glfwSetCursorPosCallback(window,
		[](GLFWwindow *window, double x, double y) {
		m_nanogui_screen->cursorPosCallbackEvent(x, y);
	}
	);

	glfwSetMouseButtonCallback(window,
		[](GLFWwindow *, int button, int action, int modifiers) {
		m_nanogui_screen->mouseButtonCallbackEvent(button, action, modifiers);
	}
	);

	glfwSetKeyCallback(window,
		[](GLFWwindow *window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		if (key >= 0 && key < 1024)
		{
			if (action == GLFW_PRESS)
				keys[key] = true;
			else if (action == GLFW_RELEASE)
				keys[key] = false;
		}
	}
	);

	glfwSetCharCallback(window,
		[](GLFWwindow *, unsigned int codepoint) {
		m_nanogui_screen->charCallbackEvent(codepoint);
	}
	);

	glfwSetDropCallback(window,
		[](GLFWwindow *, int count, const char **filenames) {
		m_nanogui_screen->dropCallbackEvent(count, filenames);
	}
	);

	glfwSetScrollCallback(window,
		[](GLFWwindow *, double x, double y) {
		m_nanogui_screen->scrollCallbackEvent(x, y);
	}
	);

	glfwSetFramebufferSizeCallback(window,
		[](GLFWwindow *, int width, int height) {
		m_nanogui_screen->resizeCallbackEvent(width, height);
	}
	);
}

void Renderer::run()
{
	//initialize
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	m_camera->init();
	m_lightings->init();
	this->m_window = glfwCreateWindow(m_camera->width, m_camera->height, "Assignment 1", nullptr, nullptr);
	glfwMakeContextCurrent(this->m_window);

	glewExperimental = GL_TRUE;
	glewInit();

	nanogui_init(this->m_window);

	//display
	Shader shader_temp = Shader("./shader_smooth/basic.vert", "./shader_smooth/basic.frag");
	Shader shader_smooth = Shader("./shader_smooth/basic.vert", "./shader_smooth/basic.frag");
	Shader shader_flat = Shader("./shader_flat/basic.vert", "./shader_flat/basic.frag");

	point_light_position = m_camera->position;
	printf("%f,%f,%f", point_light_position[0], point_light_position[1], point_light_position[2]);
	// Main frame while loop
	while (!glfwWindowShouldClose(this->m_window))
	{


		glfwPollEvents();
		camera_angle = glm::radians(ivar);
		if (shader_val == 0) {
			shader_temp = shader_smooth;
		}
		else {
			shader_temp = shader_flat;
		}

		if (reload) {
			load_models(model_path);
			m_camera->reset();
			reload = false;
		}
		shader_temp.use();
		
		// Camera uniform values
		glUniform3f(glGetUniformLocation(shader_temp.program, "camera_pos"), m_camera->position.x, m_camera->position.y, m_camera->position.z);
		glUniformMatrix4fv(glGetUniformLocation(shader_temp.program, "projection"), 1, GL_FALSE, glm::value_ptr(m_camera->get_projection_mat(z_near, z_far)));
		
		m_lightings->direction_light.status = bool_dir_light_status;
		m_lightings->direction_light.ambient[0] = col_dir_light_ambient[0];
		m_lightings->direction_light.ambient[1] = col_dir_light_ambient[1];
		m_lightings->direction_light.ambient[2] = col_dir_light_ambient[2];

		m_lightings->direction_light.diffuse[0] = col_dir_light_diffuse[0];
		m_lightings->direction_light.diffuse[1] = col_dir_light_diffuse[1];
		m_lightings->direction_light.diffuse[2] = col_dir_light_diffuse[2];

		m_lightings->direction_light.specular[0] = col_dir_light_specular[0];
		m_lightings->direction_light.specular[1] = col_dir_light_specular[1];
		m_lightings->direction_light.specular[2] = col_dir_light_specular[2];

		m_lightings->point_light.status = bool_dir_light_status;
		m_lightings->point_light.ambient[0] = col_point_light_ambient[0];
		m_lightings->point_light.ambient[1] = col_point_light_ambient[1];
		m_lightings->point_light.ambient[2] = col_point_light_ambient[2];

		m_lightings->point_light.diffuse[0] = col_point_light_diffuse[0];
		m_lightings->point_light.diffuse[1] = col_point_light_diffuse[1];
		m_lightings->point_light.diffuse[2] = col_point_light_diffuse[2];

		m_lightings->point_light.specular[0] = col_point_light_specular[0];
		m_lightings->point_light.specular[1] = col_point_light_specular[1];
		m_lightings->point_light.specular[2] = col_point_light_specular[2];

		// Light uniform values
		if (bool_dir_light_status) {
			glUniform1i(glGetUniformLocation(shader_temp.program, "dir_light.status"), bool_dir_light_status);
			glUniform3f(glGetUniformLocation(shader_temp.program, "dir_light.direction"), m_lightings->direction_light.direction[0], m_lightings->direction_light.direction[1], m_lightings->direction_light.direction[2]);
			glUniform3f(glGetUniformLocation(shader_temp.program, "dir_light.ambient"), m_lightings->direction_light.ambient[0], m_lightings->direction_light.ambient[1], m_lightings->direction_light.ambient[2]);
			glUniform3f(glGetUniformLocation(shader_temp.program, "dir_light.diffuse"), m_lightings->direction_light.diffuse[0], m_lightings->direction_light.diffuse[1], m_lightings->direction_light.diffuse[2]);
			glUniform3f(glGetUniformLocation(shader_temp.program, "dir_light.specular"), m_lightings->direction_light.specular[0], m_lightings->direction_light.specular[1], m_lightings->direction_light.specular[2]);
		}
		
		 
		else {
			glUniform1i(glGetUniformLocation(shader_temp.program, "dir_light.status"), bool_dir_light_status);
			glUniform3f(glGetUniformLocation(shader_temp.program, "dir_light.direction"), 0.0f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(shader_temp.program, "dir_light.ambient"), 0.0f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(shader_temp.program, "dir_light.diffuse"), 0.0f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(shader_temp.program, "dir_light.specular"), 0.0f, 0.0f, 0.0f);

		}
		// Set current point light as camera's position
		m_lightings->point_light.position = point_light_position;		
		if (bool_point_light_status) {
			glUniform1i(glGetUniformLocation(shader_temp.program, "point_light.status"), bool_point_light_status);
			glUniform3f(glGetUniformLocation(shader_temp.program, "point_light.position"), point_light_position[0], point_light_position[1], point_light_position[2]);
			glUniform3f(glGetUniformLocation(shader_temp.program, "point_light.ambient"), m_lightings->point_light.ambient[0], m_lightings->point_light.ambient[1], m_lightings->point_light.ambient[2]);
			glUniform3f(glGetUniformLocation(shader_temp.program, "point_light.diffuse"), m_lightings->point_light.diffuse[0], m_lightings->point_light.diffuse[1], m_lightings->point_light.diffuse[2]);
			glUniform3f(glGetUniformLocation(shader_temp.program, "point_light.specular"), m_lightings->point_light.specular[0], m_lightings->point_light.specular[1], m_lightings->point_light.specular[2]);
			glUniform1f(glGetUniformLocation(shader_temp.program, "point_light.constant"), m_lightings->point_light.constant);
			glUniform1f(glGetUniformLocation(shader_temp.program, "point_light.linear"), m_lightings->point_light.linear);
			glUniform1f(glGetUniformLocation(shader_temp.program, "point_light.quadratic"), m_lightings->point_light.quadratic);
		}

		else {
			glUniform1i(glGetUniformLocation(shader_temp.program, "point_light.status"), bool_point_light_status);
			glUniform3f(glGetUniformLocation(shader_temp.program, "point_light.position"), point_light_position[0], point_light_position[1], point_light_position[2]);
			glUniform3f(glGetUniformLocation(shader_temp.program, "point_light.ambient"),0.0f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(shader_temp.program, "point_light.diffuse"), 0.0f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(shader_temp.program, "point_light.specular"),0.0f, 0.0f, 0.0f);
			glUniform1f(glGetUniformLocation(shader_temp.program, "point_light.constant"), m_lightings->point_light.constant);
			glUniform1f(glGetUniformLocation(shader_temp.program, "point_light.linear"), m_lightings->point_light.linear);
			glUniform1f(glGetUniformLocation(shader_temp.program, "point_light.quadratic"), m_lightings->point_light.quadratic);
		}

		//point light rotation x
		if (bool_point_rot_x)
		{
			point_rot_x();
		}

		if (bool_point_rot_y) {
			point_rot_y();
		}

		if (bool_point_rot_z) {
			point_rot_z();
		}

		// Set up some basic parameters
		glClearColor(background_color[0], background_color[1], background_color[2], background_color[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		if (depth_val == 0) {
			glDepthFunc(GL_LESS);
		}
		else if (depth_val == 1) {
			glDepthFunc(GL_ALWAYS);
		}
		
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		if (cull_val == 0)
			glFrontFace(GL_CW);

		if (cull_val == 1)
			glFrontFace(GL_CCW);

		for (size_t i = 0; i < obj_list.size(); i++)
		{

			if (obj_list[i].obj_name == "object")
			{
				glm::mat4 object_mat = glm::mat4();
				glUniformMatrix4fv(glGetUniformLocation(shader_temp.program, "model"), 1, GL_FALSE, glm::value_ptr(object_mat));
				
				glUniformMatrix4fv(glGetUniformLocation(shader_temp.program, "view"), 1, GL_FALSE, glm::value_ptr(m_camera->get_view_mat()));
				obj_list[i].obj_color = glm::vec4(colval[0], colval[1], colval[2], colval[3]);
				draw_object(shader_temp, obj_list[i]);
			}
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		m_nanogui_screen->drawWidgets();

		// Swap the screen buffers
		glfwSwapBuffers(this->m_window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return;
}

void Renderer::load_models(std::string modelpath)
{
		obj_list.clear();
		Object object(modelpath);
		object.obj_color = glm::vec4(colval[0], colval[1], colval[2], colval[3]);
		object.obj_name = "object";

		glGenVertexArrays(1, &object.vao);
		glGenBuffers(1, &object.vbo);

		glBindVertexArray(object.vao);

		glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
		glBufferData(GL_ARRAY_BUFFER, object.vao_vertices.size() * sizeof(Object::Vertex), &object.vao_vertices[0], GL_STATIC_DRAW);

		// Vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Object::Vertex), (GLvoid*)0);
		// Vertex Normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Object::Vertex), (GLvoid*)offsetof(Object::Vertex, Normal));
		// Vertex Texture Coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Object::Vertex), (GLvoid*)offsetof(Object::Vertex, TexCoords));

		glBindVertexArray(0);
		
		printf("OBJECT CENTRE: %f, %f, %f\n", object.obj_center[0], object.obj_center[1], object.obj_center[2]);
		object_center = object.obj_center;

		obj_list.push_back(object);
}

void Renderer::draw_object(Shader& shader, Object& object)
{
	glBindVertexArray(object.vao);
	object.shininess = obj_shininess;
	glUniform3f(glGetUniformLocation(shader.program, "m_object.object_color"), object.obj_color[0], object.obj_color[1], object.obj_color[2]);
	glUniform1f(glGetUniformLocation(shader.program, "m_object.shininess"), object.shininess);

	//points
	if (render_val == 0){
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glDrawArrays(GL_TRIANGLES, 0, object.vao_vertices.size());//
	}

	//lines
	if (render_val == 1) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, object.vao_vertices.size());
	}

	//triangles
	if (render_val == 2) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, object.vao_vertices.size());//triangles
	}
	glBindVertexArray(0);
}

glm::vec3 new_position;
float angle = glm::radians(0.4f);
void Renderer::point_rot_x() {

	//translating to the center of the object
	point_light_position = point_light_position - object_center;

	new_position[0] = point_light_position[0];
	new_position[1] = point_light_position[1] * cos(angle) - point_light_position[2] * sin(angle);
	new_position[2] = point_light_position[1] * sin(angle) + point_light_position[2] * cos(angle);
	
	point_light_position = new_position;

	point_light_position = point_light_position + object_center;
}

void Renderer::point_rot_y() {

	point_light_position = point_light_position - object_center;

	new_position[0] = point_light_position[0]*cos(angle) - point_light_position[2]*sin(angle);
	new_position[1] = point_light_position[1];
	new_position[2] = point_light_position[0] * sin(angle) + point_light_position[2] * cos(angle);

	point_light_position = new_position;

	point_light_position = point_light_position + object_center;
}

void Renderer::point_rot_z() {

	point_light_position = point_light_position - object_center;
	
	new_position[0] = point_light_position[0] * cos(angle) - point_light_position[1] * sin(angle);
	new_position[1] = point_light_position[0] * sin(angle) + point_light_position[1]*cos(angle);
	new_position[2] = point_light_position[2];
	point_light_position = new_position;
	

	point_light_position = point_light_position + object_center;
}


int main() {
	Renderer m_renderer;
	m_renderer.run();
	return EXIT_SUCCESS;
}