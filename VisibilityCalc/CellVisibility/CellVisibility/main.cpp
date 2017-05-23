#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <cmath>

#include <glm/glm.hpp>

using namespace std;

void readFile(vector<vector<int> > &museumLayout, const string &inputName);
map<pair<int, int>, set<pair<int, int> > > calculateVisibility(const vector<vector<int>> &layout);
void writeFile(const map<pair<int, int>, set<pair<int, int>>> visibility, const string &outputName);


int main()
{

	// The user chooses which file to calculate the visibility for.
	string inputName;
	cout << "Calculate cell-to-cell visibility for input file: ";
	cin >> inputName;

	// Read the museum input file.
	vector<vector<int> > museumLayout;
	readFile(museumLayout, inputName);

	// Calculate visibility.
	map<pair<int, int>, set<pair<int, int>>> visibility;
	visibility = calculateVisibility(museumLayout);

	cout << "Writing results to file 'visibility.txt'..." << endl;

	// Write the calculated visibility to a file.
	writeFile(visibility, "visibility.txt");

	cout << "Done! You can close this window now :)" << endl;

	int dummy;
	cin >> dummy;

	return 0;
}

void readFile(vector<vector<int> > &museumLayout, const string &inputName)
{
	// Read museum input file.
	ifstream inputFile;
	inputFile.open(inputName);

	if (inputFile.is_open())
	{
		string line;
		for (int i = 0; getline(inputFile, line); i++)
		{
			stringstream ss;
			ss << line;
			vector<int> temp;
			museumLayout.push_back(temp);
			int number;
			while (ss >> number)
				museumLayout[i].push_back(number);
		}
		inputFile.close();
	}
}

void writeFile(map<pair<int, int>, set<pair<int, int>>> visibility, const string &outputName)
{
	ofstream output;

	output.open(outputName);

	for (map<pair<int, int>, set<pair<int, int>>>::iterator it = visibility.begin(); it != visibility.end(); it++)
	{
		output << it->first.first << " " << it->first.second << " ";

		for (set<pair<int, int>>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			output << it2->first << " " << it2->second << " ";
		}
		output << -1 << endl;
	}

	output.close();

}

map<pair<int,int>, set<pair<int,int>>> calculateVisibility(const vector<vector<int>> &layout)
{
	map<pair<int, int>, set<pair<int, int>>> visibility;

	long long iterations = 0;
	int iterationsSinceLastChange = 0;

	while (iterations < 1000000 && iterationsSinceLastChange < 100000)
	{
		// Pick a random cell.
		int cell_x = rand() % layout[0].size();
		int cell_y = rand() % layout.size();

		// Check that the picked cell is a walkable cell.
		while (layout[cell_y][cell_x] == 2 || layout[cell_y][cell_x] == 0)
		{
			// If not, generate a new cell.
			cell_x = rand() % layout[0].size();
			cell_y = rand() % layout.size();
		}
		visibility[make_pair(cell_x, cell_y)].insert(make_pair(cell_x, cell_y));


		// Generate two numbers between 0 and 1 and add them to the cell-position to get our starting position.
		float xPos = ((float)rand() / (RAND_MAX)) + (float)cell_x;
		float yPos = ((float)rand() / (RAND_MAX)) + (float)cell_y;
		glm::vec2 startPos(xPos, yPos);

		// Generate a random direction-vector and normalize it.
		float dir_x = ((float)rand() / (RAND_MAX)) - 0.5f;
		float dir_y = ((float)rand() / (RAND_MAX)) - 0.5f;
		glm::vec2 directionVector = glm::normalize(glm::vec2(dir_x, dir_y));

		// Step in that direction until an occlusion is met or until a certain number of steps has been done (when were outside the layout).
		float stepSize = 0.001f;
		float maxSteps = sqrt(pow(layout.size(), 2) + pow(layout[0].size(), 2)) / stepSize;

		bool changeWasMade = false;

		for (int step = 0; step < maxSteps; step++)
		{
			glm::vec2 tracePosition = startPos + directionVector * (step * stepSize);

			// Get the cell that the trace is currently at.
			int traceCell_x = (int)floor(tracePosition.x);
			int traceCell_y = (int)floor(tracePosition.y);

			// Check that we're not outside the layout.
			if (traceCell_x >= layout[0].size() || traceCell_y >= layout.size() || traceCell_x < 0 || traceCell_y < 0)
				break;

			// Try to add the traced cell to list of visible cells.
			pair<set<pair<int, int>>::iterator, bool> insertion1 = visibility[make_pair(cell_x, cell_y)].insert(make_pair(traceCell_x, traceCell_y));
			pair<set<pair<int, int>>::iterator, bool> insertion2 = visibility[make_pair(traceCell_x, traceCell_y)].insert(make_pair(cell_x, cell_y));

			// If an insertion was made, make sure that the "iterationsSinceLastChange" is set to 0 further on.
			if (insertion1.second || insertion2.second)
				changeWasMade = true;

			// Check if the traced cell is an occluder.
			if (layout[traceCell_y][traceCell_x] == 2)
			{
				// Stop the tracing of this ray.
				break;
			}
		}

		if (!changeWasMade)
			iterationsSinceLastChange++;
		else
			iterationsSinceLastChange = 0;
		
		if (iterations % 10000 == 0)
		{
			cout << iterations << "/1000000 iterations, (" << ((float)iterations / 1000000.0f) * 100 << "%) , size of map: " << visibility.size() << endl;
			cout << "Iterations since last change: " << iterationsSinceLastChange << " (will stop iterating if 100 000 is reached)" << endl << endl;
		}
		iterations++;
	}

	if (iterationsSinceLastChange >= 100000)
		cout << "Performed 100 000 iterations without finding anything new. Stopping search." << endl;
	cout << iterations << " has been performed in total." << endl;

	return visibility;

}