#include "Cube.h"

Cube::Cube()
{
	//Creates a cube with |sides| = 1 and center in origo.

	vec3 color = vec3(1.0f, 0.7f, 0.7f);

	#pragma region Creating cube vertices.
	//Triangle 1
	vboArray.push_back(vec3(0.5f, -0.5f, -0.5f));
	vboArray.push_back(vec3(0, 0, -1));
	vboArray.push_back(color);
	vboArray.push_back(vec3(-0.5f, -0.5f, -0.5f));
	vboArray.push_back(vec3(0, 0, -1));
	vboArray.push_back(color);
	vboArray.push_back(vec3(-0.5f, 0.5f, -0.5f));
	vboArray.push_back(vec3(0, 0, -1));
	vboArray.push_back(color);

	//Triangle 2
	vboArray.push_back(vec3(-0.5f, 0.5f, -0.5f));
	vboArray.push_back(vec3(0, 0, -1));
	vboArray.push_back(color);
	vboArray.push_back(vec3(0.5f, 0.5f, -0.5f));
	vboArray.push_back(vec3(0, 0, -1));
	vboArray.push_back(color);
	vboArray.push_back(vec3(0.5f, -0.5f, -0.5f));
	vboArray.push_back(vec3(0, 0, -1));
	vboArray.push_back(color);

	//Triangle 3
	vboArray.push_back(vec3(0.5f, -0.5f, 0.5f));
	vboArray.push_back(vec3(0, 0, 1));
	vboArray.push_back(color);
	vboArray.push_back(vec3(-0.5f, 0.5f, 0.5f));
	vboArray.push_back(vec3(0, 0, 1));
	vboArray.push_back(color);
	vboArray.push_back(vec3(-0.5f, -0.5f, 0.5f));
	vboArray.push_back(vec3(0, 0, 1));
	vboArray.push_back(color);

	//Triangle 4
	vboArray.push_back(vec3(-0.5f, 0.5f, 0.5f));
	vboArray.push_back(vec3(0, 0, 1));
	vboArray.push_back(color);
	vboArray.push_back(vec3(0.5f, -0.5f, 0.5f));
	vboArray.push_back(vec3(0, 0, 1));
	vboArray.push_back(color);
	vboArray.push_back(vec3(0.5f, 0.5f, 0.5f));
	vboArray.push_back(vec3(0, 0, 1));
	vboArray.push_back(color);

	//Triangle 5
	vboArray.push_back(vec3(0.5f, -0.5f, -0.5f));
	vboArray.push_back(vec3(1, 0, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(0.5f, 0.5f, -0.5f));
	vboArray.push_back(vec3(1, 0, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(0.5f, 0.5f, 0.5f));
	vboArray.push_back(vec3(1, 0, 0));
	vboArray.push_back(color);

	//Triangle 6
	vboArray.push_back(vec3(0.5f, 0.5f, 0.5f));
	vboArray.push_back(vec3(1, 0, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(0.5f, -0.5f, 0.5f));
	vboArray.push_back(vec3(1, 0, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(0.5f, -0.5f, -0.5f));
	vboArray.push_back(vec3(1, 0, 0));
	vboArray.push_back(color);

	//Triangle 7
	vboArray.push_back(vec3(-0.5f, -0.5f, -0.5f));
	vboArray.push_back(vec3(-1, 0, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(-0.5f, 0.5f, 0.5f));
	vboArray.push_back(vec3(-1, 0, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(-0.5f, 0.5f, -0.5f));
	vboArray.push_back(vec3(-1, 0, 0));
	vboArray.push_back(color);

	//Triangle 8
	vboArray.push_back(vec3(-0.5f, 0.5f, 0.5f));
	vboArray.push_back(vec3(-1, 0, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(-0.5f, -0.5f, -0.5f));
	vboArray.push_back(vec3(-1, 0, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(-0.5f, -0.5f, 0.5f));
	vboArray.push_back(vec3(-1, 0, 0));
	vboArray.push_back(color);

	//Triangle 9
	vboArray.push_back(vec3(-0.5f, 0.5f, -0.5f));
	vboArray.push_back(vec3(0, 1, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(-0.5f, 0.5f, 0.5f));
	vboArray.push_back(vec3(0, 1, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(0.5f, 0.5f, 0.5f));
	vboArray.push_back(vec3(0, 1, 0));
	vboArray.push_back(color);

	//Triangle 10
	vboArray.push_back(vec3(0.5f, 0.5f, 0.5f));
	vboArray.push_back(vec3(0, 1, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(0.5f, 0.5f, -0.5f));
	vboArray.push_back(vec3(0, 1, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(-0.5f, 0.5f, -0.5f));
	vboArray.push_back(vec3(0, 1, 0));
	vboArray.push_back(color);

	//Triangle 11
	vboArray.push_back(vec3(-0.5f, -0.5f, -0.5f));
	vboArray.push_back(vec3(0, -1, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(0.5f, -0.5f, 0.5f));
	vboArray.push_back(vec3(0, -1, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(-0.5f, -0.5f, 0.5f));
	vboArray.push_back(vec3(0, -1, 0));
	vboArray.push_back(color);

	//Triangle 12
	vboArray.push_back(vec3(0.5f, -0.5f, 0.5f));
	vboArray.push_back(vec3(0, -1, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(-0.5f, -0.5f, -0.5f));
	vboArray.push_back(vec3(0, -1, 0));
	vboArray.push_back(color);
	vboArray.push_back(vec3(0.5f, -0.5f, -0.5f));
	vboArray.push_back(vec3(0, -1, 0));
	vboArray.push_back(color);
	#pragma endregion


	// Add the indexes.
	for (int i = 0; i < 12 * 3; i += 3)
		indexes.push_back(ivec3(i, i + 1, i + 2));

	createBuffers();

}

void Cube::createBuffers()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vboArray.size(), &vboArray[0], GL_STATIC_DRAW);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec3)*indexes.size(), &indexes[0], GL_STATIC_DRAW);

	//Vertex position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	//Vertex normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), (GLvoid*)(sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);
	//Vertex color attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), (GLvoid*)(2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void Cube::drawCube(GLuint shaderProgramID)
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vboArray.size(), &vboArray[0], GL_STATIC_DRAW);
	glUseProgram(shaderProgramID);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, sizeof(glm::ivec3)*indexes.size(), GL_UNSIGNED_INT, 0);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}