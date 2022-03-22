#include "mesh.h"
#include "meshImporter.h"

void Mesh::setMesh(std::string meshFilePath) {
  MeshImporter importer;
  this->meshFilePath = meshFilePath;
  mesh = importer.readSepTriMesh(meshFilePath);
}

std::vector<float> Mesh::getMesh() {
  return mesh;
}
