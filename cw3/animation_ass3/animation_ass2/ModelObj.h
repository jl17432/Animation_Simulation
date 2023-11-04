#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Vertex.h"
#include "TriangleFace.h"
#include <sstream>

using namespace std;

class ModelObj
{
private:
	

	//the file going to render
	enum LINE_MODE { VERTEX, FACE, NORMAL, TEXTURE, OTHER };
	LINE_MODE mode;


	// called when reading an obj file, change to the right parsing mode
	void shiftMode(const string& token);

	// called when reading an obj file, choose the right function to call
	void parseObj(istringstream& line);

	//read in a line of vertex
	void parseVerts(istringstream& line);

	//read in a line of normal
	void parseNormals(istringstream& line);

	//read in a line of texture
	void parseTexture(istringstream& line);

	//read in a line of face
	void parseFace(istringstream& line);


public:

	//verts
	vector<Vertex> vertices;
	//faces
	vector<TriangleFace> faces;
	//normals
	vector<Vertex> normals;
	//texture points, stores as a Vertex, but only use x and y value for UVs, z is always 0
	vector<Vertex> textures;


	//constructor
	ModelObj();

	//get vertex given the index
	Vertex getVert(int index) { return vertices[index]; }

	//get normal given the index
	Vertex getNorm(int index) { return normals[index]; }

	//get texture given the index
	Vertex getTexture(int index) { return textures[index]; }

	//get triangle face given the index
	TriangleFace getFace(int index) { return faces[index]; }

	//add a new vertex
	void pushVert(Vertex vert) { vertices.push_back(vert); }

	//add a new face
	void pushFace(TriangleFace face) { faces.push_back(face); }

	//total num of verts
	int countVerts() { return int(vertices.size()); }

	//total num of faces
	int countFaces() { return int(faces.size()); }

	//read an obj file
	bool readObjFile(string & filename);

	
	//write data into an obj file
	void writeObj(ofstream& file);


};