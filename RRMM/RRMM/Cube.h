#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

using namespace std;
using namespace glm;

class Cube
{
public:
	Cube();

	void drawCube(GLuint shaderProgramID);
	

private:
	void createBuffers();

	uint VBO, VAO, EBO;
	vector<vec3> vboArray;
	vector<ivec3> indexes;
};