#pragma once
#include "ModelObj.h"
#include <unordered_map>
#include "Vertex.h"
#include <algorithm>
#include "Spring.h"

using namespace std;

// pi as a constance
#define PI 3.1415926







/*==============================================================
		*** This is a util class ***

	all the functions are static and will be 
	
	called directly in the main function


==============================================================*/


class RigidBody 
{





public:

	// translation the model
	static void transformModel(ModelObj &model, float xTransform, float yTransform, float zTransform);

	// rotate the model along X-axis
	static void rotateModel_X(ModelObj &model, float xRotation);

	// rotate the model along Y-axis
	static void rotateModel_Y(ModelObj& model, float yRotation);

	// rotate the model along Z-axis
	static void rotateModel_Z(ModelObj& model, float zRotation);

	// to generate a vector to represent the 9x9 vertex matrix
	static vector<int> generateSortedVertex(ModelObj &model);

	// find and push the Structual Neighbours
	static void pushStructualNeighbours(int vertexID, vector<int>& neighbourVec, vector<Spring> &springVec, ModelObj& model);

	// find and push the Shear Neighbours
	static void pushShearNeighbours(int vertexID, vector<int>& neighbourVec, vector<Spring> &springVec, ModelObj& model);

	// find and push the flexion neighbours
	static void pushFlexionNeighbours(int vertexID, vector<int>& neighbourVec, vector<Spring> &springVec, ModelObj& model);

	// return all the springs in a vector
	static vector<Spring> generateSpringVec(ModelObj& model);
};