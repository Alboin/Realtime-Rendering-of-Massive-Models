#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include <map>

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
vec3 position = vec3(-2.0f, 0.2f, -2.0f);
bool exitProgram = false;
int windowWidth = 1200;
int windowHeight = 800;
float horizontalAngle = -90, verticalAngle = 0;
int wireframe = 0;
int octreeLevels = 15;
int numberOfLODs = 3;
int LOD = numberOfLODs - 1;

//Some global variables for the fps-counter
double t0 = 0.0;
int frames = 0;
char titlestring[250];

// Function declaration
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void updateView();
map<pair<int, int>, set<pair<int, int>>> loadVisibilityFile();


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
	// Disable mouse-pointer.
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
	glfwSetScrollCallback(window, scroll_callback);
	#pragma endregion

	#pragma region Read museum layout-file.
	vector<vector<uint> > museumLayout;
	ifstream inputFile;
	string inputName;
	cout << "Please enter name of museum layout-file to use: ";
	cin >> inputName;
	inputFile.open(inputName);

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

	// Read the precomputed visibility-information for all the cells.
	map<pair<int, int>, set<pair<int, int>>> visibility = loadVisibilityFile();

	#pragma region Create lists to handle model benefits and LODs.
	// Create list with a benefit-entry for each model to be rendered, also a list with LOD for each model.
	vector<float> modelNextLODBenefits;
	vector<float> modelLODs;
	vector<int> modelTriangleIncrease;
	vector<float> modelDiagonals;
	vector<float> modelDistances;
	vector<int> whichMesh;
	vector<bool> willBeRendered;
	int k = 0;
	for (int i = 0; i < museumLayout.size(); i++)
	{
		for (int j = 0; j < museumLayout[i].size(); j++)
		{
			//Set starting position.
			if (museumLayout[i][j] == 6)
			{
				float xPos =  j - museumLayout.size() / 2.0f;
				float zPos =  i - museumLayout[i].size() / 2.0f;
				position = vec3(xPos, 0.2f, zPos);
			}
			else if (museumLayout[i][j] > 2)
			{
				modelNextLODBenefits.push_back(1);
				modelLODs.push_back(0); // All models start with the lowest LOD.
				modelTriangleIncrease.push_back(1);
				willBeRendered.push_back(true);

				modelDiagonals.push_back(0);
				modelDistances.push_back(0);
				if (museumLayout[i][j] == 3)
					whichMesh.push_back(0);
				else if (museumLayout[i][j] == 4)
					whichMesh.push_back(1);
				else if (museumLayout[i][j] == 5)
					whichMesh.push_back(2);

				k++;
			}
		}
	}
	#pragma endregion


	//Load ply-models
	PLYDrawer *model[3];
	model[0] = new PLYDrawer(PLYModel("models/bunny.ply", false, false), octreeLevels, numberOfLODs);
	model[1] = new PLYDrawer(PLYModel("models/Armadillo.ply", false, false), octreeLevels, numberOfLODs);
	model[2] = new PLYDrawer(PLYModel("models/happy.ply", false, false), octreeLevels, numberOfLODs);

	Cube cube = Cube();

	vec3 lightPos(0.0f, 15.0f, 10.0f);

	double fps = 1;
	int desiredFPS = 30;
	float startingFPS = desiredFPS;
	int trianglesPerSecond = 1;
	float maxCost = -1;
	double deltaTime = 1;
	bool firstSecond = true;

	double startOfLoop = glfwGetTime();


	while (!glfwWindowShouldClose(window) && !exitProgram)
	{
		double startTime = glfwGetTime();


		//Checks if any events are triggered (like keyboard or mouse events)
		glfwPollEvents();
		//Update mouse position
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

		// Reset the triangle-count for each frame.
		int totalTrianglesRendered = 0;

		// Determine player cell position.
		int cell_x = position.x + museumLayout[0].size() / 2.0f + 0.5f;
		int cell_y = position.z + museumLayout.size() / 2.0f + 0.5f;


		// Render museum layout
		#pragma region Cube-rendering
		for (int i = 0; i < museumLayout.size(); i++)
		{
			for (int j = 0; j < museumLayout[i].size(); j++)
			{
				// Check if the wall/floor-cell is visible from current cell.
				set<pair<int, int>>::iterator it = visibility[make_pair(cell_x, cell_y)].find(make_pair(j, i));

				// If it's not visible, skip to the next cube!
				if (it == visibility[make_pair(cell_x, cell_y)].end())
					continue;


				float xPos =  j - museumLayout[i].size() / 2.0f;
				float zPos = i - museumLayout.size() / 2.0f;

				//Create model matrix
				glm::mat4 model(1.0f);
				bool isFloor = false;

				//Draw wall.
				if (museumLayout[i][j] == 2)
				{
					model = glm::scale(model, vec3(1, 3, 1));
					model = glm::translate(model, vec3(xPos, 0, zPos));
				}
				//Draw floor.
				else
				{
					model = glm::translate(model, vec3(xPos, -1, zPos));
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
				GLuint LODlevelLoc = glGetUniformLocation(shaderProgramID, "LODlevel");
				glUniform1i(LODlevelLoc, -1);
				//Set lightpos uniform
				GLint lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
				glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
				#pragma endregion

				cube.drawCube(shaderProgramID);
				
			}
		}
		#pragma endregion


		// Render museum models
		#pragma region Model-rendering
		int k = 0;
		for (int i = 0; i < museumLayout.size(); i++)
		{
			for (int j = 0; j < museumLayout[i].size(); j++)
			{

				float xPos = j - museumLayout[i].size() / 2.0f;
				float zPos = i - museumLayout.size() / 2.0f;

				PLYDrawer *modelPointer;
				if (museumLayout[i][j] == 3)
					modelPointer = model[0];
				else if (museumLayout[i][j] == 4)
					modelPointer = model[1];
				else if (museumLayout[i][j] == 5)
					modelPointer = model[2];
				else
					continue;

				// Check if the model is visible from current cell.
				set<pair<int, int>>::iterator it = visibility[make_pair(cell_x, cell_y)].find(make_pair(j, i));

				// If it's not visible, skip to the next model!
				if (it == visibility[make_pair(cell_x, cell_y)].end())
				{
					willBeRendered[k] = false;
					k++;
					continue;
				}

				willBeRendered[k] = true;

				#pragma region Model MVP-matrix
				//Create model matrix
				glm::mat4 model(1.0f);
				//Translate the model in the x/z-plane.
				model = glm::translate(model, vec3(xPos, /*-modelPointer->height / 2.0 -modelPointer->minPos.y*/0, zPos));
				//Normalize the model scale
				float scaleFactor = std::max(modelPointer->width, std::max(modelPointer->height, modelPointer->depth));
				scaleFactor = 1.0f / scaleFactor;
				model = scale(model, vec3(scaleFactor, scaleFactor, scaleFactor));

				//Create projection matrix
				glm::mat4 projection;
				projection = glm::perspective(45.0f, (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
				//Create MVP matrix
				mat4 MVP = projection * view * model;
				#pragma endregion

				// Use the correct LOD for this model.
				if(!firstSecond)
					LOD = modelLODs[k];

				#pragma region Model uniforms
				//Send the transformation matrix to the vertex shader
				GLuint transformLoc = glGetUniformLocation(shaderProgramID, "MVP");
				glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(MVP));
				//Send model matrix to shader
				GLuint modeltransLoc = glGetUniformLocation(shaderProgramID, "model");
				glUniformMatrix4fv(modeltransLoc, 1, GL_FALSE, glm::value_ptr(model));
				GLuint LODlevelLoc = glGetUniformLocation(shaderProgramID, "LODlevel");
				glUniform1i(LODlevelLoc, LOD);
				#pragma endregion

				// Calculate the scaled bounding-box diagonal.
				vec3 minPos = modelPointer->minPos * scaleFactor;
				vec3 maxPos = modelPointer->maxPos * scaleFactor;
				float diagonal = distance(minPos, maxPos);

				// Calculate the distance between camera and model.
				vec3 cameraPosition = (inverse(view))[3];
				vec3 modelPosition = vec3(xPos, 0, zPos);
				float modelDistance = distance(cameraPosition, modelPosition);

				// Update the benefit for this model.
				modelDiagonals[k] = diagonal;
				modelDistances[k] = modelDistance;

				// Update the number of triangles that an increase in LOD would yield for this model.
				if (LOD < numberOfLODs)
					modelTriangleIncrease[k] = modelPointer->trianglesInLOD[LOD + 1] - modelPointer->trianglesInLOD[LOD];
				else
					modelTriangleIncrease[k] = 0;

				// Increment the model "iterator".
				k++;

				// Draw model.
				modelPointer->drawPlyModel(shaderProgramID, LOD);

				if(willBeRendered[k - 1])
					totalTrianglesRendered += modelPointer->trianglesInLOD[LOD];

			}
		}
		#pragma endregion


		#pragma region Determine LODs for all models.
		int triangleBudget = 0;
		// Set the LOD of all models to 0.
		for (int i = 0; i < modelLODs.size(); i++)
			modelLODs[i] = 0;

		// Loop to configure the LOD's depending on the desired fps (that maxCost depends on).
		while (triangleBudget < maxCost && maxCost > 0)
		{
			// Calculate the benefits for all models.
			for (int i = 0; i < modelNextLODBenefits.size(); i++)
				//modelNextLODBenefits[i] = (pow(2, (float(LOD + 1) / float(numberOfLODs) * octreeLevels)) * modelDistances[i]) / modelDiagonals[i];
				// Inverted function works for some reason instead of the above, which gives high LOD's to objects far away...
				modelNextLODBenefits[i] = modelDiagonals[i] / (pow(2, (float(LOD + 1) / float(numberOfLODs) * octreeLevels)) * modelDistances[i]);

			int indexWithLargestQuantity = 0;
			float largestQuantity = -1;
			for (int i = 0; i < modelLODs.size(); i++)
			{
				float quantity = -1;
				// Make sure that the model does not already have the highest LOD (and also that the model will actually be rendered).
				if (modelLODs[i] < numberOfLODs && willBeRendered[i])
					quantity = modelNextLODBenefits[i] / model[whichMesh[i]]->trianglesInLOD[modelLODs[i] + 1];

				if (quantity > largestQuantity)
				{
					largestQuantity = quantity;
					indexWithLargestQuantity = i;
				}
			}

			// Increase the LOD of the model with the largest quantity.
			if (largestQuantity > 0)
			{
				int current = model[whichMesh[indexWithLargestQuantity]]->trianglesInLOD[modelLODs[indexWithLargestQuantity]];
				int next = model[whichMesh[indexWithLargestQuantity]]->trianglesInLOD[modelLODs[indexWithLargestQuantity] + 1];
				// Make sure that increasing the LOD won't break the maxCost limit.
				if(triangleBudget + next - current < maxCost)
					modelLODs[indexWithLargestQuantity]++;
				else
					break;
			}
			else
				break;

			triangleBudget = 0;
			//Calculate the current number of triangles rendered.
			for (int i = 0; i < modelLODs.size(); i++)
			{
				// But only if the model will actually be rendered.
				if(willBeRendered[i])
					triangleBudget += model[whichMesh[i]]->trianglesInLOD[modelLODs[i]];

			}

		}
		#pragma endregion

		//Swap the buffers
		glfwSwapBuffers(window);

		#pragma region fps-counter
		//Show fps in window title
		double t = glfwGetTime();
		// If one second has passed, or if this is the very first frame
		if ((t - t0) > 1.0f || frames == 0)
		{
			float framerate = (double)frames / (t - t0);
			sprintf(titlestring, "RRMM (%.1f fps) LOD: %d / %d | Total number of triangles: %d", framerate, LOD + 1, numberOfLODs + 1, totalTrianglesRendered);
			glfwSetWindowTitle(window, titlestring);
			t0 = t;
			frames = 0;
		}
		frames++;
		#pragma endregion

		// Compute fps.
		deltaTime = glfwGetTime() - startTime;
		fps = 1.0f / deltaTime;

		// Compute number of triangles rendered per second.
		if (glfwGetTime() - startOfLoop > 1.0f && firstSecond)
		{
			startingFPS = fps;
			maxCost = trianglesPerSecond / desiredFPS;
			firstSecond = false;
		}
		else if(firstSecond)
		{
			trianglesPerSecond += totalTrianglesRendered;
		}


	}

	glfwTerminate();
	delete model[0];
	delete model[1];
	delete model[2];

	return 0;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		wireframe++;
		if (wireframe > 2)
			wireframe = 0;
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		exitProgram = true;


}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//Zoom in and out through scrolling
	view = scale(view, vec3(1.0 + 0.1*yoffset));
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

map<pair<int, int>, set<pair<int, int>>> loadVisibilityFile()
{
	cout << endl << "Reading file with visibility-information..." << endl;

	ifstream inputFile;
	inputFile.open("visibility.txt");

	map<pair<int, int>, set<pair<int, int>>> visibility;

	if (inputFile.is_open())
	{
		// Go through the file until the end.
		while (!inputFile.eof())
		{
			// First two numbers is the cell we're in.
			int x_pos;
			int y_pos;

			inputFile >> x_pos;
			inputFile >> y_pos;

			// Then all visible cells. A -1 marks the end of one line.
			int visible_x;
			int visible_y;

			while (!inputFile.eof())
			{
				inputFile >> visible_x;

				// If the end of the line has been reached, break the loop.
				if (visible_x == -1)
					break;

				inputFile >> visible_y;

				// Add the visible cell to the set coupled with the cell we're in.
				visibility[make_pair(x_pos, y_pos)].insert(make_pair(visible_x, visible_y));
			}
		}
		inputFile.close();
		cout << "Visibility information read!" << endl;

	}
	else
	{
		cout << "ERROR: Could not open file with visibility information!!! ('visibility.txt')" << endl;
	}


	return visibility;

}
