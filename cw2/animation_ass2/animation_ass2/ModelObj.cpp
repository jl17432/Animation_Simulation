#include "ModelObj.h"
#include <iostream>


//constructor
ModelObj::ModelObj()
{
    vertices.resize(0);
    faces.resize(0);
    normals.resize(0);

}



// read an obj file
bool ModelObj::readObjFile(string & filename)
{
    //open obj file
    ifstream myOBJ;
    myOBJ.open(filename);
    //if can not open, print issue and return false
    if (!myOBJ)
    {
        cout << "can not open file: " << filename;
        return false;
    }

    // contains space and backslash, for split the string
    char   deliminator[] = " /";

    // stores data read in
    char * buffer = new char[1024];
    string token = "";


    // loop to read when is not eof
    while (!myOBJ.eof())
    {
        token.clear();
        myOBJ.getline(buffer, 1024);
        istringstream line(buffer);
        line >> token;
        shiftMode(token);
        parseObj(line);
        line.clear();
    }


    return true;
}

// called when reading an obj file, change to the right parsing mode
void ModelObj::shiftMode(const string & token)
{
    // vertex 
    if (token == "v")
    {
        mode = VERTEX;
    }
    // normal
    else if (token == "vn")
    {
        mode = NORMAL;
    }
    // face
    else if (token == "f")
    {
        mode = FACE;
    }
    // texture
    else if (token == "vt")
    {
        mode = TEXTURE;
    }
    // other circumstances, e.g. comments, object name, materials, etc...
    else
    {
        mode = OTHER;
    }
}

// called when reading an obj file, choose the right function to call
void ModelObj::parseObj(istringstream & line)
{
    switch (mode)
    {
        case VERTEX:
            parseVerts(line);
            break;

        case NORMAL:
            parseNormals(line);
            break;

        case TEXTURE:
            parseTexture(line);
            break;

        case FACE:
            parseFace(line);

        // for other circumstances
        default:
            break;
    }
}

// read in a vertex
void ModelObj::parseVerts(istringstream & line)
{
    Vertex v;
    line >> v.x >> v.y >> v.z;
    vertices.push_back(v);
}

//read in a normal
void ModelObj::parseNormals(istringstream & line)
{
    Vertex n;
    line >> n.x >> n.y >> n.z;
    normals.push_back(n);
}

//read in a texture
void ModelObj::parseTexture(istringstream& line)
{
    Vertex t;
    line >> t.x >> t.y;
    t.z = 0;
    textures.push_back(t);
}

//read in a face, this function had f&&king consumed my brain cells...
void ModelObj::parseFace(istringstream& line)
{
    //the face going to push_back
    TriangleFace face;

    //goint to store the string piece sliced from the line
    string tempStr = "";

    //assuming the obj is written in triangle mode, i.e. three vertice per face
    for (int i = 0; i < 3; i++)
    {   
        //get sliced piece for one vertex
        tempStr.clear();
        line >> tempStr;

        string subString, vertex, texture, normal = "";

        //find first '/', if there is one...
        size_t pos = tempStr.find("/", 0);
    
        if (pos == string::npos)
        {
            //if reach here, means there are no texture point and normal for this vertex
            // 
            //record vertex ID
            face.vertices[i] = stoi(tempStr)-1;
            continue;
        }
        else
            //if reach here, means there are texture point or normal or both for this vertex
        {
            //slice the string and get vertex ID, don't forget to -1
            vertex = tempStr.substr(0, pos);
            face.vertices[i] = stoi(vertex) - 1;

            //find second '/', if there is one...
            size_t tpos = tempStr.find("/", pos + 1);
            if (tpos == string::npos)
            {
                //if reach here, means there are no normal for this vertex
                texture = tempStr.substr(pos + 1, tempStr.size());
                //but still have a chance it don't have a texture ID. If it has, record it
                if (texture != ""){ face.textures[i] = stoi(texture)-1; }
                continue;
            }
            else 
            {
                //if reach here, means the vertex has a normal.

                //make sure there actually exist a texture ID
                if (tpos > pos + 2){ texture = tempStr.substr(pos + 1, tpos - 1); }
                normal = tempStr.substr(tpos + 1, tempStr.size());

                //record texture and normal
                if (texture != "") { face.textures[i] = stoi(texture)-1; }
                if (normal != "") { face.normals[i] = stoi(normal)-1; }
            }

        }

    }
    //finally...........push the face in to the stack
    faces.push_back(face);

}


void ModelObj::writeObj(ofstream& file)
{
   
    // write vertex
    for (auto &vertex : vertices)
    {
        file << "v " << vertex.x << " " << vertex.y << " " << vertex.z << endl;
    }
    // write texture points...if there is any...
    if (textures.size() > 0)
    {
        for (auto &texture : textures)
        {
            file << "vt " << texture.x << " " << texture.y << endl;
        }
    }
    // write normals...if there is any...
    if (normals.size() > 0)
    {
        for (auto &normal : normals)
        {
            file << "vn " << normal.x << " " << normal.y << " " << normal.z << endl;
        }
    }
    // write faces
    for (auto &face : faces)
    {
        string output_line = "f ";

        // for the 3 vertices
        for (int i = 0; i < 3; i++)
        {
            output_line += to_string(face.vertices[i] + 1);

            // if the vertex has a texture ID
            bool hasTexture = false;
            if (face.textures[i] != -1)
            {
                hasTexture = true;
                output_line += "/";
                output_line += to_string(face.textures[i] + 1);
            }
            // if the vertex has a normal ID
            if (face.normals[i] != -1)
            {
                output_line += "/";
                // incase the vertex doesn't have a texture ID
                if (!hasTexture) { output_line += "/"; }
                output_line += to_string(face.normals[i] + 1);
            }

            output_line += " ";
        }
        //finished one face
        file << output_line << endl;
    }
    cout << "finished writing file! " << endl;
}


//void ModelObj::calculateMassCentre()
//{
//    float maxX = 0, minX = 0, maxY = 0, minY = 0, maxZ = 0, minZ = 0;
//    for (auto& vertex : vertices)
//    {
//        maxX = max(maxX, vertex.x);
//        maxY = max(maxY, vertex.y);
//        maxZ = max(maxZ, vertex.z);
//        minX = min(minX, vertex.x);
//        minY = min(minY, vertex.y);
//        minZ = min(minZ, vertex.z);
//    }
//    massCentre.x = minX + 0.5f * (maxX - minX);
//    massCentre.y = minY + 0.5f * (maxY - minY);
//    massCentre.z = minZ + 0.5f * (maxZ - minZ);
//}