#pragma once
#include "VulkanHelper.h"
class TextureImage : VulkanHelper
{
public:
	VkDescriptorSet descriptorSet;
	void Load(const char* path);
	void Free();
	void Destroy();
private:
	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	void createTextureImage(const char* path);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	void createTextureImageView();
	void createTextureSampler();
	void createTextureDescriptorSet();

};