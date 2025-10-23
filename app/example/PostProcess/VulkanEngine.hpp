#pragma once

#include "vk_types.hpp"
#include "vkinit.hpp"
#include "vk_images.hpp"
#include "DeletionQueue.hpp"
#include "vk_descriptors.hpp"
#include "vk_pipelines.hpp"
#include <VkBootstrap.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../ExampleConfig.hpp"

#define ROOT_PATH(x) (std::string(ex::ExampleConfig::RootPath) + std::string(x))

constexpr bool bUseValidationLayers = true;

constexpr unsigned int FRAME_OVERLAP = 2;

struct ComputePushConstants
{
    glm::vec4 data1;
    glm::vec4 data2;
    glm::vec4 data3;
    glm::vec4 data4;
};

struct ComputeEffect
{
    const char* name;

    VkPipeline pipeline;
    VkPipelineLayout layout;

    ComputePushConstants data;
};


class VulkanEngine
{
public:
    void run();

    void initWindow();

    static void framebufferResizeCallback(GLFWwindow* window, int, int);

    void init();


    void init_background_pipelines();

    void init_pipelines();

    void init_descriptors();

    void init_vulkan();

    void init_swapchain();

    void init_commands();

    void init_sync_structures();

    void mainLoop();

    void draw_background(VkCommandBuffer cmd);


    void drawFrame();

    void cleanup();

    void create_swapchain(uint32_t width, uint32_t height);

    void destroy_swapchain();

    void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

private:

    void init_imgui();
    void draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView);

private:
    GLFWwindow* window{ nullptr };
    bool framebufferResized = false;

    bool _isInitialized{ false };
    int _frameNumber{ 0 };
    VkExtent2D _windowExtent{ 1024 , 768 };

    VmaAllocator _allocator;

    VkInstance _instance;// Vulkan library handle
    VkDebugUtilsMessengerEXT _debug_messenger;// Vulkan debug output handle
    VkPhysicalDevice _chosenGPU;// GPU chosen as the default device
    VkDevice _device; // Vulkan device for commands
    VkSurfaceKHR _surface;// Vulkan window surface


    VkSwapchainKHR _swapchain;
    VkFormat _swapchainImageFormat;

    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;
    VkExtent2D _swapchainExtent;


    FrameData _frames[FRAME_OVERLAP];

    FrameData& get_current_frame() { return _frames[_frameNumber % FRAME_OVERLAP]; };

    VkQueue _graphicsQueue;
    uint32_t _graphicsQueueFamily;


    DeletionQueue _mainDeletionQueue;

    AllocatedImage _drawImage;
    VkExtent2D _drawExtent;
    DescriptorAllocator globalDescriptorAllocator;

    VkDescriptorSet _drawImageDescriptors;
    VkDescriptorSetLayout _drawImageDescriptorLayout;

    VkPipeline _gradientPipeline;
    VkPipelineLayout _gradientPipelineLayout;

    VkFence _immFence;
    VkCommandBuffer _immCommandBuffer;
    VkCommandPool _immCommandPool;

    std::vector<ComputeEffect> backgroundEffects;
    int currentBackgroundEffect{ 0 };

};