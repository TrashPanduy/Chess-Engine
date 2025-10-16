#pragma once
#include "common.h"

class VulkanHelper{


public:
	bool debugMode = false;

	const char* toString(GameState state) {
		switch (state) {
		case GameState::Main_Menu:   return "Main_Menu";
		case GameState::in_Game:     return "in_Game";
		case GameState::End_Screen:  return "End_Screen";
		default:                     return "Unknown";
		}
	}

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	VkCommandBuffer beginSingleTimeCommands();
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	bool TestRayOBBIntersection(glm::vec3 rayOrigin,
		glm::vec3 rayDir
		, glm::vec3 aabbMin,
		glm::vec3 aabbMax,
		glm::mat4 modelMatrix,
		float& outDistance);

	bool RayIntersectsMesh(const glm::vec3& rayOrigin,
		const glm::vec3& rayDir,
		const std::vector<Vertex>& vertices,
		const std::vector<uint32_t>& indices,
		const glm::mat4& modelMatrix,
		float& closestT);

	bool RayIntersectsTriangle(const glm::vec3& rayOrigin,
		const glm::vec3& rayDir,
		const glm::vec3& v0,
		const glm::vec3& v1,
		const glm::vec3& v2,
		float& outT);
};