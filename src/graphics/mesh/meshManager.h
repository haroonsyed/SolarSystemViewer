#pragma once
#include <string>
#include <unordered_map>

// Singleton pattern (I think it's fine since this class takes over code in the main)
// I could decouple, but I really like the caching feature
// NOT THREAD SAFE
class MeshManager {
private:
    static MeshManager* instance;
    static std::vector<unsigned int> bufferInfo; // Contains filename -> [vao, vbo, vertexCount]
    static std::unordered_map<std::string, std::vector<unsigned int>> meshMap; // Contains filename -> [vao, vbo, vertexCount]
    MeshManager();

public:
    static MeshManager* getInstance();
    // Contains filename -> [vao, vbo, vertexCount]
    std::vector<unsigned int> MeshManager::getBufferInfo();
    void bindMesh(std::string meshFilePath);
};