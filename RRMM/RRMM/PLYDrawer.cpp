#include "PLYDrawer.h"
#include <iostream>

PLYDrawer::PLYDrawer(const PLYModel &ply, const int nLevelsOctree, int nLOD)
	:model(ply), nLODs(nLOD)
{
	maxPos = glm::vec3(0, 0, 0);
	minPos = glm::vec3(0, 0, 0);

	copyVBO.resize(nLODs + 1);
	copyVAO.resize(nLODs + 1);
	copyEBO.resize(nLODs + 1);

	std::cout << endl << "Loading model..." << endl;

	// Fill the vboArray with values.
	for (uint i = 0; i < model.vertexCount * 3; i++)
	{
		// Vertex positions
		if (i % 3 == 0)
			vboArray.push_back(model.positions[i / 3]);
		// Vertex normal, filled with temporary value.
		else if (i % 3 == 1)
			vboArray.push_back(glm::vec3(0, 0, 0));
		// Vertex color
		else
			vboArray.push_back(glm::vec3(0.5, 0.6, 0.9));

		// Calculate the smallest and largest x,y,z positions.
		if (model.positions[i / 3].x < minPos.x)
			minPos.x = model.positions[i / 3].x;
		if (model.positions[i / 3].y < minPos.y)
			minPos.y = model.positions[i / 3].y;
		if (model.positions[i / 3].z < minPos.z)
			minPos.z = model.positions[i / 3].z;

		if (model.positions[i / 3].x > maxPos.x)
			maxPos.x = model.positions[i / 3].x;
		if (model.positions[i / 3].y > maxPos.y)
			maxPos.y = model.positions[i / 3].y;
		if (model.positions[i / 3].z > maxPos.z)
			maxPos.z = model.positions[i / 3].z;
	}

	width = abs(maxPos.x - minPos.x);
	height = abs(maxPos.y - minPos.y);
	depth = abs(maxPos.z - minPos.z);

	nOriginalFaces = model.faceCount;

	computeNormals();

	computeVerticeFaceRelation();

	std::cout << "Model loaded!" << endl << endl;

	// Create an octree for the model.
	std::cout << "Creating octree..." << endl;
	octreeLevels = nLevelsOctree;
	createOctree(nLevelsOctree);
	std::cout << "Octree created!" << endl << endl;

	// Create different LOD's for the model.
	allVBOs.resize(nLODs);
	allIndexes.resize(nLODs);

	for (int i = 1; i <= nLODs; i++)
	{
		std::cout << "Creating LOD's, " << i << "/" << nLODs << endl;
		chooseLOD(nLevelsOctree * i / nLODs);
		allVBOs[i - 1] = vboOctree;
		allIndexes[i - 1] = newIndexes;
		createNewBuffers(copyVBO[i - 1], copyVAO[i - 1], copyEBO[i - 1], i - 1, true);
	}
	std::cout << "Finished creating LOD's!" << endl << endl;
	createNewBuffers(copyVBO[nLODs], copyVAO[nLODs], copyEBO[nLODs], nLODs, true);

	vboOctree.clear();
	newIndexes.clear();
	deleteOctree(octreeRoot);


}

void PLYDrawer::createNewBuffers(GLuint &VBO, GLuint &VAO, GLuint &EBO, int LOD, bool createForOctree)
{
	if (LOD >= nLODs)
		createForOctree = false;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	if(!createForOctree)
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vboArray.size(), &vboArray[0], GL_STATIC_DRAW);
	else
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*allVBOs[LOD].size(), &allVBOs[LOD][0], GL_STATIC_DRAW);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	if(!createForOctree)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec3)*model.faceCount, &model.faces[0], GL_STATIC_DRAW);
	else
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec3)*allIndexes[LOD].size(), &allIndexes[LOD][0], GL_STATIC_DRAW);

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

	// Save the adresses.
	copyVBO[LOD] = VBO;
	copyVAO[LOD] = VAO;
	copyEBO[LOD] = EBO;
}

void PLYDrawer::drawPlyModel(GLuint shaderProgramID, int LOD)
{
	bool renderOctree = true;
	if (LOD >= nLODs)
	{
		renderOctree = false;
		LOD = nLODs;
	}

	glBindBuffer(GL_ARRAY_BUFFER, copyVBO[LOD]);
	
	if (!renderOctree)
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vboArray.size(), &vboArray[0], GL_STATIC_DRAW);
		glUseProgram(shaderProgramID);
		glBindVertexArray(copyVAO[LOD]);
		glDrawElements(GL_TRIANGLES, sizeof(glm::ivec3)*model.faceCount, GL_UNSIGNED_INT, 0);
	}
	else
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*allVBOs[LOD].size(), &allVBOs[LOD][0], GL_STATIC_DRAW);
		glUseProgram(shaderProgramID);
		glBindVertexArray(copyVAO[LOD]);
		glDrawElements(GL_TRIANGLES, sizeof(glm::ivec3)*allIndexes[LOD].size() , GL_UNSIGNED_INT, 0);
	}


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void PLYDrawer::computeNormals()
{
	if (vboOctree.size() == 0)
	{
		// Calculate several normals for every vertex and add together.
		for (int i = 0; i < model.faces.size(); i++)
		{
			glm::vec3 vertexA = vboArray[model.faces[i].x * 3];
			glm::vec3 vertexB = vboArray[model.faces[i].y * 3];
			glm::vec3 vertexC = vboArray[model.faces[i].z * 3];

			glm::vec3 vecA = vertexC - vertexA;
			glm::vec3 vecB = vertexC - vertexB;

			glm::vec3 n = normalize(cross(vecA, vecB));

			vboArray[model.faces[i].x * 3 + 1] += n;
			vboArray[model.faces[i].y * 3 + 1] += n;
			vboArray[model.faces[i].z * 3 + 1] += n;
		}
		// Normalize all normals.
		for (int i = 1; i < vboArray.size(); i += 3)
		{
			vboArray[i] = normalize(vboArray[i]);
		}
	}
	else
	{
		// Calculate several normals for every vertex and add together.
		for (int i = 0; i < newIndexes.size(); i++)
		{
			glm::vec3 vertexA = vboOctree[newIndexes[i].x * 3];
			glm::vec3 vertexB = vboOctree[newIndexes[i].y * 3];
			glm::vec3 vertexC = vboOctree[newIndexes[i].z * 3];

			glm::vec3 vecA = vertexC - vertexA;
			glm::vec3 vecB = vertexC - vertexB;

			glm::vec3 n = normalize(cross(vecA, vecB));

			vboOctree[newIndexes[i].x * 3 + 1] += n;
			vboOctree[newIndexes[i].y * 3 + 1] += n;
			vboOctree[newIndexes[i].z * 3 + 1] += n;
		}
		// Normalize all normals.
		for (int i = 1; i < vboOctree.size(); i += 3)
		{
			vboOctree[i] = normalize(vboOctree[i]);
		}
	}


}

// Fills the vector "verticeFaces" where each slot represents a
// vertex and containts a list of face-indexes to the triangles 
// that the vertex is part of.
void PLYDrawer::computeVerticeFaceRelation()
{
	verticeFaces.resize(model.vertexCount);

	for (int i = 0; i < model.faceCount; i++)
	{
		verticeFaces[model.faces[i].x].push_back(i);
		verticeFaces[model.faces[i].y].push_back(i);
		verticeFaces[model.faces[i].z].push_back(i);
	}
}

void PLYDrawer::deleteOctree(Node *node)
{
	for (int i = 0; i < node->children.size(); i++)
	{
		deleteOctree(node->children[i]);
	}
	if (node->children.size() == 0)
		delete node;
}


void PLYDrawer::chooseLOD(int LOD)
{

	vboOctree.clear();

	vector<Node*> nodes;

	nodes.push_back(octreeRoot);
	vector<Node*> leafNodes;

	// Iterate downwards in the octree until the right LOD.
	for (int i = 0; i < LOD; i++)
	{
		vector<Node*> tempNodes;

		// Fill the vector temp with the children of all nodes in nodes-vector.
		for (int j = 0; j < nodes.size(); j++)
		{
			for (int k = 0; k < nodes[j]->children.size(); k++)
				tempNodes.push_back(nodes[j]->children[k]);
			if (nodes[j]->children.size() == 0)
				leafNodes.push_back(nodes[j]);
		}
			
		// Empty the nodes-vector and fill it with the childrens nodes.
		nodes.clear();
		nodes = tempNodes;
	}
	// A lazy and pretty ugly solution to make sure that the full mesh is showed when full LOD is selected.
	// I tried to fix it by preventing leaf-node's vertices from merging and then try to give them a correct
	// index, but it was very complicated, it was easier to just create temporary nodes for each vertex in a
	// leaf-node and the algorithm will do all the work for me.
	for (int i = 0; i < leafNodes.size(); i++)
		for (int j = 0; j < leafNodes[i]->vertices.size(); j++)
		{
			nodes.push_back(new Node);
			nodes[nodes.size() - 1]->vertices.push_back(leafNodes[i]->vertices[j]);
		}

	// Create a vector for the vertex indexes.
	// The first dimension represents nodes and the second dimension all vertices in that node.
	vector<vector<uint> > vertexIDs;
	vertexIDs.resize(nodes.size());


	// Fill the vector with all the vertex-id's.
	for (int i = 0; i < nodes.size(); i++)
		getVertexIDs(nodes[i], i, vertexIDs);
	
	int success = 0, fail = 0;

	// Calculate new merged vertices.
	// Loop through all nodes.
	for (int i = 0; i < vertexIDs.size()/* - leafNodes.size()*/; i++)
	{
		// Skip the nodes with no vertices inside.
		while (vertexIDs[i].size() == 0 && i < vertexIDs.size())
			i++;

		glm::vec3 averagePosition = glm::vec3(0, 0, 0);
		glm::vec3 normal = glm::vec3(0, 0, 0);
		glm::vec3 color = glm::vec3(0, 0, 0);

		set<uint> connectedTriangles;
		
		// Loop through all vertices of one node.
		for (int j = 0; j < vertexIDs[i].size(); j++)
		{
			// Add together the positions, normals and color of all the vertices.
			averagePosition += vboArray[vertexIDs[i][j] * 3];
			normal += vboArray[vertexIDs[i][j] * 3 + 1];
			color += vboArray[vertexIDs[i][j] * 3 + 2];

			// Add the indexes of all triangles connected to the vertices.
			for (int k = 0; k < verticeFaces[vertexIDs[i][j]].size(); k++)
				connectedTriangles.insert(verticeFaces[vertexIDs[i][j]][k]);
		}
		
		// Correct the averagePosition, color and normal.
		averagePosition /= vertexIDs[i].size();
		color /= vertexIDs[i].size();
		normal = normalize(normal);

		Eigen::Matrix4f M;
		M.fill(0.0f);

		// Loop through the connected triangles in this node.
		for (set<uint>::iterator it = connectedTriangles.begin(); it != connectedTriangles.end(); it++)
		{
			// Get the triangle index.
			ivec3 face = model.faces[*it];
			// Get the triangle vertices.
			vec3 a = model.positions[face.x];
			vec3 b = model.positions[face.y];
			vec3 c = model.positions[face.z];
			// Compute the triangle normal.
			vec3 normal = normalize(cross((a - b), (c - b)));
			float d = -normal.x * a.x - normal.y * a.y - normal.z * a.z;
			// Compute the plane vector.
			Eigen::Vector4f plane = Eigen::Vector4f(normal.x, normal.y, normal.z, d);
			// Add it to the M-matrix.
			M += (plane * plane.transpose());
		}

		

		//M /= connectedTriangles.size();

		// Make some adjustments to the M-matrix before the inverse.
		M(3, 0) = 0.0f;
		M(3, 1) = 0.0f;
		M(3, 2) = 0.0f;
		M(3, 3) = 1.0f;
		M(0, 0) += 1.0f;
		M(1, 1) += 1.0f;
		M(2, 2) += 1.0f;
		M(0, 3) += -averagePosition.x;
		M(1, 3) += -averagePosition.y;
		M(2, 3) += -averagePosition.z;

		//cout << endl << M << endl;

		Eigen::Matrix4f A;
		bool inverseOK;
		M.computeInverseWithCheck(A, inverseOK, 0.01f);


		// Add everything to the new vbo.
		if (inverseOK)
		{
			Eigen::Vector4f temp = A * Eigen::Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
			Eigen::Vector4f test = temp - Eigen::Vector4f(averagePosition.x, averagePosition.y, averagePosition.z, 1.0f);
			vec3 newPosition = vec3(temp(0), temp(1), temp(2));

			//Discard the new vertex if its position is too far from the average position.
			if (distance(newPosition, nodes[i]->centroid) > distance(averagePosition, nodes[i]->centroid) * 2)
			{
				inverseOK = false;
			}
			else
			{
				vboOctree.push_back(newPosition);
				vboOctree.push_back(normal); //temporarily, the normals will have to be recalculated later on.
				vboOctree.push_back(color);
				success++;
			}

		}
		if(!inverseOK)
		{
			vboOctree.push_back(averagePosition);
			vboOctree.push_back(normal);
			vboOctree.push_back(color);
			fail++;
		}



	}

	std::cout << success << " successful merges created with error metrics, " << fail << " created as means." << endl;

	// Copy the original indexes to the vector that will contain the new.
	newIndexes = model.faces;
	// Give all the indexes new values, this is to be able to separate them from the new indexes.
	// The correct position can still be obtained by subtracting "model.faceCount".
	for (int i = 0; i < newIndexes.size(); i++)
		newIndexes[i] += model.faceCount;


	// Change the indexes to the correct ones, according to the new vbo.
	// Loop through the non-leaf nodes.
	int correctVertexIndex = 0;
	for (int nodeIndx = 0; nodeIndx < vertexIDs.size(); nodeIndx++)
	{
		//Skip the nodes with no vertices, without incrementing the correctVertexIndex.
		while (vertexIDs[nodeIndx].size() == 0 && nodeIndx < vertexIDs.size())
			nodeIndx++;

		// Loop through the vertices in that node.
		for (int vertexIndx = 0; vertexIndx < vertexIDs[nodeIndx].size(); vertexIndx++)
			// Loop through the faces connected to that vertex.
			for (int i = 0; i < verticeFaces[vertexIDs[nodeIndx][vertexIndx]].size(); i++)
			{
				int faceIndex = verticeFaces[vertexIDs[nodeIndx][vertexIndx]][i];

				if (newIndexes[faceIndex].x == vertexIDs[nodeIndx][vertexIndx] + model.faceCount)
					newIndexes[faceIndex].x = correctVertexIndex;
				if (newIndexes[faceIndex].y == vertexIDs[nodeIndx][vertexIndx] + model.faceCount)
					newIndexes[faceIndex].y = correctVertexIndex;
				if (newIndexes[faceIndex].z == vertexIDs[nodeIndx][vertexIndx] + model.faceCount)
					newIndexes[faceIndex].z = correctVertexIndex;
			}
		correctVertexIndex++;
	}





	// Create a vector with indexes to all degenerate triangles.
	vector<uint> removeIndex;
	for (int i = 0; i < newIndexes.size(); i++)
	{
		if (newIndexes[i].x == newIndexes[i].y)
			removeIndex.push_back(i);
		else if (newIndexes[i].x == newIndexes[i].z)
			removeIndex.push_back(i);
		else if (newIndexes[i].y == newIndexes[i].z)
			removeIndex.push_back(i);
	}
	// Remove the degenerate triangles, starting from the back of the index-list.
	for (int i = removeIndex.size() - 1; i >= 0; i--)
		newIndexes.erase(newIndexes.begin() + removeIndex[i]);

	nOctreeFaces = newIndexes.size();
}

// A recursive function to fill a certain position in a vector of vectors with all the vertex-id's
// that is somewhere below a certain node in the octree.
void PLYDrawer::getVertexIDs(Node* node, int position, vector<vector<uint> > &ids)
{
	// Add all the vertex-id's of the current node to the vector.
	for (int i = 0; i < node->vertices.size(); i++)
		ids[position].push_back(node->vertices[i]);

	// Run the function on each of the node's children, if there is any.
	for (int i = 0; i < node->children.size(); i++)
		getVertexIDs(node->children[i], position, ids);
}


void PLYDrawer::createOctree(int maxDepth)
{
	octreeRoot = new Node;
	
	// Use the smallest/largest positions from the model to create the boundaries of the octree.
	float smallest = std::min(minPos.z, std::min(minPos.x, minPos.y));
	float largest = std::max(maxPos.z, std::max(maxPos.x, maxPos.y));

	octreeRoot->radius = (largest - smallest) / 2;
	octreeRoot->centroid = glm::vec3(largest - smallest, largest - smallest, largest - smallest) / 2.0f;

	// This offset will be used later for vertex/boundingbox comparisons.
	octreeOffset = glm::vec3(-smallest, -smallest, -smallest);

	// Copy the indexes from all the original vertices.
	for (int i = 0; i < vboArray.size() / 3; i++)
		octreeRoot->vertices.push_back(i);

	// When the root of the octree is created, call the other function to start the recursion.
	createOctree(maxDepth, octreeRoot);
}



void PLYDrawer::createOctree(int remainingDepth, Node* node)
{
	// If the maximum depth has been reached, stop the recursion.
	if (remainingDepth <= 0)
		return;

	// If the node contains only one vertex or fewer, stop the recursion.
	if (node->vertices.size() <= 1)
		return;

	// Create 8 new nodes as children to current.
	for (int i = 0; i < 8; i++)
	{
		node->children.push_back(new Node);
		node->children[i]->radius = node->radius / 2.0f;
		node->children[i]->centroid = node->centroid;
	}

	// Correct the children centroids.
	node->children[0]->centroid += glm::vec3(-0.5f * node->centroid.x, -0.5f * node->centroid.y, -0.5f * node->centroid.z);
	node->children[1]->centroid += glm::vec3(0.5f * node->centroid.x, -0.5f * node->centroid.y, -0.5f * node->centroid.z);
	node->children[2]->centroid += glm::vec3(-0.5f * node->centroid.x, 0.5f * node->centroid.y, -0.5f * node->centroid.z);
	node->children[3]->centroid += glm::vec3(0.5f * node->centroid.x, 0.5f * node->centroid.y, -0.5f * node->centroid.z);
	node->children[4]->centroid += glm::vec3(-0.5f * node->centroid.x, -0.5f * node->centroid.y, 0.5f * node->centroid.z);
	node->children[5]->centroid += glm::vec3(0.5f * node->centroid.x, -0.5f * node->centroid.y, 0.5f * node->centroid.z);
	node->children[6]->centroid += glm::vec3(-0.5f * node->centroid.x, 0.5f * node->centroid.y, 0.5f * node->centroid.z);
	node->children[7]->centroid += glm::vec3(0.5f * node->centroid.x, 0.5f * node->centroid.y, 0.5f * node->centroid.z);
	
	
	// Place the vertices from the current node in the correct children-node.
	for (int i = 0; i < node->vertices.size(); i++)
	{
		// Get the vertex position inside the bounding-box.
		glm::vec3 vertexPos = vboArray[node->vertices[i] * 3] + octreeOffset;
		int child = 0;

		if (vertexPos.x > node->centroid.x)
			child += 1;
		
		if (vertexPos.y > node->centroid.y)
			child += 2;

		if (vertexPos.z > node->centroid.z)
			child += 4;
		
		node->children[child]->vertices.push_back(node->vertices[i]);
	}
	
	// Remove all vertice-pointers from the current node.
	node->vertices.clear();

	// Continue the octree creation at the node children.
	for (int i = 0; i < node->children.size(); i++)
		createOctree(remainingDepth - 1, node->children[i]);

}

PLYDrawer::~PLYDrawer()
{
	//deleteOctree(octreeRoot);
	model.FreeMemory();
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
