# Gather source files
file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(VulkanTest ${SOURCES})

# ---- Include directories ----
target_include_directories(VulkanTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Vulkan
    ${CMAKE_CURRENT_SOURCE_DIR}/Rendering_Helpers
    ${CMAKE_CURRENT_SOURCE_DIR}/Game_Logic
    ${CMAKE_SOURCE_DIR}/imgUI_CPP

    $ENV{VULKAN_SDK}/Include
    "C:/Visual Studio/Libraries/glfw-3.4.bin.WIN64/include"
    "C:/Visual Studio/Libraries/glm"
    "C:/Visual Studio/Libraries/stb-master/stb-master"
    "C:/Visual Studio/Libraries/tinyobjloader-release/tinyobjloader-release"
    "C:/Visual Studio/Libraries/imgui"
    "C:/Visual Studio/Libraries/imgui/backends"
)

# ---- Link directories ----
link_directories(
    "C:/Visual Studio/Libraries/glfw-3.4.bin.WIN64/lib-vc2022"
    $ENV{VULKAN_SDK}/Lib
)

# ---- Link libraries ----
target_link_libraries(VulkanTest PRIVATE
    "C:/Visual Studio/Libraries/glfw-3.4.bin.WIN64/lib-vc2022/glfw3.lib"
    Vulkan::Vulkan
)

# ---- Compiler definitions ----
target_compile_definitions(VulkanTest PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    WIN32
    _CONSOLE
)

add_custom_command(
    TARGET VulkanTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:VulkanTest>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/textures
        $<TARGET_FILE_DIR:VulkanTest>/textures
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/models
        $<TARGET_FILE_DIR:VulkanTest>/models
)

