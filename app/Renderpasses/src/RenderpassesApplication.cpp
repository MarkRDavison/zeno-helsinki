#include <RenderpassesApplication.hpp>
#include <stdexcept>
#include <iostream>
#include <format>
#include <chrono>
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <helsinki/Renderer/Vulkan/VulkanRenderpassResources.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphPipelineResources.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphRenderpassResources.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>

#include "../RenderpassesConfig.hpp"

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
        _title = title;
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
        const float delta = 1.0f / 60.0f;
        float accumulator = 0.0f;
        float statsAccumulator = 0.0f;
        unsigned int fps = 0;
        unsigned int ups = 0;

        auto start = std::chrono::steady_clock::now();

        while (!glfwWindowShouldClose(_window))
        {
            auto now = std::chrono::steady_clock::now();
            const auto frameTime = now - start;
            start = now;

            accumulator += std::chrono::duration<float>(frameTime).count();
            statsAccumulator += std::chrono::duration<float>(frameTime).count();

            if (statsAccumulator >= 1.0f)
            {
                // TODO: config like multisampling...
                const auto title = std::format("{} - FPS: {} UPS: {}", _title, fps, ups);
                
                glfwSetWindowTitle(_window, title.c_str());
                statsAccumulator -= 1.0f;
                fps = 0;
                ups = 0;
            }

            while (accumulator >= delta)
            {
                glfwPollEvents();

                update(delta);
                ups++;

                accumulator -= delta;
            }

            draw();
            fps++;
        }

        _device.waitIdle();
    }

    void RenderpassesApplication::cleanup()
    {
        _swapChain.destroy();

        _renderGraph->destroy();
        delete _renderGraph;

        _model.destroy();
        _skyBoxTexture.destroy();

        _resourceManager.UnloadAll();

        _syncContext.destroy();
        _commandPool.destroy();
        _oneTimeCommandPool.destroy();
        _device.destroy();
        _surface.destroy();
        _instance.destroy();

        glfwDestroyWindow(_window);

        glfwTerminate();
    }

    void RenderpassesApplication::update(float delta)
    {

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

        updateUniformBuffer(_modelMatrixHandle.Get()->getUniformBuffer(currentFrame));
        _syncContext.getFence(currentFrame).reset();
        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSemaphore waitSemaphores[] = { _syncContext.getImageAvailableSemaphore(currentFrame)._semaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore signalSemaphores[] = { _syncContext.getRenderFinishedSemaphore(currentFrame)._semaphore };
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        CHECK_VK_RESULT(vkQueueSubmit(_device._graphicsQueue._queue, 1, &submitInfo, _syncContext.getFence(currentFrame)._fence));

        VkSwapchainKHR swapChains[] = { _swapChain._swapChain };
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
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
                                        .resource = "viking_room"
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
                .inputs = {},
                .outputs =
                {
                    hl::ResourceInfo
                    {
                        .name = "ui_color",
                        .type = hl::ResourceType::Color,
                        .format = "VK_FORMAT_B8G8R8A8_SRGB",
                        .clear = VkClearValue{.color = {{ 0.0f, 0.0f, 0.0f, 0.0f }}}
                    }
                },
                .pipelines =
                {
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
                        .enableBlending = true // keep blending for UI elements
                    }
                }
            },
            hl::RenderpassInfo
            {
                .name = "composite_pass",
                .useMultiSampling = false,
                .inputs = { "post_color", "ui_color" },
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
                        .name = "composite_pipeline",
                        .shaderVert = ROOT_PATH("/data/shaders/fullscreen_sample.vert"),
                        .shaderFrag = ROOT_PATH("/data/shaders/composite.frag"),
                        .descriptorSets =
                        {
                            hl::DescriptorSetInfo
                            {
                                .name = "composite_inputs",
                                .bindings =
                                {
                                    hl::DescriptorBinding
                                    {
                                        .binding = 0,
                                        .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                        .stage = "FRAGMENT",
                                        .resource = "post_color"
                                    },
                                    hl::DescriptorBinding
                                    {
                                        .binding = 1,
                                        .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                        .stage = "FRAGMENT",
                                        .resource = "ui_color"
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
                        .enableBlending = true
                    }
                }
            }
        };

        _commandPool.create();
        _oneTimeCommandPool.createTransferPool();

        _model.create(_oneTimeCommandPool, MODEL_PATH, TEXTURE_PATH);
        _skyBoxTexture.create(_oneTimeCommandPool, 
            {
                ROOT_PATH("/data/textures/skybox_texture-right.png"),
                ROOT_PATH("/data/textures/skybox_texture-left.png"),
                ROOT_PATH("/data/textures/skybox_texture-top.png"),
                ROOT_PATH("/data/textures/skybox_texture-bottom.png"),
                ROOT_PATH("/data/textures/skybox_texture-front.png"),
                ROOT_PATH("/data/textures/skybox_texture-back.png")
            });

        createCommandBuffers();
        _syncContext.create();

        hl::ResourceContext resourceContext
        {
            .device = &_device,
            .pool = &_oneTimeCommandPool, 
            .rootPath = rp::RenderpassesConfig::RootPath
        };

        _resourceManager.Load<hl::TextureResource>(
            "viking_room",
            resourceContext);
        _resourceManager.LoadAs<hl::CubemapTextureResource, hl::TextureResource>(
            "skybox_texture",
            resourceContext);

        _modelMatrixHandle = _resourceManager.Load<hl::UniformBufferResource>(
            "model_matrix_ubo", 
            resourceContext, 
            sizeof(UniformBufferObject), 
            MAX_FRAMES_IN_FLIGHT);

        _renderGraph = new hl::GeneratedRenderGraph(
            _device,
            _swapChain,
            renderpasses,
            _renderResourcesSystem,
            _resourceManager);

        _renderGraph->updateAllDescriptorSets();
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

    VkExtent2D getExtent(VkExtent2D framebufferExtent, VkExtent2D swapchainExtent)
    {
        if (framebufferExtent.width == 0 || framebufferExtent.height == 0)
        {
            return swapchainExtent;
        }

        return framebufferExtent;
    }

    void RenderpassesApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        const auto& lastRenderpassName = _renderGraph->getResources().back()->Name;

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
            renderpassBegin.renderArea.extent = getExtent(renderpass->getExtent(), _swapChain._swapChainExtent);

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
                viewport.width = (float)getExtent(renderpass->getExtent(), _swapChain._swapChainExtent).width;
                viewport.height = (float)getExtent(renderpass->getExtent(), _swapChain._swapChainExtent).height;
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                VkRect2D scissor{};
                scissor.offset = { 0, 0 };
                scissor.extent = getExtent(renderpass->getExtent(), _swapChain._swapChainExtent);
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                renderPipelineDraw(commandBuffer, pipeline);
            }

            vkCmdEndRenderPass(commandBuffer);
        }

        vkEndCommandBuffer(commandBuffer);
    }


    void RenderpassesApplication::renderPipelineDraw(VkCommandBuffer commandBuffer, hl::VulkanRenderGraphPipelineResources* pipeline)
    {
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
        else if (pipeline->Name == "composite_pipeline")
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
            vkCmdDraw(commandBuffer, 3, 1, 0, 0); // halfscreen triangle
        }
        else
        {
            throw std::runtime_error("TODO: HARD CODED DRAW FUNCTIONS");
        }
    }
}