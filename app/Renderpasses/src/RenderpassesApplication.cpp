#include <RenderpassesApplication.hpp>
#include <stdexcept>

#include <helsinki/Renderer/Vulkan/VulkanRenderpassResources.hpp>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../RenderpassesConfig.hpp"

#include <chrono>

#define ROOT_PATH(x) (std::string(rp::RenderpassesConfig::RootPath) + std::string(x))

const std::string MODEL_PATH = ROOT_PATH("/data/models/viking_room.obj");
const std::string TEXTURE_PATH = ROOT_PATH("/data/textures/viking_room.png");

struct UniformBufferObject
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

namespace rp
{
    static void framebufferResizeCallback(GLFWwindow* window, int, int)
    {
        auto app = reinterpret_cast<RenderpassesApplication*>(glfwGetWindowUserPointer(window));
        app->notifyFramebufferResized();
    }

    RenderpassesApplication::RenderpassesApplication(
    ) :
        _window(nullptr),
        _instance(),
        _surface(_instance),
        _device(_instance, _surface),
        _swapChain(_device, _surface),
        _defaultRenderpassResources(_device),
        _postProcessRenderpassResources(_device),
        _commandPool(_device),
        _oneTimeCommandPool(_device),
        _syncContext(_device),
        _model(_device)
    {

    }

    void RenderpassesApplication::init(uint32_t width, uint32_t height, const char* title)
    {
        initWindow(width, height, title);
        initVulkan(title);
        mainLoop();
    }
    void RenderpassesApplication::run()
    {
        cleanup();
    }
    void RenderpassesApplication::mainLoop()
    {
        while (!glfwWindowShouldClose(_window))
        {
            glfwPollEvents();
            draw();
        }

        _device.waitIdle();
    }
    void RenderpassesApplication::cleanup()
    {
        _swapChain.destroy();

        _defaultRenderpassResources.destroy();
        _postProcessRenderpassResources.destroy();

        for (size_t i = 0; i < _uniformBuffers.size(); i++)
        {
            _uniformBuffers[i].destroy();
        }

        _model.destroy();

        _syncContext.destroy();
        _commandPool.destroy();
        _oneTimeCommandPool.destroy();
        _device.destroy();
        _surface.destroy();
        _instance.destroy();

        glfwDestroyWindow(_window);

        glfwTerminate();
    }
    void RenderpassesApplication::draw()
    {
        _syncContext.getFence(currentFrame).wait();

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(
            _device._device,
            _swapChain._swapChain,
            UINT64_MAX,
            _syncContext.getImageAvailableSemaphore(currentFrame)._semaphore,
            VK_NULL_HANDLE,
            &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        updateUniformBuffer(_uniformBuffers[currentFrame]);

        _syncContext.getFence(currentFrame).reset();

        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { _syncContext.getImageAvailableSemaphore(currentFrame)._semaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = { _syncContext.getRenderFinishedSemaphore(currentFrame)._semaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(_device._graphicsQueue._queue, 1, &submitInfo, _syncContext.getFence(currentFrame)._fence) != VK_SUCCESS)
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
    void RenderpassesApplication::initWindow(uint32_t width, uint32_t height, const char* title)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        _window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        glfwSetWindowUserPointer(_window, this);
        glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
    }
    void RenderpassesApplication::initVulkan(const char* title)
    {
        _instance.create(title);
        _surface.create(_window);
        _device.create();
        _swapChain.create(_useMultiSampling);

        _defaultRenderpassResources
            .create(
                _swapChain._swapChainImageFormat,
                _swapChain.findDepthFormat(_device._physicalDevice),
                _swapChain._swapChainExtent,
                _useMultiSampling);


        _postProcessRenderpassResources
            .create(
                _swapChain,
                false); // No multisampling for post process

        _commandPool.create();
        _oneTimeCommandPool.create();

        _model.create(_oneTimeCommandPool, MODEL_PATH, TEXTURE_PATH);
        createUniformBuffers();
        createCommandBuffers();
        _syncContext.create();

        // TODO: Update if needed???
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {            
            _defaultRenderpassResources._descriptorSet.update(i, _uniformBuffers[i], _model._texture);
        }
    }

    void RenderpassesApplication::createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            _uniformBuffers.emplace_back(_device);

            _uniformBuffers.back().create(bufferSize, sizeof(UniformBufferObject));
        }
    }

    void RenderpassesApplication::createCommandBuffers()
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


    void RenderpassesApplication::recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(_window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(_window, &width, &height);
            glfwWaitEvents();
        }

        _device.waitIdle();

        _defaultRenderpassResources.recreate(VkExtent2D{.width = (uint32_t)width, .height = (uint32_t)height});
        _postProcessRenderpassResources.recreate(VkExtent2D{ .width = (uint32_t)width, .height = (uint32_t)height });
    }

    void RenderpassesApplication::updateUniformBuffer(hl::VulkanUniformBuffer& uniformBuffer)
    {
        UniformBufferObject ubo{};
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), _swapChain._swapChainExtent.width / (float)_swapChain._swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        uniformBuffer.writeToBuffer(&ubo);
    }

    void RenderpassesApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
       /* _defaultRenderpassResources.recordCommandBuffer(
            commandBuffer, 
            imageIndex, 
            [&]() -> void 
            {
                {
                    static auto startTime = std::chrono::high_resolution_clock::now();

                    auto currentTime = std::chrono::high_resolution_clock::now();
                    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

                    auto model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

                    vkCmdPushConstants(
                        commandBuffer,
                        _defaultRenderpassResources._graphicsPipeline._pipelineLayout._pipelineLayout,
                        VK_SHADER_STAGE_VERTEX_BIT,
                        0,
                        sizeof(glm::mat4),
                        &model
                    );
                }

                _model.draw(
                    commandBuffer,
                    _defaultRenderpassResources._graphicsPipeline,
                    _defaultRenderpassResources._descriptorSet._descriptorSets[currentFrame]);
            });*/

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        // 1) SCENE PASS -> OFFSCREEN
        {
            VkRenderPassBeginInfo rpBegin{};
            rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            rpBegin.renderPass = _defaultRenderpassResources._renderpass._renderPass;
            rpBegin.framebuffer = _defaultRenderpassResources._framebuffers[currentFrame]._framebuffer;
            rpBegin.renderArea.offset = { 0, 0 };
            rpBegin.renderArea.extent = _defaultRenderpassResources._renderpassExtent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clearValues[1].depthStencil = { 1.0f, 0 };
            rpBegin.clearValueCount = (uint32_t)clearValues.size();
            rpBegin.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                _defaultRenderpassResources._graphicsPipeline._graphicsPipeline
            );

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)_defaultRenderpassResources._renderpassExtent.width;
            viewport.height = (float)_defaultRenderpassResources._renderpassExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = _defaultRenderpassResources._renderpassExtent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            static auto startTime = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float>(currentTime - startTime).count();
            glm::mat4 model = glm::rotate(glm::mat4(1.0f), time, glm::vec3(0, 0, 1));

            vkCmdPushConstants(
                commandBuffer,
                _defaultRenderpassResources._graphicsPipeline._pipelineLayout._pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(glm::mat4),
                &model
            );

            _model.draw(
                commandBuffer,
                _defaultRenderpassResources._graphicsPipeline,
                _defaultRenderpassResources._descriptorSet._descriptorSets[currentFrame]
            );

            vkCmdEndRenderPass(commandBuffer);
        }

        // 2) LAYOUT TRANSITION: Offscreen color -> Shader read
       // {
       //     VkImageMemoryBarrier barrier{};
       //     barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
       //     barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
       //     barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
       //     barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
       //     barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
       //     barrier.image = _useMultiSampling
       //         ? _defaultRenderpassResources._colorResolveImages[currentFrame]._image
       //         : _defaultRenderpassResources._colorImages[currentFrame]._image;
       // 
       //     barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
       //     barrier.subresourceRange.levelCount = 1;
       //     barrier.subresourceRange.layerCount = 1;
       // 
       //     vkCmdPipelineBarrier(
       //         commandBuffer,
       //         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
       //         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
       //         0,
       //         0, nullptr,
       //         0, nullptr,
       //         1, &barrier
       //     );
       // }

        // 3) POST-PROCESS PASS -> SWAPCHAIN
        {
            VkRenderPassBeginInfo rpBegin{};
            rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            rpBegin.renderPass = _postProcessRenderpassResources._renderpass._renderPass;
            rpBegin.framebuffer = _postProcessRenderpassResources._framebuffers[currentFrame]._framebuffer;
            rpBegin.renderArea.offset = { 0, 0 };
            rpBegin.renderArea.extent = _postProcessRenderpassResources._renderpassExtent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} }; // color
            clearValues[1].depthStencil = { 1.0f, 0 };           // depth = 1.0, stencil = 0
            rpBegin.clearValueCount = clearValues.size();
            rpBegin.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _postProcessRenderpassResources._graphicsPipeline._graphicsPipeline);
            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                _postProcessRenderpassResources._graphicsPipeline._pipelineLayout._pipelineLayout,
                0,
                1,
                &_postProcessRenderpassResources._descriptorSet._descriptorSets[currentFrame],
                0,
                nullptr);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0); // fullscreen triangle
            vkCmdEndRenderPass(commandBuffer);
        }

        vkEndCommandBuffer(commandBuffer);

    }
}