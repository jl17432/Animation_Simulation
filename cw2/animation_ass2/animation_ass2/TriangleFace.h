#pragma once
#include "Vertex.h"




class TriangleFace
{

public:
    int vertices[3]{ -1, -1, -1 };
    int textures[3]{ -1, -1, -1 };
    int normals[3]{ -1, -1, -1 };
    Vertex colors[3];

    TriangleFace();

};