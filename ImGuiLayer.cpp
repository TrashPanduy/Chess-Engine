#include "ImGuiLayer.h"
#include "VulkanApplication.h"

void ImGuiLayer::createDescriptorPool() {
    VkDescriptorPoolSize pool_sizes[] = {
    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes));
    pool_info.pPoolSizes = pool_sizes;


    if (vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create ImGui descriptor pool!");
    }


}
void ImGuiLayer::init(GLFWwindow* window, VkInstance instance, VkDevice logical_device, VkRenderPass renderPass, VkQueue queue, uint32_t imageCount, QueueFamilyIndices indices, VkPhysicalDevice pdevice,VkSampleCountFlagBits MSAA) {
    this->device = logical_device;
    this->graphicsQueue = queue;
    this->physicalDevice = pdevice;

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    
    createDescriptorPool();
    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.ApiVersion = VK_API_VERSION_1_4;
    init_info.Instance = instance;
    init_info.Device = device;
    init_info.PhysicalDevice = pdevice;
    init_info.QueueFamily = indices.graphicsFamily.value();
    init_info.Queue = graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imguiDescriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = imageCount;
    init_info.ImageCount = imageCount;
    init_info.MSAASamples = MSAA;
    init_info.Allocator = nullptr;
    init_info.RenderPass = renderPass;

    assert(init_info.Instance != VK_NULL_HANDLE);
    assert(init_info.PhysicalDevice != VK_NULL_HANDLE);
    assert(init_info.Device != VK_NULL_HANDLE);
    assert(init_info.Queue != VK_NULL_HANDLE);
    assert(init_info.MinImageCount >= 2);
    assert(init_info.ImageCount >= init_info.MinImageCount);

    if (!ImGui_ImplVulkan_Init(&init_info))
        throw std::runtime_error("Failed to initialize ImGui Vulkan backend.");

}

void ImGuiLayer::newFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::render(VkCommandBuffer cmd) {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

void ImGuiLayer::cleanup() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(device, imguiDescriptorPool, nullptr);
}

void ImGuiLayer::genStartScreen(GLFWwindow* window, GameState& gameState) {
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);

    // Update ImGui display size before newFrame
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)display_w, (float)display_h);

    VulkanApplication* app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));

    // Start new frame *before* UI code
    newFrame();

    if (gameState == GameState::Main_Menu) {
        // Fullscreen transparent background window
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)display_w, (float)display_h));
        ImGui::SetNextWindowBgAlpha(1.0f);

        ImGui::Begin("Main Menu", nullptr, window_flags);

        // Button layout
        ImVec2 button_size(200, 60);
        float total_height = button_size.y * 2 + 40.0f; // 2 buttons + spacing
        float start_y = (io.DisplaySize.y - total_height) * 0.5f;
        ImGui::SetCursorPos(ImVec2((io.DisplaySize.x - button_size.x) * 0.5f, start_y));
        if (ImGui::Button("Vs AI", button_size)) {
            app->set_AI_Team(Black,true);
            gameState = in_Game;
        }
        ImGui::SetCursorPos(ImVec2((io.DisplaySize.x - button_size.x) * 0.5f, start_y + button_size.y+10));
        if (ImGui::Button("Co-Op", button_size)) {
            app->set_AI_Team(Null,false);
            gameState = in_Game;
        }

        ImGui::SetCursorPos(ImVec2((io.DisplaySize.x - button_size.x) * 0.5f, start_y + button_size.y + 80.0f));
        if (ImGui::Button("Exit", button_size)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        ImGui::End();
    }
    else if (gameState == GameState::End_Screen) {
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        ImGui::SetNextWindowPos(ImVec2((display_w - display_w / 2) * 0.5f, (display_h - display_h / 2) * 0.5f));
        ImGui::SetNextWindowSize(ImVec2((float)display_w / 2, (float)display_h / 2));
        ImGui::SetNextWindowBgAlpha(0.75f);

        ImGui::Begin("Game Over", nullptr, window_flags);
        ImVec2 button_size(150, 45);
        ImVec2 window_size = ImGui::GetWindowSize();
        float total_height = button_size.y * 3 + 80.0f; // 3 buttons + spacing
        float start_y = (window_size.y - total_height) * 0.5f;

        // Horizontal center based on window width
        float center_x = (window_size.x - button_size.x) * 0.5f;

        
        Team winner = app->whoWon();
        std::string winString = TeamToString(winner);
        std::string message = "Winning Team: " + winString;

        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", message.c_str());


        VulkanApplication* app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));

        ImGui::SetCursorPos(ImVec2(center_x, start_y));
        if (ImGui::Button("Rematch", button_size)) {
            app->cancelAI();
            gameState = in_Game;
            app->resetGameboard();
            
        }

        ImGui::SetCursorPos(ImVec2(center_x, start_y + button_size.y + 10.0f));
        if (ImGui::Button("Exit To Menu", button_size)) {
            app->cancelAI();
            gameState = Main_Menu;
        }
        ImGui::End();   
    }
    else if(gameState == in_Game){
        escape_menu(window, gameState);
    }
}

void ImGuiLayer::escape_menu(GLFWwindow* window, GameState& gameState) {
    if (escape_menu_active) {
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)display_w, (float)display_h);

        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        ImGui::SetNextWindowPos(ImVec2((display_w - display_w / 2) * 0.5f, (display_h - display_h / 2) * 0.5f));
        ImGui::SetNextWindowSize(ImVec2((float)display_w / 2, (float)display_h / 2));
        ImGui::SetNextWindowBgAlpha(0.75f);

        ImGui::Begin("Escape Menu", nullptr, window_flags);

        ImVec2 button_size(150, 45);
        ImVec2 window_size = ImGui::GetWindowSize();

        // Calculate vertical starting point to center buttons
        float total_height = button_size.y * 3 + 80.0f; // 3 buttons + spacing
        float start_y = (window_size.y - total_height) * 0.5f;

        // Horizontal center based on window width
        float center_x = (window_size.x - button_size.x) * 0.5f;

        VulkanApplication* app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));

        ImGui::SetCursorPos(ImVec2(center_x, start_y));
        if (ImGui::Button("Restart", button_size)) {
            app->cancelAI();
            app->resetGameboard();
            gameState = in_Game;
            escape_menu_active = false;
        }
        ImGui::SetCursorPos(ImVec2(center_x, start_y + button_size.y + 10.0f));
        if (ImGui::Button("Main Menu", button_size)) {
            app->cancelAI();
            app->resetGameboard();
            gameState = Main_Menu;
            escape_menu_active = false;
        }

        ImGui::SetCursorPos(ImVec2(center_x, start_y + 2 * (button_size.y + 10.0f) + 20.0f));
        if (ImGui::Button("Quit", button_size)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        ImGui::End();
    }

}