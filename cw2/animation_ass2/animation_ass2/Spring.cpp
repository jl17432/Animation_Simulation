#include "Spring.h"

Spring::Spring(int left, int right, SPRING_TYPE type, ModelObj& model)
{
	leftID = left;
	rightID = right;
	spring_type = type;
	
	rest_len = (model.getVert(leftID) - model.getVert(rightID)).modulus();
}

Vertex Spring::calculateLeftForce(ModelObj& model)
{

	Vertex dir = (model.getVert(rightID) - model.getVert(leftID)).normalize();
	update_curr_len(model);
	float force = ElasticCoefficient * ((model.getVert(leftID) - model.getVert(rightID)).modulus() - rest_len);
	
	return dir * force;

}

Vertex Spring::calculateRightForce(ModelObj& model)
{
	Vertex dir = (model.getVert(leftID) - model.getVert(rightID)).normalize();
	update_curr_len(model);
	float force = ElasticCoefficient * ((model.getVert(leftID) - model.getVert(rightID)).modulus() - rest_len);

	return dir * force;

}
