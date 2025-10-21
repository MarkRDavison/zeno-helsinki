#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include <fstream>
#include <array>
#include <unordered_map>

#include <helsinki/Renderer/Vulkan/VulkanInstance.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSurface.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSwapChain.hpp>
#include <helsinki/Renderer/Vulkan/VulkanImage.hpp>
#include <helsinki/Renderer/Vulkan/VulkanRenderpass.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>
#include <helsinki/Renderer/Vulkan/VulkanTexture.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorSetLayout.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorPool.hpp>
#include <helsinki/Renderer/Vulkan/VulkanUniformBuffer.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorSet.hpp>
#include <helsinki/Renderer/Vulkan/VulkanGraphicsPipeline.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <chrono>

#include "ExampleConfig.hpp"

#define ROOT_PATHED(x) (std::string(ex::ExampleConfig::RootPath) + std::string(x))

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600; 

const std::string MODEL_PATH = ROOT_PATHED("/data/models/viking_room.obj");
const std::string TEXTURE_PATH = ROOT_PATHED("/data/textures/viking_room.png");

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class HelloTriangleApplication
{
public:
    HelloTriangleApplication() :
        _instance(),
        _surface(_instance),
        _device(_instance, _surface),
        _texture(_device),
        _swapChain(_device, _surface),
        _renderpass(_device, _swapChain),
        _commandPool(_device),
        _descriptorSetLayout(_device),
        _descriptorPool(_device),
        _vertexBuffer(_device),
        _indexBuffer(_device),
        _descriptorSet(_device),
        _graphicsPipeline(_device)
    {

    }

    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;

    hl::VulkanInstance _instance;
    hl::VulkanSurface _surface;
    hl::VulkanDevice _device;
    hl::VulkanSwapChain _swapChain;
    hl::VulkanRenderpass _renderpass;
    hl::VulkanCommandPool _commandPool;
    hl::VulkanTexture _texture;
    hl::VulkanDescriptorSetLayout _descriptorSetLayout;
    hl::VulkanDescriptorPool _descriptorPool;
    hl::VulkanDescriptorSet _descriptorSet;
    hl::VulkanGraphicsPipeline _graphicsPipeline;

    hl::VulkanBuffer _vertexBuffer;
    hl::VulkanBuffer _indexBuffer;

    std::vector<hl::VulkanUniformBuffer> _uniformBuffers;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    uint32_t indexCount = 0;

    bool framebufferResized = false;

    void initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int, int)
    {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void initVulkan()
    {
        _instance.create("Hello Triangle.");
        _surface.create(window);
        _device.create();
        _swapChain.create();
        _renderpass.create();

        _swapChain.createFramebuffers(_renderpass);
        // TODO: Need to read this in from shader adjacent files?
        _descriptorSetLayout.create({
            VkDescriptorSetLayoutBinding
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = nullptr
            },
            VkDescriptorSetLayoutBinding
            {
                .binding = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = nullptr
            }
            });
        _descriptorPool.create();

        _graphicsPipeline.create(
            std::string(ROOT_PATHED("/data/shaders/vert.spv")),
            std::string(ROOT_PATHED("/data/shaders/frag.spv")),
            _renderpass,
            _descriptorSetLayout);
        _commandPool.create();
        _texture.create(_commandPool, TEXTURE_PATH);
        
        createModelBuffers();
        createUniformBuffers();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            drawFrame();
        }

        _device.waitIdle();
    }

    void cleanup()
    {
        _swapChain.destroy();

        _graphicsPipeline.destroy();
        
        _renderpass.destroy();        

        for (size_t i = 0; i < _uniformBuffers.size(); i++)
        {
            _uniformBuffers[i].destroy();
        }

        _descriptorPool.destroy();
        _descriptorSetLayout.destroy();

        _texture.destroy();

        _indexBuffer.destroy();
        _vertexBuffer.destroy();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(_device._device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(_device._device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(_device._device, inFlightFences[i], nullptr);
        }

        _commandPool.destroy();
        _device.destroy();
        _surface.destroy();
        _instance.destroy();

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        _device.waitIdle();

        _swapChain.destroy();
        _swapChain.create();

        _swapChain.createFramebuffers(_renderpass);
    }

    std::pair<std::vector<hl::Vertex>, std::vector<uint32_t>> loadModel()
    {
        std::vector<hl::Vertex> vertices;
        std::vector<uint32_t> indices;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;
        std::string warn;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
        {
            throw std::runtime_error(err);
        }

        std::unordered_map<hl::Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                hl::Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }

        return { vertices, indices };
    }

    void createModelBuffers()
    {
        auto [vertices, indices] = loadModel();

        indexCount =(uint32_t) indices.size();

        {
            VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

            hl::VulkanBuffer stagingBuffer(_device);
            stagingBuffer.create(
                bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            stagingBuffer.mapMemory(
                vertices.data());

            _vertexBuffer.create(
                bufferSize,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            stagingBuffer.copyToBuffer(
                _commandPool,
                bufferSize,
                _vertexBuffer);

            stagingBuffer.destroy();
        }
        {
            VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

            hl::VulkanBuffer stagingBuffer(_device);

            stagingBuffer.create(
                bufferSize, 
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            stagingBuffer.mapMemory(
                indices.data());

            _indexBuffer.create(
                bufferSize, 
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            stagingBuffer.copyToBuffer(
                _commandPool,
                bufferSize,
                _indexBuffer);

            stagingBuffer.destroy();
        }
    }

    void createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            _uniformBuffers.emplace_back(_device);

            _uniformBuffers.back().create(bufferSize, sizeof(UniformBufferObject));
        }
    }

    void createDescriptorSets()
    {
        _descriptorSet.create(_descriptorPool, _descriptorSetLayout);

         // SEPARATE creating them and updating them

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            _descriptorSet.update(i, _uniformBuffers[i], _texture);
        }
    }

    void createCommandBuffers()
    {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = _commandPool._commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(_device._device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _renderpass._renderPass;
        renderPassInfo.framebuffer = _swapChain._swapChainFramebuffers[imageIndex]._framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = _swapChain._swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline._graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)_swapChain._swapChainExtent.width;
        viewport.height = (float)_swapChain._swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = _swapChain._swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = { _vertexBuffer._buffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, _indexBuffer._buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline._pipelineLayout._pipelineLayout, 0, 1, &_descriptorSet._descriptorSets[currentFrame], 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void createSyncObjects()
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(_device._device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(_device._device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(_device._device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), _swapChain._swapChainExtent.width / (float)_swapChain._swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        _uniformBuffers[currentImage].writeToBuffer(&ubo);
    }

    void drawFrame()
    {
        vkWaitForFences(_device._device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(_device._device, _swapChain._swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        updateUniformBuffer(currentFrame);

        vkResetFences(_device._device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(_device._graphicsQueue._queue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { _swapChain._swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(_device._presentQueue._queue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
        {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    VkShaderModule createShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(_device._device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }
};

int main()
{
    HelloTriangleApplication app;

    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}