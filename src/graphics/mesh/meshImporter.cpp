#include "meshImporter.h"

#include <fstream>
#include <iostream>
#include "../../util.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/*********************/
// PRIVATE FUNCTIONS
/*********************/

// Returns the n-let of float data from an index of vertex index (ex: x,y,z position for vertex #5)
std::vector<float> MeshImporter::getIndexedData(std::vector<float> vIndex, int index, int vSize = 3)
{
    std::vector<float> vertex;
    // Get vertices and return them 
    for (int i = 0; i < vSize; i++) {
      vertex.push_back(vIndex[index * vSize + i]);
    }

    return vertex;
}

// Used to build the index of vertex data for an attribute. The actual indices themseleves are from buildFaces
template <typename type>
std::vector<type> MeshImporter::getAttributeIndex(std::string meshFilePath, std::string attribute) {
    std::ifstream file(meshFilePath);
    StringUtil sutil;

    std::vector<type> attributeIndex;
    std::string line;

    while (std::getline(file, line)) {

        if (line.empty()) {
            continue;
        }

        auto delimited = sutil.delimit(line, ' ');

        // Build index
        if (delimited[0] == attribute) {

          // Add information. Start at one to avoid identifier
          for (int i = 1; i < delimited.size(); i++) {
            attributeIndex.push_back((type)std::stof(delimited[i]));
          }

        }

    }

    return attributeIndex;
}

// Given a string containing vertex data in .obj format ; returns vectorized indices
// NEEDS CHANGE TO SUPPORT TEXTURES.
std::vector<unsigned int> MeshImporter::buildVertex(std::string line) {

    StringUtil sutil;
    std::vector<std::string> delimited = sutil.delimit(line, '/');
    std::vector<unsigned int> vertexIndices;
    /*
    1. Position
    2. Texture
    3. Normal
    */

   for (std::string indice : delimited) {
       vertexIndices.push_back(std::stoul(indice) - 1); // -1 to account for obj index start at 1 instead of 0
   }

    return vertexIndices;
}

// Given a line containing an ngon, breaks into tris and adds vertices to list
// Output format is vector of vertices each containing a vector of vertex data
std::vector<std::vector<unsigned int>> MeshImporter::buildFace(std::string line) {

    StringUtil sutil;
    std::vector<std::vector<unsigned int>> face;

    auto delimited = sutil.delimit(line, ' ');

    if (delimited[0] != "f") return face; // Return nothing if the line is not face data

    // Loop breaks n-gons into tris
    for (int i = 0; i < delimited.size() - 3; i++) {

        face.push_back(buildVertex(delimited[1]));
        face.push_back(buildVertex(delimited[i + 2]));
        face.push_back(buildVertex(delimited[i + 3]));

    }

    return face;
}

// [vertex][position texture normal] 
std::vector<std::vector<unsigned int>> MeshImporter::buildMesh(std::string meshFilePath) {

    std::ifstream file(meshFilePath);
    StringUtil sutil;
    std::vector<std::vector<unsigned int>> meshData;
    std::string line;

    while (std::getline(file, line)) {

        if (line.empty()) {
            continue;
        }

        auto face = buildFace(line);
        meshData.insert(meshData.end(), face.begin(), face.end());

    }

    return meshData;
}

// Divides vIndex by maximum coordinate within it
void MeshImporter::normalizeMesh(std::vector<float>& vIndex)
{
    // Normalize vertices
    float max = 0;
    for (float v : vIndex) {
        max = std::abs(v) > max ? v : max;
    }
    for (int i = 0; i < vIndex.size(); i++) {
        vIndex[i] /= max;
    }
}

/*********************/
// PUBLIC FUNCTIONS
/*********************/

// Returns an index of all the vertices in a mesh
std::vector<float> MeshImporter::getPositionIndex(std::string meshFilePath)
{
    std::vector<float> posIndex = getAttributeIndex<float>(meshFilePath, "v");

    return posIndex;
}

// Gets all normals corresponding to the vertices in the vIndex
std::vector<float> MeshImporter::getNormalIndex(std::string meshFilePath) {
    std::vector<float> normalIndex = getAttributeIndex<float>(meshFilePath, "vn");

    return normalIndex;
}

// Gets all uv coordinates for textures corresponding to vertices
std::vector<float> MeshImporter::getTexIndex(std::string meshFilePath) {
    std::vector<float> uvCoords = getAttributeIndex<float>(meshFilePath, "vt");

    return uvCoords;
}

// Groundwork for index buffer object if implemented later
// std::vector<unsigned int> MeshImporter::getMesh(std::string meshFilePath) {
//     std::vector<unsigned int> mesh;

//     std::vector<std::vector<unsigned int>> vertexVec = buildMesh(meshFilePath);

//     // Inefficient I know, but I really like being able to address each vertex indiv.
//     // If there are performance issues when loading I'll directly load the mesh
//     for (auto vertex : vertexVec) {
//         mesh.insert(mesh.end(), vertex.begin(), vertex.end());
//     }

//     return mesh;
// }

// Uses indexes to build a separate tri structure
std::vector<float> MeshImporter::readSepTriMesh(std::string meshFilePath)
{

    std::vector<float> posIndex = getPositionIndex(meshFilePath);
    std::vector<float> nIndex = getNormalIndex(meshFilePath);
    std::vector<float> texIndex = getTexIndex(meshFilePath);
    std::vector<std::vector<unsigned int>> mesh = buildMesh(meshFilePath);
    normalizeMesh(posIndex);

    std::vector<float> vertices;

    if(posIndex.size() == 0 ||  nIndex.size() == 0 || texIndex.size() == 0) {
        std::cout << "Missing mesh data!" << std::endl;
    }

    // Go through the posIndex and fIndex and build a separate tri structure with color data
    // Vertex Format: x,y,z,nx,ny,nz,r,g,b

    int vertexNumber = 0;
    for (std::vector<unsigned int> vertex : mesh) {

        // Get coord data from index
        auto pos = getIndexedData(posIndex, vertex[0]);
        auto tex = getIndexedData(texIndex, vertex[1], 2);
        auto norm = getIndexedData(nIndex, vertex[2]); 

        //Insert into vertices, with color data
        vertices.insert(vertices.end(), pos.begin(), pos.end());
        vertices.insert(vertices.end(), tex.begin(), tex.end());
        vertices.insert(vertices.end(), norm.begin(), norm.end());

        vertexNumber++;

    }

    return vertices;
}




