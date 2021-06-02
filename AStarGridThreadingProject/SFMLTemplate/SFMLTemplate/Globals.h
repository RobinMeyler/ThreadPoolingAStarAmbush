// Author: Robin Meyler
// Date: 2021
// Title: Thread Pool Implmentation of A* Ambush

#pragma once
#include <vector>
#include "MyVector3.h"

// Nodes for A*
struct Node
{
	int ID;
	bool passable;
	int arcIDs[4];
	int numberOfProcessed;
	bool occupied = false;
	std::vector<Node*> arcs;
	MyVector3 position;
	std::vector<bool> marked;
	std::vector<float> costToGoal;
	std::vector<float> totalCostFromStart;
	std::vector<float> totalCostAccumlative;
	std::vector <Node*> previous;
};

