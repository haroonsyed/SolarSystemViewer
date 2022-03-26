#include "meshManager.h"
#include <iostream>
#include <GL/glew.h>
#include "./meshImporter.h"

MeshManager* MeshManager::instance = nullptr;
std::vector<unsigned int> MeshManager::bufferInfo;
std::unordered_map<std::string, std::vector<unsigned int>> MeshManager::meshMap;

MeshManager::MeshManager() {};

MeshManager* MeshManager::getInstance() {
    if (instance == nullptr) {
        instance = new MeshManager();
    }
    return instance;
}

std::vector<unsigned int> MeshManager::getBufferInfo() {
    return bufferInfo;
}

void MeshManager::bindMesh(std::string meshFilePath) {

    // Shaders are cached so they aren't built every time
    std::string meshKey = meshFilePath;

    if (meshMap.count(meshKey) == 0) {

        MeshImporter importer;
        std::vector<float> meshData = importer.readSepTriMesh(meshFilePath);
        float* vertices = &meshData[0];

        unsigned int numDataPoints = 9; // Each vertex has 9 datapoints (pos,norm,col)
        unsigned int numVertices = meshData.size() / numDataPoints;

        // Setup buffers for data
        unsigned int VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numDataPoints * numVertices, vertices, GL_STATIC_DRAW);

        // Add attribute data
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, numDataPoints * sizeof(float), (void*)0); // Position Data
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, numDataPoints * sizeof(float), (void*)(3 * sizeof(float))); // Normal data
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, numDataPoints * sizeof(float), (void*)(6 * sizeof(float))); // Color data
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        // Add to map
        meshMap[meshKey] = std::vector<unsigned int>{ VAO, VBO, numVertices };

    }

    // Bind the correct VAO
    bufferInfo = meshMap.at(meshKey);
    glBindVertexArray(bufferInfo[0]);

}