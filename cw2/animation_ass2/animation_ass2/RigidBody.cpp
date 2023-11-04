#include "RigidBody.h"

void RigidBody::transformModel(ModelObj &model, float xTransform, float yTransform, float zTransform)
{
	for (int i = 0; i < model.countVerts(); i++)
	{
		model.vertices[i] = model.vertices[i] + Vertex(xTransform, yTransform, zTransform);
		
	}
}

void RigidBody::rotateModel_X(ModelObj &model, float xRotation)
{
	float degree = xRotation * PI / 180.f;
	for (int i = 0; i < model.countVerts(); i++)
	{
		Vertex v = model.vertices[i];
		model.vertices[i].y = v.y * cos(degree) - v.z * sin(degree);
		model.vertices[i].z = v.y * sin(degree) + v.z * cos(degree);
	}
}
void RigidBody::rotateModel_Y(ModelObj& model, float yRotation)
{
	float degree = yRotation * PI / 180.f;
	for (int i = 0; i < model.countVerts(); i++)
	{
		Vertex v = model.vertices[i];
		model.vertices[i].x = v.x * cos(degree) + v.z * sin(degree);
		model.vertices[i].z = -v.x * sin(degree) + v.z * cos(degree);
	}
}
void RigidBody::rotateModel_Z(ModelObj& model, float zRotation)
{
	float degree = zRotation * PI / 180.f;
	for (int i = 0; i < model.countVerts(); i++)
	{ 
		Vertex v = model.vertices[i];
		model.vertices[i].x = v.x * cos(degree) - v.y * sin(degree);
		model.vertices[i].y = v.x * sin(degree) + v.y * cos(degree);
	}
}

vector<int> RigidBody::generateSortedVertex(ModelObj &model)
{
	vector<int> result;
	result.resize(model.vertices.size());

	// insertion sort the vertex, smaller Z-value comes first, then smaller X-value comes first. 
	int vertexOneRow = sqrt(model.vertices.size());
	for (int i = 1; i < model.vertices.size(); i++)
	{
		Vertex currVertex = model.getVert(i);
		int j = i - 1;

		while (j >= 0 && model.getVert(result[j]).z * vertexOneRow + model.getVert(result[j]).x > currVertex.z * vertexOneRow + currVertex.x)
		{
			result[j + 1] = result[j];
			j--;
		}
		result[j + 1] = i;
	}

	return result;

}

void RigidBody::pushStructualNeighbours(int vertexID, vector<int> & neighbourVec, vector<Spring> &springVec, ModelObj& model)
{
	int vertexPerRow = sqrt(neighbourVec.size());
	int size = neighbourVec.size();


	int right = vertexID + 1;

	int below = vertexID + vertexPerRow;


	if (right >= 0 && (vertexID + 1) % vertexPerRow != 0) {
		springVec.push_back(Spring::Spring(neighbourVec[vertexID], neighbourVec[right], Spring::STRUCTUAL, model));
	}

	if (below >= 0 && below < size) { 
		springVec.push_back(Spring::Spring(neighbourVec[vertexID], neighbourVec[below], Spring::STRUCTUAL, model));
	}

}

void RigidBody::pushShearNeighbours(int vertexID, vector<int>& neighbourVec, vector<Spring> &springVec, ModelObj& model)
{
	int vertexPerRow = sqrt(neighbourVec.size());
	int size = neighbourVec.size();


	int right_above = vertexID - vertexPerRow + 1;

	int right_below = vertexID + vertexPerRow + 1;


	if (right_above >= 0 && (vertexID + 1) % vertexPerRow != 0) { 
		springVec.push_back(Spring::Spring(neighbourVec[vertexID], neighbourVec[right_above], Spring::SHEAR, model));
	}

	if (right_below >= 0 && (vertexID + 1) % vertexPerRow != 0 && right_below < size) { 
		springVec.push_back(Spring::Spring(neighbourVec[vertexID], neighbourVec[right_below], Spring::SHEAR, model));
	}

}

void RigidBody::pushFlexionNeighbours(int vertexID, vector<int>& neighbourVec, vector<Spring> &springVec, ModelObj& model)
{
	int vertexPerRow = sqrt(neighbourVec.size());
	int size = neighbourVec.size();

	int right = vertexID + 2;
	int below = vertexID + 2 * vertexPerRow;
	int right_below = vertexID + 2 + 2 * vertexPerRow;
	
	if (right >= 0 && (vertexID + 1) % vertexPerRow != 0 && (vertexID + 2) % vertexPerRow != 0) { 
		springVec.push_back(Spring::Spring(neighbourVec[vertexID], neighbourVec[right], Spring::FLEXION, model));
	}

	if (below >= 0 && below < size) { 
		springVec.push_back(Spring::Spring(neighbourVec[vertexID], neighbourVec[below], Spring::FLEXION, model));
	}

	if (right_below >= 0 && (vertexID + 1) % vertexPerRow != 0 && (vertexID + 2) % vertexPerRow != 0 && right_below < size) {
		springVec.push_back(Spring::Spring(neighbourVec[vertexID], neighbourVec[right_below], Spring::FLEXION, model));
	}

}




vector<Spring> RigidBody::generateSpringVec(ModelObj& model)
{
	vector<Spring> result;
	vector<int> sortedVertexID = generateSortedVertex(model);

	for (int i = 0; i < sortedVertexID.size(); i++)
	{
		pushStructualNeighbours(i, sortedVertexID, result, model);
		pushShearNeighbours(i, sortedVertexID, result, model);
		pushFlexionNeighbours(i, sortedVertexID, result, model);
	}

	// only for the hanging part, does nothing for the main logic.
	model.leftCorner  = sortedVertexID[0];
	model.rightCorner = sortedVertexID[sqrt(sortedVertexID.size()) - 1];

	return result;
}