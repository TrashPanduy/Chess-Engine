#include "ModelLoader.h"

//Public methods
void ModelLoader::Destroy() {
   vkDestroyBuffer(device, indexBuffer, nullptr);
   vkFreeMemory(device, indexBufferMemory, nullptr);

   vkDestroyBuffer(device, vertexBuffer, nullptr);
   vkFreeMemory(device, vertexBufferMemory, nullptr);
}
void ModelLoader::Load(const char* Path)
{
    if (device == VK_NULL_HANDLE) {
        throw std::runtime_error("ModelLoader::Load - VkDevice is null");
    }
    loadModel(Path);
    createVertexBuffer();
    createIndexBuffer();
}
void ModelLoader::Render(VkCommandBuffer commandBuffer)
{
    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);


    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

//loading models
//uses tinyObjs, .obj files.
//loads a single model according to preset values, MODEL_PATH.c_str() is path to model file.
void ModelLoader::loadModel(const char* Path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    vertices.clear();
    indices.clear();

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, Path)) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            //update aabbMin and aabbMax
            aabbMin = glm::min(aabbMin, vertex.pos);
            aabbMax = glm::max(aabbMax, vertex.pos);

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }

    }
   

}
void ModelLoader::createVertexBuffer() {
    if (vertices.empty()) return; // prevent crash
    if (vertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vertexBuffer = VK_NULL_HANDLE;
    }
    if (vertexBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, vertexBufferMemory, nullptr);
        vertexBufferMemory = VK_NULL_HANDLE;
    }
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}
void ModelLoader::createIndexBuffer() {
    if (indices.empty()) return;
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    if (indexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, vertexBuffer, nullptr);
        indexBuffer = VK_NULL_HANDLE;
    }
    if (indexBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, indexBufferMemory, nullptr);
        indexBufferMemory = VK_NULL_HANDLE;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}
void ModelLoader::LoadDebugLine(glm::vec3 from, glm::vec3 to, glm::vec3 color) {
    vertices.clear();
    indices.clear();

    vertices = {
    { from, color, glm::vec2(0.0f) },
    { to,   color, glm::vec2(1.0f) }
    };


    indices = { 0, 1 };

    createVertexBuffer();
    createIndexBuffer();

    aabbMin = glm::min(from, to);
    aabbMax = glm::max(from, to);
}

void ModelLoader::GenerateAABBCube(glm::vec3 color) {
    vertices.clear();
    indices.clear();

    glm::vec3 min = aabbMin;
    glm::vec3 max = aabbMax;

    // 8 corners of AABB
    std::vector<Vertex> verts = {
        {{min.x, min.y, min.z}, color, {0.0f, 0.0f}}, // 0
        {{max.x, min.y, min.z}, color, {1.0f, 0.0f}}, // 1
        {{max.x, max.y, min.z}, color, {1.0f, 1.0f}}, // 2
        {{min.x, max.y, min.z}, color, {0.0f, 1.0f}}, // 3
        {{min.x, min.y, max.z}, color, {0.0f, 0.0f}}, // 4
        {{max.x, min.y, max.z}, color, {1.0f, 0.0f}}, // 5
        {{max.x, max.y, max.z}, color, {1.0f, 1.0f}}, // 6
        {{min.x, max.y, max.z}, color, {0.0f, 1.0f}}  // 7
    };

    std::vector<uint32_t> lines = {
        0, 1, 1, 2, 2, 3, 3, 0, // bottom face
        4, 5, 5, 6, 6, 7, 7, 4, // top face
        0, 4, 1, 5, 2, 6, 3, 7  // vertical edges
    };

    vertices = verts;
    indices = lines;

    createVertexBuffer();
    createIndexBuffer();
}