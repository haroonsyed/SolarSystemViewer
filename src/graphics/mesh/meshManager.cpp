#include "meshManager.h"
#include <iostream>
#include <GL/glew.h>
#include "./meshImporter.h"

MeshManager* MeshManager::m_instance = nullptr;
std::vector<unsigned int> MeshManager::m_bufferInfo;
std::unordered_map<std::string, std::vector<unsigned int>> MeshManager::m_meshMap;

MeshManager::MeshManager() {};

MeshManager* MeshManager::getInstance() {
    if (m_instance == nullptr) {
        m_instance = new MeshManager();
    }
    return m_instance;
}

std::vector<unsigned int> MeshManager::getBufferInfo() {
    return m_bufferInfo;
}

void MeshManager::bindMesh(std::string meshFilePath) {

    // Shaders are cached so they aren't built every time
    std::string meshKey = meshFilePath;

    if (m_meshMap.count(meshKey) == 0) {

        MeshImporter importer;
        std::vector<float> meshData = importer.readSepTriMesh(meshFilePath);
        float* vertices = &meshData[0];

        unsigned int numDataPoints = 11; // Each vertex has pos(3), tex(2), norm(3), tan(3)
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
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, numDataPoints * sizeof(float), (void*)(3 * sizeof(float))); // Texture data
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, numDataPoints * sizeof(float), (void*)(5 * sizeof(float))); // Normal data
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, numDataPoints * sizeof(float), (void*)(8 * sizeof(float))); // Tangent data
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);

        // Add to map
        m_meshMap[meshKey] = std::vector<unsigned int>{ VAO, VBO, numVertices };

    }

    // Bind the correct VAO
    m_bufferInfo = m_meshMap.at(meshKey);
    glBindVertexArray(m_bufferInfo[0]);

}