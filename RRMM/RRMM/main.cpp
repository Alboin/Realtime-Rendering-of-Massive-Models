#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderLoader.h"
#include "plyloader.h"
#include "PLYDrawer.h"
#include "Cube.h"

using namespace glm;
using namespace std;

// Global variables
mat4 view;
GLFWwindow* window;
bool leftMousePressed = false;
vec3 position = vec3(-2.0f, 0.2f, -2.0f);
bool exitProgram = false;
int windowWidth = 1200;
int windowHeight = 800;
float horizontalAngle = -90, verticalAngle = 0;
int wireframe = 0;
double mouseX, mouseY;
int modelsToRender = 1;
int octreeLevels = 100;
int LOD = 0;
int numberOfLODs = 4;

//Some global variables for the fps-counter
double t0 = 0.0;
int frames = 0;
char titlestring[200];

// Function declaration
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void updateView();


int main()
{

	#pragma region initiation


	//Starting position of camera
	view = lookAt(position, vec3(0, 0, 0), vec3(0, 1, 0));

	//Initiate glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	//Try to create a window
	window = glfwCreateWindow(windowWidth, windowHeight, "RRMM", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	//Initiate glew
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	//Tell OpenGL the size of the viewport
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);
	glPointSize(5);

	// Create and compile our GLSL program from the shaders
	GLuint shaderProgramID = LoadShaders("vertexshader.glsl", "fragmentshader.glsl");

	//Register external intpu in GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	#pragma endregion

	#pragma region Read museum layout-file.
	vector<vector<uint> > museumLayout;
	ifstream inputFile;
	inputFile.open("museum.txt");

	if (inputFile.is_open())
	{
		string line;
		for (int i = 0; getline(inputFile, line); i++)
		{
			stringstream ss;
			ss << line;
			vector<uint> temp;
			museumLayout.push_back(temp);
			int number;
			while (ss >> number)
				museumLayout[i].push_back(number);
		}
		inputFile.close();
	}
	#pragma endregion

	/*for (int i = 0; i < museumLayout.size(); i++)
	{
		cout << "row: " << i << ":";
		for (int j = 0; j < museumLayout[i].size(); j++)
		{
			cout << museumLayout[i][j] << ", ";
		}
		cout << endl;
	}

	int dummy;
	cin >> dummy;*/

	//Load ply-models
	PLYDrawer *model1;
	model1 = new PLYDrawer(PLYModel("models/bunny.ply", false, false), octreeLevels, numberOfLODs);
	PLYDrawer *model2;
	model2 = new PLYDrawer(PLYModel("models/Armadillo.ply", false, false), octreeLevels, numberOfLODs);
	//PLYDrawer *model3;
	//model3 = new PLYDrawer(PLYModel("models/dragon.ply", false, false), octreeLevels, numberOfLODs);

	Cube cube = Cube();

	vec3 lightPos(0.0f, 15.0f, 10.0f);


	while (!glfwWindowShouldClose(window) && !exitProgram)
	{

		//Checks if any events are triggered (like keyboard or mouse events)
		glfwPollEvents();
		//Update mouse position
		glfwGetCursorPos(window, &mouseX, &mouseY);
		updateView();

		//Set the lightPos to be the same as the camera position.
		lightPos = (inverse(view))[3];

		//Rendering commands here
		glClearColor(0.2, 0.2, 0.2, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (wireframe == 0)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else if (wireframe == 1)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else if (wireframe == 2)
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

		//Render the model several times.
		/*for (int i = 0; i < modelsToRender; i++)
		{
			for (int j = 0; j < modelsToRender; j++)
			{
				PLYDrawer *modelPointer = model1;
				//if ((i+j) % 2 == 0)
					//modelPointer = model2;

				#pragma region MVP-matrix 1
				//Create model matrix
				glm::mat4 model(1.0f);

				//Normalize the model scale
				float scaleFactor = std::max(modelPointer->width, std::max(modelPointer->height, modelPointer->depth));
				scaleFactor = 1.0f / scaleFactor;
				model = scale(model, vec3(scaleFactor, scaleFactor, scaleFactor));
				//Tile the models in the x/z-plane
				model = glm::translate(model, vec3(
					modelPointer->width * j - modelPointer->width * modelsToRender/2.0 + modelPointer->width / 2.0, //width
					-modelPointer->height / 2.0 - modelPointer->minPos.y, //height
					modelPointer->depth * i - modelPointer->depth * modelsToRender / 2.0 + modelPointer->depth / 2.0)); //depth


				//Create projection matrix
				glm::mat4 projection;
				projection = glm::perspective(45.0f, (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
				//Create MVP matrix
				mat4 MVP = projection * view * model;
				#pragma endregion

				#pragma region Uniforms
				//Send the transformation matrix to the vertex shader
				GLuint transformLoc = glGetUniformLocation(shaderProgramID, "MVP");
				glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(MVP));
				//Send model matrix to shader
				GLuint modeltransLoc = glGetUniformLocation(shaderProgramID, "model");
				glUniformMatrix4fv(modeltransLoc, 1, GL_FALSE, glm::value_ptr(model));
				//Set lightpos uniform
				GLint lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
				glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
				#pragma endregion

				//RENDERING HERE
				modelPointer->drawPlyModel(shaderProgramID, LOD);


			}
		}*/

		// Render museum layout
		#pragma region Cube-rendering
		for (int i = 0; i < museumLayout.size(); i++)
		{
			int control_j = 0;
			for (int j = 0; j < museumLayout[i].size(); j++)
			{
				float xPos = museumLayout.size() * (float(i) / float(museumLayout.size())) - museumLayout.size() / 2.0f;
				float zPos = museumLayout[i].size() * (float(j) / float(museumLayout[i].size())) - museumLayout[i].size() / 2.0f;

				//Create model matrix
				glm::mat4 model(1.0f);
				bool isFloor = false;

				//Draw wall.
				if (museumLayout[i][j] == 2)
				{
					model = glm::scale(model, vec3(1, 3, 1));
					model = glm::translate(model, vec3(xPos, 0, zPos));
				}
				//Draw floor and ceiling.
				else
				{
					if (j == control_j)
					{
						model = glm::translate(model, vec3(xPos, -1, zPos));
					}
					else
					{
						//model = glm::translate(model, vec3(xPos, 1, zPos));
						control_j = j;
					}
				}

				#pragma region Cube MVP-matrix
				//Create projection matrix
				glm::mat4 projection;
				projection = glm::perspective(45.0f, (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
				//Create MVP matrix
				mat4 MVP = projection * view * model;
				#pragma endregion

				#pragma region Cube uniforms
				//Send the transformation matrix to the vertex shader
				GLuint transformLoc = glGetUniformLocation(shaderProgramID, "MVP");
				glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(MVP));
				//Send model matrix to shader
				GLuint modeltransLoc = glGetUniformLocation(shaderProgramID, "model");
				glUniformMatrix4fv(modeltransLoc, 1, GL_FALSE, glm::value_ptr(model));
				//Set lightpos uniform
				GLint lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
				glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
				#pragma endregion

				cube.drawCube(shaderProgramID);

				if (isFloor)
					j--;
				control_j++;
				
			}
		}
		#pragma endregion


		// Render museum models
		#pragma region Model-rendering
		for (int i = 0; i < museumLayout.size(); i++)
		{
			for (int j = 0; j < museumLayout[i].size(); j++)
			{
				float xPos = museumLayout.size() * (float(i) / float(museumLayout.size())) - museumLayout.size() / 2.0f;
				float zPos = museumLayout[i].size() * (float(j) / float(museumLayout[i].size())) - museumLayout[i].size() / 2.0f;

				PLYDrawer *modelPointer;
				if (museumLayout[i][j] == 3)
					modelPointer = model1;
				else if (museumLayout[i][j] == 4)
					modelPointer = model2;
				else if (museumLayout[i][j] == 5)
					modelPointer = model1;//modelPointer = model3;
				else
					continue;
					//modelPointer = model2;


				#pragma region Model MVP-matrix
				//Create model matrix
				glm::mat4 model(1.0f);

				//vec4 minPos = vec4(modelPointer->minPos, 1.0f);

				//Translate the model in the x/z-plane.
				model = glm::translate(model, vec3(xPos, /*-modelPointer->height / 2.0 -modelPointer->minPos.y*/0, zPos));
				//Normalize the model scale
				float scaleFactor = std::max(modelPointer->width, std::max(modelPointer->height, modelPointer->depth));
				scaleFactor = 1.0f / scaleFactor;
				model = scale(model, vec3(scaleFactor, scaleFactor, scaleFactor));

				//minPos = scale(mat4(1.0f), vec3(scaleFactor, scaleFactor, scaleFactor)) * minPos;

				//model = translate(model, vec3(0, -minPos.y, 0));

				//Create projection matrix
				glm::mat4 projection;
				projection = glm::perspective(45.0f, (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
				//Create MVP matrix
				mat4 MVP = projection * view * model;
				#pragma endregion

				#pragma region Model uniforms
				//Send the transformation matrix to the vertex shader
				GLuint transformLoc = glGetUniformLocation(shaderProgramID, "MVP");
				glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(MVP));
				//Send model matrix to shader
				GLuint modeltransLoc = glGetUniformLocation(shaderProgramID, "model");
				glUniformMatrix4fv(modeltransLoc, 1, GL_FALSE, glm::value_ptr(model));
				#pragma endregion

				modelPointer->drawPlyModel(shaderProgramID, LOD);

			}
		}
		#pragma endregion


		//Swap the buffers
		glfwSwapBuffers(window);

		#pragma region fps-counter
		//Show fps in window title
		double t = glfwGetTime();
		// If one second has passed, or if this is the very first frame
		if ((t - t0) > 1.0 || frames == 0)
		{
			double fps = (double)frames / (t - t0);
			if(LOD <= octreeLevels)
				sprintf(titlestring, "RRMM (%.1f fps) LOD: %d / %d", fps, LOD, numberOfLODs);
			else
				sprintf(titlestring, "RRMM (%.1f fps) Original mesh", fps);
			glfwSetWindowTitle(window, titlestring);
			t0 = t;
			frames = 0;
		}
		frames++;
		#pragma endregion

	}

	glfwTerminate();
	delete model1;
	//delete model2;

	return 0;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_1)
		modelsToRender = 1;
	else if (key == GLFW_KEY_2)
		modelsToRender = 2;
	else if (key == GLFW_KEY_3)
		modelsToRender = 3;
	else if (key == GLFW_KEY_4)
		modelsToRender = 4;
	else if (key == GLFW_KEY_5)
		modelsToRender = 5;
	else if (key == GLFW_KEY_6)
		modelsToRender = 6;
	else if (key == GLFW_KEY_7)
		modelsToRender = 7;
	else if (key == GLFW_KEY_8)
		modelsToRender = 8;
	else if (key == GLFW_KEY_9)
		modelsToRender = 9;
	else if (key == GLFW_KEY_0)
		modelsToRender = 10;

	if (key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		if (LOD < numberOfLODs)
			LOD++;
	}
	else if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		if (LOD > 0)
			LOD--;
	}


	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		wireframe++;
		if (wireframe > 2)
			wireframe = 0;
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		exitProgram = true;

}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (leftMousePressed)
	{
		//Rotate around Y-axis
		view = rotate(view, (float)(xpos - mouseX) / 100, vec3(0.0f, 1.0f, 0.0f));

		//Rotate around up/down
		mat3 rotMat(view);
		vec3 transl(view[3]);
		vec3 camera_pos = -transl * rotMat;

		camera_pos = vec3(camera_pos.x, 0.0f, camera_pos.z);
		vec3 temp = normalize(cross(vec3(0.0f, 1.0f, 0.0f), camera_pos));
		view = rotate(view, (float)(ypos - mouseY) / 150, temp);
	}


}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//Zoom in and out through scrolling
	view = scale(view, vec3(1.0 + 0.1*yoffset));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		leftMousePressed = true;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		leftMousePressed = false;
}

void updateView()
{
	static double lastTime = glfwGetTime();
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);
	float mouseSpeed = 0.0015f;
	float speed = 1.5f;

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(windowWidth / 2 - xpos);
	verticalAngle += mouseSpeed * float(windowHeight / 2 - ypos);

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	// Up vector
	glm::vec3 up = glm::cross(right, direction);

	// Move forward
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= right * deltaTime * speed;
	}

	// Camera matrix
	view = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;

	// Reset mouse position for next frame
	glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);


}