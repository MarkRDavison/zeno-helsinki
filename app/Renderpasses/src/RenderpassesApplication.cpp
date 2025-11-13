#include <RenderpassesApplication.hpp>
#include <stdexcept>
#include <iostream>

#include <helsinki/Renderer/Vulkan/VulkanRenderpassResources.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphPipelineResources.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphRenderpassResources.hpp>

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
        _commandPool(_device),
        _oneTimeCommandPool(_device),
        _syncContext(_device),
        _model(_device),
        _renderResourcesSystem(),
        _skyBoxTexture(_device)
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

        _renderGraph->destroy();
        delete _renderGraph;

        for (size_t i = 0; i < _uniformBuffers.size(); i++)
        {
            _uniformBuffers[i].destroy();
        }

        _model.destroy();
        _skyBoxTexture.destroy();

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
                        .format = "VK_FORMAT_B8G8R8A8_SRGB"
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
                        .name = "skybox_pipeline",
                        .shaderVert = ROOT_PATH("/data/shaders/skybox.vert"),
                        .shaderFrag = ROOT_PATH("/data/shaders/skybox.frag"),
                        .descriptorSets =
                        {
                            hl::DescriptorSetInfo
                            {
                                .name = "",
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
                                        .resource = "skybox_texture"
                                    }
                                }
                            }
                        },
                        .depthState =
                        {
                            .writeEnable = false,
                            .compareOp = VK_COMPARE_OP_LESS_OR_EQUAL
                        },
                        .rasterState = 
                        {
                            .cullMode = VK_CULL_MODE_NONE
                        },
                        .enableBlending = false
                    },
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
                        .vertexInputInfo = hl::VertexInputInfo
                        {
                            .attributes =
                            {
                                {
                                    .name = "inPosition",
                                    .format = hl::VertexAttributeFormat::Vec3,
                                    .location = 0,
                                    .offset = offsetof(hl::Vertex, pos)
                                },
                                {
                                    .name = "inColor",
                                    .format = hl::VertexAttributeFormat::Vec3,
                                    .location = 1,
                                    .offset = offsetof(hl::Vertex, color)
                                },
                                {
                                    .name = "inTexCoord",
                                    .format = hl::VertexAttributeFormat::Vec2,
                                    .location = 2,
                                    .offset = offsetof(hl::Vertex, texCoord)
                                },
                            },
                            .stride = sizeof(hl::Vertex)
                        },
                        .rasterState =
                        {
                            .cullMode = VK_CULL_MODE_BACK_BIT
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
                        .format = "VK_FORMAT_B8G8R8A8_SRGB"
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
                        .depthState =
                        {
                            .testEnable = false,
                            .writeEnable = false
                        },
                        .rasterState =
                        {
                            .cullMode = VK_CULL_MODE_NONE
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
                        .format = "VK_FORMAT_B8G8R8A8_SRGB"
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
                        .depthState =
                        {
                            .testEnable = false,
                            .writeEnable = false
                        },
                        .rasterState =
                        {
                            .cullMode = VK_CULL_MODE_NONE
                        },
                        .enableBlending = false
                    },
                    hl::PipelineInfo
                    {
                        .name = "ui",
                        .shaderVert = ROOT_PATH("/data/shaders/ui.vert"),
                        .shaderFrag = ROOT_PATH("/data/shaders/ui.frag"),
                        .descriptorSets = {},
                        .depthState =
                        {
                            .testEnable = false,
                            .writeEnable = false
                        },
                        .rasterState =
                        {
                            .cullMode = VK_CULL_MODE_NONE
                        },
                        .enableBlending = true
                    }
                }
            }
        };

        _commandPool.create();
        _oneTimeCommandPool.create();

        _model.create(_oneTimeCommandPool, MODEL_PATH, TEXTURE_PATH);
        _skyBoxTexture.create(_oneTimeCommandPool, 
            {
                ROOT_PATH("/data/textures/cratered-01-right.png"),
                ROOT_PATH("/data/textures/cratered-01-left.png"),
                ROOT_PATH("/data/textures/cratered-01-top.png"),
                ROOT_PATH("/data/textures/cratered-01-bottom.png"),
                ROOT_PATH("/data/textures/cratered-01-front.png"),
                ROOT_PATH("/data/textures/cratered-01-back.png")
                //ROOT_PATH("/data/textures/right.png"),
                //ROOT_PATH("/data/textures/left.png"),
                //ROOT_PATH("/data/textures/top.png"),
                //ROOT_PATH("/data/textures/bottom.png"),
                //ROOT_PATH("/data/textures/front.png"),
                //ROOT_PATH("/data/textures/back.png")
            });

        createUniformBuffers();
        createCommandBuffers();
        _syncContext.create();

        // TODO: BAD!
        std::vector<hl::VulkanUniformBuffer*> ubs = {
            &_uniformBuffers[0],
            &_uniformBuffers[1]
        };

        _renderResourcesSystem.addUniformBuffers("model_matrix_ubo", ubs);
        _renderResourcesSystem.addTexture("viking_texture", &_model._texture);
        _renderResourcesSystem.addTexture("skybox_texture", &_skyBoxTexture);

        _renderGraph = new hl::GeneratedRenderGraph(
            _device,
            _swapChain,
            renderpasses,
            _renderResourcesSystem);

        _renderGraph->updateAllDescriptorSets();
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
        
        _swapChain.destroy();
        _swapChain.create();

        _renderGraph->recreate((uint32_t)width, (uint32_t)height);

        _renderGraph->updateAllDescriptorSets();
    }

    void RenderpassesApplication::updateUniformBuffer(hl::VulkanUniformBuffer& uniformBuffer)
    {
        UniformBufferObject ubo{};
        ubo.view = glm::lookAt(
            glm::vec3(2.0f, 0.5f, -2.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));

        ubo.proj = glm::perspective(
            glm::radians(45.0f),
            _swapChain._swapChainExtent.width / (float)_swapChain._swapChainExtent.height,
            0.1f,
            10.0f);

        ubo.proj[1][1] *= -1;

        uniformBuffer.writeToBuffer(&ubo);
    }

    void RenderpassesApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        const auto lastRenderpassName = _renderGraph->getResources().back()->Name;


        for (auto& renderpass : _renderGraph->getResources())
        {
            VkRenderPassBeginInfo renderpassBegin{};
            renderpassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderpassBegin.renderPass = renderpass->getRenderPass();
            renderpassBegin.framebuffer = renderpass->getFramebuffer(
                lastRenderpassName == renderpass->Name
                    ? imageIndex
                    : currentFrame);
            renderpassBegin.renderArea.offset = { 0,0 };
            renderpassBegin.renderArea.extent = _swapChain._swapChainExtent; //TODO: FROM FRAMEBUFFER/RENDERPASS

            const auto& clearValues = renderpass->getClearValues();

            renderpassBegin.clearValueCount = (uint32_t)clearValues.size();
            renderpassBegin.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffer, &renderpassBegin, VK_SUBPASS_CONTENTS_INLINE);

            for (auto& pipeline : renderpass->getPipelines())
            {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());

                VkViewport viewport{};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = (float)_swapChain._swapChainExtent.width; //TODO: FROM FRAMEBUFFER/RENDERPASS
                viewport.height = (float)_swapChain._swapChainExtent.height; //TODO: FROM FRAMEBUFFER/RENDERPASS
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                VkRect2D scissor{};
                scissor.offset = { 0, 0 };
                scissor.extent = _swapChain._swapChainExtent;//TODO: FROM FRAMEBUFFER/RENDERPASS
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);


                if (pipeline->Name == "skybox_pipeline")
                {
                    auto descriptorSet = pipeline->getDescriptorSet(currentFrame);
                    vkCmdBindDescriptorSets(commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipeline->getPipelineLayout(),
                        0,
                        1,
                        &descriptorSet,
                        0,
                        nullptr);

                    vkCmdDraw(commandBuffer, 36, 1, 0, 0); // quad from 12 triangles
                }
                else if (pipeline->Name == "model_pipeline")
                {
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

                    vkCmdPushConstants(
                        commandBuffer,
                        pipeline->getPipelineLayout(),
                        VK_SHADER_STAGE_VERTEX_BIT,
                        0,
                        sizeof(glm::mat4),
                        &model
                    );

                    VkBuffer vertexBuffers[] = { _model._vertexBuffer._buffer };
                    VkDeviceSize offsets[] = { 0 };
                    vkCmdBindVertexBuffers(
                        commandBuffer,
                        0,
                        1,
                        vertexBuffers,
                        offsets);

                    vkCmdBindIndexBuffer(
                        commandBuffer,
                        _model._indexBuffer._buffer,
                        0,
                        VK_INDEX_TYPE_UINT32);

                    auto descriptorSet = pipeline->getDescriptorSet(currentFrame);
                    vkCmdBindDescriptorSets(
                        commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipeline->getPipelineLayout(),
                        0,
                        1,
                        &descriptorSet,
                        0,
                        nullptr);

                    vkCmdDrawIndexed(commandBuffer, _model.indexCount, 1, 0, 0, 0); // viking model
                }
                else if (pipeline->Name == "postprocess_pipeline")
                {
                    auto descriptorSet = pipeline->getDescriptorSet(currentFrame);
                    vkCmdBindDescriptorSets(commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipeline->getPipelineLayout(),
                        0,
                        1,
                        &descriptorSet,
                        0,
                        nullptr);

                    vkCmdDraw(commandBuffer, 3, 1, 0, 0); // fullscreen triangle
                }
                else if (pipeline->Name == "fullscreen_sample")
                {
                    auto descriptorSet = pipeline->getDescriptorSet(currentFrame);
                    vkCmdBindDescriptorSets(commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipeline->getPipelineLayout(),
                        0,
                        1,
                        &descriptorSet,
                        0,
                        nullptr);

                    vkCmdDraw(commandBuffer, 3, 1, 0, 0); // fullscreen triangle
                }
                else if (pipeline->Name == "ui")
                {
                    vkCmdDraw(commandBuffer, 3, 1, 0, 0); // halfscreen triangle
                }
                else
                {
                    throw std::runtime_error("TODO: HARD CODED DRAW FUNCTIONS");
                }
            }

            vkCmdEndRenderPass(commandBuffer);
        }

        vkEndCommandBuffer(commandBuffer);
    }
}