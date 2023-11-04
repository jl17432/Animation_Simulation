#pragma once
#include "ModelObj.h"
#include "Vertex.h"



using namespace std;

class Spring
{

private:
	// ElasticCoefficient as a constance
	float ElasticCoefficient = 10.f;

public:
	// 3 types of springs, but seems not needed...
	enum SPRING_TYPE { STRUCTUAL, SHEAR, FLEXION };
	SPRING_TYPE spring_type;
	
	// constructor, given left & right IDs, type and the model
	Spring(int left, int right, SPRING_TYPE type, ModelObj& model);
	
	// parametres
	int leftID;
	int rightID;
	float rest_len;
	

	// calculate current length
	float update_curr_len(ModelObj & model) { return (model.getVert(leftID) - model.getVert(rightID)).modulus(); }

	// return the force the left vertex teken
	Vertex calculateLeftForce(ModelObj& model);

	// return the force the right vertex teken
	Vertex calculateRightForce(ModelObj& model);


};