#include "VulkanApplication.h"

void VulkanApplication::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    VulkanApplication* app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
    if (app->gameState == GameState::in_Game) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            glm::vec3 rotation = app->Gameboard.getBoardRotation();

            switch (key) {
            case GLFW_KEY_W:
                rotation.x += 3.0f;
                break;
            case GLFW_KEY_S:
                rotation.x -= 3.0f;
                break;
            case GLFW_KEY_A:
                rotation.y += 3.0f;
                break;
            case GLFW_KEY_D:
                rotation.y -= 3.0f;
                break;
            default:
                break; // No action, so return early
            }

            app->Gameboard.setBoardRotation(rotation);
        }
    }
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_ESCAPE) {
            if (app->imgui.get_escape_menu_state()) {
                app->imgui.set_escape_menu_state(false);
            }
            else {
                app->imgui.set_escape_menu_state(true);
            }
            
        }
    }



}

void VulkanApplication::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {  
   VulkanApplication* app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));  
   if (app->gameState == GameState::in_Game) {  
       if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)  
           app->RayCast_Select_Object(window);  

       if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {  
           app->isRightMouseButtonPressed = true;  
           double xpos, ypos;  
           glfwGetCursorPos(window, &xpos, &ypos);  
           app->lastMousePos.x = static_cast<float>(xpos);  
           app->lastMousePos.y = static_cast<float>(ypos);  
       }  
       else if (action == GLFW_RELEASE) {  
           app->isRightMouseButtonPressed = false;  
       }  
   }  
}

void VulkanApplication::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    VulkanApplication* app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
    if (app->gameState == GameState::in_Game) {
        // Change the camera X based on scroll direction
        float zoomSpeed = 0.5f;
        glm::vec3 direction = glm::normalize(app->cameraPos - glm::vec3(0.0f, 0.0f, 0.0f));

        app->cameraPos -= direction * static_cast<float>(yoffset) * zoomSpeed;

        //clamp distance
        float distance = glm::length(app->cameraPos);
        distance = glm::clamp(distance, 1.0f, 10.0f);

        // normalize value
        app->cameraPos = glm::normalize(app->cameraPos) * distance;
    }
}
