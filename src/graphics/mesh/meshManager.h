#pragma once
#include <string>
#include <vector>
#include <unordered_map>

// Singleton pattern (I think it's fine since this class takes over code in the main)
// I could decouple, but I really like the caching feature
// NOT THREAD SAFE
class MeshManager {
private:
    static MeshManager* m_instance;
    static std::vector<unsigned int> m_bufferInfo; // Contains filename -> [vao, vbo, vertexCount]
    static std::unordered_map<std::string, std::vector<unsigned int>> m_meshMap; // Contains filename -> [vao, vbo, vertexCount]
    MeshManager();

public:
    static MeshManager* getInstance();
    // Contains filename -> [vao, vbo, vertexCount]
    std::vector<unsigned int> getBufferInfo();
    void bindMesh(std::string meshFilePath);
};