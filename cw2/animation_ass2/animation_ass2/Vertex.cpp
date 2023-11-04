#include "Vertex.h"



//constructors
Vertex::Vertex()
    : x(0.0), y(0.0), z(0.0)
{}

Vertex::Vertex(float X, float Y, float Z)
    : x(X), y(Y), z(Z)
{}

Vertex::Vertex(const Vertex& other)
    : x(other.x), y(other.y), z(other.z)
{}

//operators
bool Vertex::operator ==(const Vertex& other) const
{ 
    return (abs(x - other.x) < std::numeric_limits<float>::epsilon() && abs(y - other.y) < std::numeric_limits<float>::epsilon() && abs(z - other.z) < std::numeric_limits<float>::epsilon());
}

// addition
Vertex Vertex::operator +(const Vertex& other) const
{
    return  Vertex(x + other.x, y + other.y, z + other.z);;
}

// subtraction
Vertex Vertex::operator -(const Vertex& other) const
{ 
    return Vertex(x - other.x, y - other.y, z - other.z);
}

// multiplication
Vertex Vertex::operator *(float factor) const
{
    return Vertex(x * factor, y * factor, z * factor);
}

// division operator
Vertex Vertex::operator /(float factor) const
{
    
    return Vertex(x / factor, y / factor, z / factor);
}

// dot product routine
float Vertex::dot(const Vertex& other) const{
    float returnVal = x * other.x + y * other.y + z * other.z;
    return returnVal;
}

// cross product routine
Vertex Vertex::cross(const Vertex& other) const
{ 
    return Vertex(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
} 

// calculate modulus
float Vertex::modulus() const {
    return sqrt(x * x + y * y + z * z);
}

// normalize the vertex
Vertex Vertex::normalize() const{
    float length = sqrt(x * x + y * y + z * z);
    Vertex returnVal(x / length, y / length, z / length);
    return returnVal;
}


std::ostream& operator<<(std::ostream& os, const Vertex& vertex) {
    os << "(" << vertex.x << ", " << vertex.y << ", " << vertex.z << ") ";
    return os;
}