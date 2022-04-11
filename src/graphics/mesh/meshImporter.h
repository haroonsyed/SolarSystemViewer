#pragma once
#include <string>
#include <vector>

class MeshImporter {
  private:
    std::vector<float> getIndexedData(std::vector<float> vIndex, int index, int vSize);
    template <typename type> std::vector<type> getAttributeIndex(std::string meshFilePath, std::string attribute);
    std::vector<unsigned int> buildVertex(std::string vertex);
    std::vector<std::vector<unsigned int>> buildFace(std::string line);
    std::vector<std::vector<unsigned int>> buildMesh(std::string meshFilePath);
    void normalizeMesh(std::vector<float>& vertexIndex);
    std::vector<float> calculateTangent(std::vector<std::vector<float>> vertices, std::vector<std::vector<float>> UVs);
  public:
    std::vector<float> getPositionIndex(std::string meshFilePath);
    std::vector<float> getNormalIndex(std::string meshFilePath);
    std::vector<float> getUVIndex(std::string meshFilePath);
    std::vector<float> readSepTriMesh(std::string meshFilePath);
};
