#include <vector>
#include <set>

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Eigen/Eigen/Core>
#include <Eigen/Eigen/LU>

#include "plyloader.h"

using namespace std;
using namespace glm;

// Data structure used for Octree.
struct Node {
	vector<Node*> children;
	// A list with indexes to all the vertices in this cell.
	vector<uint> vertices;
	//vec3 maxBorder, minBorder;
	vec3 centroid;
	float radius;
};

class PLYDrawer
{
public:
	PLYDrawer(const PLYModel &ply, const int nLevelsOctree, int nLOD);
	~PLYDrawer();

	void drawPlyModel(GLuint shaderProgramID, int LOD);
	void computeNormals();

	void chooseLOD(int LOD);

	void createNewBuffers(GLuint &VBO, GLuint &VAO, GLuint &EBO, int LOD, bool createForOctree = false);

	float width, height, depth;
	// A vec3 containing the smallest x,y & z-positions of all the vertices.
	glm::vec3 minPos;
	// A vec3 containing the largest x,y & z-positions of all the vertices.
	glm::vec3 maxPos;

	Node *octreeRoot;
	int octreeLevels; //set from main.cpp
	void deleteOctree(Node* node);

	vector<uint> trianglesInLOD;

	int nOctreeFaces, nOriginalFaces, nLODs;
private:
	vector<glm::vec3> vboArray;
	vector<glm::vec3> vboOctree;
	vector<glm::ivec3> newIndexes;
	vector<vector<uint> > verticeFaces;

	vector<vector<glm::vec3> > allVBOs;
	vector<vector<glm::ivec3> > allIndexes;

	PLYModel model;

	vec3 octreeOffset;
	void createOctree(int maxDepth);
	void createOctree(int remainingDepth, Node* node);
	void getVertexIDs(Node* node, int position, vector<vector<uint> > &ids);

	void computeVerticeFaceRelation();

	vector<GLuint> copyVBO, copyVAO, copyEBO;
};

