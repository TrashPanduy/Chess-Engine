#pragma once
#include "TextureImage.h"


class ImGuiLayer{
public:
    void init(GLFWwindow* window, VkInstance instance, VkDevice device, VkRenderPass renderPass, VkQueue queue, uint32_t imageCount, QueueFamilyIndices indices, VkPhysicalDevice pdevice, VkSampleCountFlagBits MSAA);
    void newFrame();
    void render(VkCommandBuffer commandBuffer);
    void cleanup();
    void genStartScreen(GLFWwindow* window, GameState& gameState);
    bool get_escape_menu_state() {
        return  escape_menu_active;
    }
    void set_escape_menu_state(bool state) {
        escape_menu_active = state;
    }

    TextureImage MenuBackground;
    ImTextureID backgroundTexture;


private:
    void createDescriptorPool();
    void escape_menu(GLFWwindow* window, GameState& gameState);
    bool escape_menu_active = false;
    // Store any Vulkan objects if needed (descriptor pool, etc.)
    VkDescriptorPool imguiDescriptorPool;
    VkDevice device;
    VkQueue graphicsQueue;
    VkPhysicalDevice physicalDevice;

};
