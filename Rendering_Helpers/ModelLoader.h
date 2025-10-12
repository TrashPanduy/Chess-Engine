#pragma once
#include "VulkanHelper.h"

class ModelLoader : public VulkanHelper {

private:
    void loadModel(const char* Path);
    void createVertexBuffer();
    void createIndexBuffer();

    glm::vec3 aabbMin = glm::vec3(FLT_MAX);
    glm::vec3 aabbMax = glm::vec3(-FLT_MAX);

protected:
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkCommandPool commandPool;
    VkQueue graphicsQueue;

public:
    ModelLoader() = default;

    ModelLoader(VkDevice d, VkPhysicalDevice p, VkCommandPool c, VkQueue q) {
        device = d;
        physicalDevice = p;
        commandPool = c;
        graphicsQueue = q;
    }

    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    void Load(const char* path);
    void Destroy();
    void Render(VkCommandBuffer commandBuffer);

    void LoadDebugLine(glm::vec3 from, glm::vec3 to, glm::vec3 color = glm::vec3(1, 0, 0));

    glm::vec3 getAABBMin() const { return aabbMin; }
    glm::vec3 getAABBMax() const { return aabbMax; }
    void setAABBMin(glm::vec3 m) { aabbMin = m; }
    void setAABBMax(glm::vec3 m) { aabbMax = m; }
    void GenerateAABBCube(glm::vec3 color);

};