#include "common.h" 

VkDevice device;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkQueue graphicsQueue;
VkCommandPool commandPool;
VkDescriptorPool descriptorPool;
VkDescriptorSetLayout uniformDescriptorSetLayout;
VkDescriptorSetLayout textureDescriptorSetLayout;
unsigned int WIDTH = 1280;
unsigned int HEIGHT = 720;
