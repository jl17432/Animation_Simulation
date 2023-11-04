#pragma once
#include "math.h"
#include <iomanip>
#include <limits>
#include <iostream>

class Vertex
{
public:
    // the coordinates
    float x, y, z;

    // constructors
    Vertex();
    Vertex(float X, float Y, float Z);
    Vertex(const Vertex& other);

    // equality operator
    bool operator ==(const Vertex& other) const;

    // addition operator
    Vertex operator +(const Vertex& other) const;

    // subtraction operator
    Vertex operator -(const Vertex& other) const;

    // multiplication operator
    Vertex operator *(float factor) const;

    // division operator
    Vertex operator /(float factor) const;

    // dot product routine
    float dot(const Vertex& other) const;

    // cross product routine
    Vertex cross(const Vertex& other) const;

    // routine to find the length
    float modulus() const;

    // normalisation routine
    Vertex normalize() const;

    friend std::ostream& operator<<(std::ostream& os, const Vertex& vertex);
};
