#pragma once
#include "Vertex.h"
#include <vector>

using namespace std;

class Particle
{
public:

	Vertex position;
	Vertex velocity;
	Vertex velocity_last_frame;
	Vertex force;
	vector<int> neighbours;
	int col_inGrid;
	int row_inGrid;

	float density;


	//constructors
	Particle();
	Particle(Vertex &new_position);


	//update velocity
	void updateVelocity(float delta_t);


	//update position
	void updatePosition(float delta_t);
};