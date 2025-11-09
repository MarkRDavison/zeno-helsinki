#include <RenderpassesApplication.hpp>
#include <stdexcept>
#include <iostream>

#include <helsinki/Renderer/Vulkan/VulkanRenderpassResources.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>

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
        _defaultRenderpassResources("SCENE", _device),
        _postProcessRenderpassResources("POST", _device),
        _uiRenderpassResources("UI", _device),
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

        _uiRenderpassResources.destroy();
        _postProcessRenderpassResources.destroy();
        _defaultRenderpassResources.destroy();

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
        _swapChain.create();

        std::vector<hl::RenderpassInfo> renderpasses = 
        {
            hl::RenderpassInfo
            {
                .name = "scene_pass",
                .useMultiSampling = true,
                .inputs = {},
                .outputs = 
                {
                    hl::ResourceInfo
                    {
                        .name = "scene_color",
                        .type = hl::ResourceType::Color,
                        .format = "VK_FORMAT_R8G8B8A8_UNORM"
                    },
                    hl::ResourceInfo
                    {
                        .name = "scene_depth",
                        .type = hl::ResourceType::Depth,
                        .format = "VK_FORMAT_D32_SFLOAT"
                    }
                },
                .pipelines = 
                {
                    hl::PipelineInfo
                    {
                        .name = "model_pipeline",
                        .shaderVert = ROOT_PATH("/data/shaders/triangle.vert"),
                        .shaderFrag = ROOT_PATH("/data/shaders/triangle.frag"),
                        .descriptorSets =
                        {
                            hl::DescriptorSetInfo
                            {
                                .name = "model_uniforms",
                                .bindings = 
                                {
                                    hl::DescriptorBinding
                                    {
                                        .binding = 0,
                                        .type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
                                        .stage = "VERTEX",
                                        .resource = "model_matrix_ubo"
                                    },
                                    hl::DescriptorBinding
                                    {
                                        .binding = 1,
                                        .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                        .stage = "FRAGMENT",
                                        .resource = "viking_texture"
                                    }
                                }
                            }
                        },
                        .enableBlending = false
                    }
                }
            },
            hl::RenderpassInfo
            {
                .name = "postprocess_pass",
                .useMultiSampling = false,
                .inputs = { "scene_color" },
                .outputs = 
                {
                    hl::ResourceInfo
                    {
                        .name = "post_color",
                        .type = hl::ResourceType::Color,
                        .format = "VK_FORMAT_R8G8B8A8_UNORM"
                    },
                    hl::ResourceInfo
                    {
                        .name = "post_depth",
                        .type = hl::ResourceType::Depth,
                        .format = "VK_FORMAT_D32_SFLOAT"
                    }
                },
                .pipelines =
                {
                    hl::PipelineInfo
                    {
                        .name = "postprocess_pipeline",
                        .shaderVert = ROOT_PATH("/data/shaders/post_process.vert"),
                        .shaderFrag = ROOT_PATH("/data/shaders/post_process.frag"),
                        .descriptorSets =
                        {
                            hl::DescriptorSetInfo
                            {
                                .name = "input_sampler",
                                .bindings =
                                {
                                    hl::DescriptorBinding
                                    {
                                        .binding = 0,
                                        .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                        .stage = "FRAGMENT",
                                        .resource = "scene_color"
                                    }
                                }
                            }
                        },
                        .enableBlending = false
                    }
                }
            },
            hl::RenderpassInfo
            {
                .name = "ui_pass",
                .useMultiSampling = false,
                .inputs = { "post_color" },
                .outputs =
                {
                    hl::ResourceInfo
                    {
                        .name = "swapchain_color",
                        .type = hl::ResourceType::Color,
                        .format = "VK_FORMAT_B8G8R8A8_UNORM"
                    },
                    hl::ResourceInfo
                    {
                        .name = "post_depth",
                        .type = hl::ResourceType::Depth,
                        .format = "VK_FORMAT_D32_SFLOAT"
                    }
                },
                .pipelines =
                {
                    hl::PipelineInfo
                    {
                        .name = "fullscreen_sample",
                        .shaderVert = ROOT_PATH("/data/shaders/fullscreen_sample.vert"),
                        .shaderFrag = ROOT_PATH("/data/shaders/fullscreen_sample.frag"),
                        .descriptorSets =
                        {
                            hl::DescriptorSetInfo
                            {
                                .name = "fullscreen_sample_input",
                                .bindings =
                                {
                                    hl::DescriptorBinding
                                    {
                                        .binding = 0,
                                        .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                        .stage = "FRAGMENT",
                                        .resource = "post_color"
                                    }
                                }
                            }
                        },
                        .enableBlending = false
                    },
                    hl::PipelineInfo
                    {
                        .name = "ui",
                        .shaderVert = ROOT_PATH("/data/shaders/ui.vert"),
                        .shaderFrag = ROOT_PATH("/data/shaders/ui.frag"),
                        .descriptorSets = {},
                        .enableBlending = true
                    }
                }
            }
        };



        _defaultRenderpassResources
            .create(
                _swapChain._swapChainImageFormat,
                _swapChain.findDepthFormat(_device._physicalDevice),
                _swapChain._swapChainExtent,
                _useMultiSampling);

        _postProcessRenderpassResources
            .createPostProcess(
                _swapChain._swapChainImageFormat,
                _swapChain.findDepthFormat(_device._physicalDevice),
                _swapChain._swapChainExtent,
                false); // No multisampling for post process

        _uiRenderpassResources
            .createUi(
                _swapChain,
                false); // No multisampling for ui


        {
            auto generatedRenderpassResources = hl::RenderGraph::create(
                renderpasses, 
                _device,
                _swapChain._swapChainExtent.width,
                _swapChain._swapChainExtent.height,
                (uint32_t)_swapChain._swapChainImages.size());

            hl::RenderGraph::destroy(generatedRenderpassResources);
        }



        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (_useMultiSampling)
            {
                _postProcessRenderpassResources._graphicPipelines[0]._descriptorSet
                    .updatePostProcess(
                        i, 
                        _defaultRenderpassResources._colorResolveImages[i], 
                        _postProcessRenderpassResources._outputSampler);
            }
            else
            {
                _postProcessRenderpassResources._graphicPipelines[0]._descriptorSet
                    .updatePostProcess(
                        i, 
                        _defaultRenderpassResources._colorImages[i], 
                        _postProcessRenderpassResources._outputSampler);
            }

            // no multisampling in post process, so we always know where we sample from
            _uiRenderpassResources._graphicPipelines[0]._descriptorSet
                .updatePostProcess(
                    i,
                    _postProcessRenderpassResources._colorImages[i],
                    _uiRenderpassResources._outputSampler);
        }

        _commandPool.create();
        _oneTimeCommandPool.create();

        _model.create(_oneTimeCommandPool, MODEL_PATH, TEXTURE_PATH);

        createUniformBuffers();
        createCommandBuffers();
        _syncContext.create();

        // TODO: Update if needed???
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {            
            _defaultRenderpassResources._graphicPipelines[0]._descriptorSet.update(i, _uniformBuffers[i], _model._texture);
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
        _uiRenderpassResources.recreate(VkExtent2D{ .width = (uint32_t)width, .height = (uint32_t)height });

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (_useMultiSampling)
            {
                _postProcessRenderpassResources._graphicPipelines[0]._descriptorSet
                    .updatePostProcess(
                        i, 
                        _defaultRenderpassResources._colorResolveImages[i],
                        _postProcessRenderpassResources._outputSampler);
            }
            else
            {
                _postProcessRenderpassResources._graphicPipelines[0]._descriptorSet
                    .updatePostProcess(
                        i, 
                        _defaultRenderpassResources._colorImages[i], 
                        _postProcessRenderpassResources._outputSampler);
            }

            // Dont need to update descriptor set for second pipeline, has no ubo or samplers
            _uiRenderpassResources._graphicPipelines[0]._descriptorSet
                .updatePostProcess(
                    i, 
                    _postProcessRenderpassResources._colorImages[i],
                    _uiRenderpassResources._outputSampler);
        }
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
                _defaultRenderpassResources._graphicPipelines[0]._graphicsPipeline._graphicsPipeline
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
                _defaultRenderpassResources._graphicPipelines[0]._graphicsPipeline._pipelineLayout._pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(glm::mat4),
                &model
            );

            _model.draw(
                commandBuffer,
                _defaultRenderpassResources._graphicPipelines[0]._graphicsPipeline,
                _defaultRenderpassResources._graphicPipelines[0]._descriptorSet._descriptorSets[currentFrame]
            );

            vkCmdEndRenderPass(commandBuffer);
        }

        // 2) POST-PROCESS PASS -> OFFSCREEN 2
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

            vkCmdBindPipeline(commandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                _postProcessRenderpassResources._graphicPipelines[0]._graphicsPipeline._graphicsPipeline);
            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                _postProcessRenderpassResources._graphicPipelines[0]._graphicsPipeline._pipelineLayout._pipelineLayout,
                0,
                1,
                &_postProcessRenderpassResources._graphicPipelines[0]._descriptorSet._descriptorSets[currentFrame],
                0,
                nullptr);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0); // fullscreen triangle
            vkCmdEndRenderPass(commandBuffer);
        } 

        // 3) UI PASS -> SWAPCHAIN
        {
            VkRenderPassBeginInfo rpBegin{};
            rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            rpBegin.renderPass = _uiRenderpassResources._renderpass._renderPass;
            // TODO: imageIndex vs currentFrame, this depends on whether it is writing to the swapchain or not....
            rpBegin.framebuffer = _uiRenderpassResources._framebuffers[imageIndex]._framebuffer; 
            rpBegin.renderArea.offset = { 0, 0 };
            rpBegin.renderArea.extent = _uiRenderpassResources._renderpassExtent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} }; // color
            clearValues[1].depthStencil = { 1.0f, 0 };           // depth = 1.0, stencil = 0
            rpBegin.clearValueCount = clearValues.size();
            rpBegin.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

            {
                vkCmdBindPipeline(commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    _uiRenderpassResources._graphicPipelines[0]._graphicsPipeline._graphicsPipeline);
                vkCmdBindDescriptorSets(commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    _uiRenderpassResources._graphicPipelines[0]._graphicsPipeline._pipelineLayout._pipelineLayout,
                    0,
                    1,
                    &_uiRenderpassResources._graphicPipelines[0]._descriptorSet._descriptorSets[currentFrame],
                    0,
                    nullptr);

                vkCmdDraw(commandBuffer, 3, 1, 0, 0); // fullscreen triangle
            }

            {
                vkCmdBindPipeline(commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    _uiRenderpassResources._graphicPipelines[1]._graphicsPipeline._graphicsPipeline);

                vkCmdDraw(commandBuffer, 3, 1, 0, 0); // halfscreen triangle
            }

            vkCmdEndRenderPass(commandBuffer);
        }

        vkEndCommandBuffer(commandBuffer);

    }
}