#include <RenderpassesApplication.hpp>
#include <stdexcept>
#include <iostream>
#include <format>
#include <chrono>
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphPipelineResources.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphRenderpassResources.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Events/ScrollEvent.hpp>
#include <helsinki/System/Events/EventDispatcher.hpp>
#include <helsinki/System/HelsinkiTracy.hpp>

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

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<RenderpassesApplication*>(glfwGetWindowUserPointer(window));
        app->notifyFramebufferResized(width, height);
    }
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto app = reinterpret_cast<RenderpassesApplication*>(glfwGetWindowUserPointer(window));
        if (action == GLFW_PRESS)
        {
            hl::KeyPressEvent event(key);
            app->sendEvent(event);
        }
        else if (action == GLFW_RELEASE)
        {
            hl::KeyReleaseEvent event(key);
            app->sendEvent(event);
        }
    }
    static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
    {
        auto app = reinterpret_cast<RenderpassesApplication*>(glfwGetWindowUserPointer(window));
        hl::ScrollEvent event((int)xOffset, (int)yOffset);
        app->sendEvent(event);
    }

    RenderpassesApplication::RenderpassesApplication(
        hl::EventBus& eventBus
    ) :
        _eventBus(eventBus),
        _window(nullptr),
        _instance(),
        _surface(_instance),
        _device(_instance, _surface),
        _swapChain(_device, _surface),
        _commandPool(_device),
        _oneTimeCommandPool(_device),
        _syncContext(_device)
    {
        _eventBus.AddListener(this);

        _camera = new hl::Camera(
            glm::vec3(2.0f, 0.5f, -2.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            135.0f,
            -5.0f);
    }
    RenderpassesApplication::~RenderpassesApplication()
    {
        delete _camera;
        _eventBus.RemoveListener(this);
    }

    void RenderpassesApplication::init(RenderpassConfig config)
    {
        _config = config;
        initWindow(_config.Width, _config.Height, _config.Title.c_str());
        initVulkan(_config.Title.c_str());
        mainLoop();
    }
    void RenderpassesApplication::run()
    {
        cleanup();
    }
    void RenderpassesApplication::notifyFramebufferResized(int width, int height)
    { 
        framebufferResized = true; 
        hl::WindowResizeEvent event(width, height);
        _eventBus.PublishEvent(event);
    }
    void RenderpassesApplication::sendEvent(const hl::Event& event)
    {
        _eventBus.PublishEvent(event);
    }

    void RenderpassesApplication::OnEvent(const hl::Event& event)
    {
        hl::EventDispatcher dispatcher(event);
        dispatcher.Dispatch<hl::KeyPressEvent>([this](const hl::KeyPressEvent& e)
            {
                if (e.GetKeyCode() == GLFW_KEY_A)
                {
                    _camera->processKeyboard(hl::CameraMovement::LEFT, 1.0f / 60.0f);
                }
                if (e.GetKeyCode() == GLFW_KEY_D)
                {
                    _camera->processKeyboard(hl::CameraMovement::RIGHT, 1.0f / 60.0f);
                }
                if (e.GetKeyCode() == GLFW_KEY_S)
                {
                    _camera->processKeyboard(hl::CameraMovement::BACKWARD, 1.0f / 60.0f);
                }
                if (e.GetKeyCode() == GLFW_KEY_W)
                {
                    _camera->processKeyboard(hl::CameraMovement::FORWARD, 1.0f / 60.0f);
                }
            });
        dispatcher.Dispatch<hl::KeyReleaseEvent>([this](const hl::KeyReleaseEvent& e)
            {

            });
        dispatcher.Dispatch<hl::ScrollEvent>([this](const hl::ScrollEvent& e)
            {
                const auto y = e.getY();
                if (y < 0)
                {
                    _camera->setZoom(_camera->getZoom() * 1.1f);
                }
                else
                {
                    _camera->setZoom(_camera->getZoom() / 1.1f);
                }
            });
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
            ZoneScopedN("MainLoop");
            auto now = std::chrono::steady_clock::now();
            const auto frameTime = now - start;
            start = now;

            accumulator += std::chrono::duration<float>(frameTime).count();
            statsAccumulator += std::chrono::duration<float>(frameTime).count();

            if (statsAccumulator >= 1.0f)
            {
                if (_config.DisplayFps)
                {
                    const auto title = std::format("{} - FPS: {} UPS: {}", _config.Title, fps, ups);
                    glfwSetWindowTitle(_window, title.c_str());
                }
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
        ZoneScopedN("Update");
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    void RenderpassesApplication::draw()
    {
        ZoneScopedN("Draw");

        {
            ZoneScopedN("Wait fence");
            _syncContext.getFence(currentFrame).wait();
        }

        VkResult result;
        uint32_t imageIndex;
        {
            ZoneScopedN("Acquire next image");
            result = vkAcquireNextImageKHR(
                _device._device,
                _swapChain._swapChain,
                UINT64_MAX,
                _syncContext.getImageAvailableSemaphore(currentFrame)._semaphore,
                VK_NULL_HANDLE,
                &imageIndex);
        }

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            ZoneScopedN("Recreate swapchain");
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        updateUniformBuffer(_modelMatrixHandle.Get()->getUniformBuffer(currentFrame));
        _syncContext.getFence(currentFrame).reset();

        recordCommandBuffer(_frameResources[currentFrame], imageIndex);

        VkSemaphore waitSemaphores[] = { _syncContext.getImageAvailableSemaphore(currentFrame)._semaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore signalSemaphores[] = { _syncContext.getRenderFinishedSemaphore(currentFrame)._semaphore };
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &_frameResources[currentFrame].primaryCmd;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        {
            ZoneScopedN("Submit render queue");
            CHECK_VK_RESULT(vkQueueSubmit(_device._graphicsQueue._queue, 1, &submitInfo, _syncContext.getFence(currentFrame)._fence));
        }

        VkSwapchainKHR swapChains[] = { _swapChain._swapChain };
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        {
            ZoneScopedN("Present queue");
            result = vkQueuePresentKHR(_device._presentQueue._queue, &presentInfo);
        }

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
        {
            ZoneScopedN("Recreate swapchain");
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
        ZoneScopedN("InitWindow");
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        _window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        glfwSetWindowUserPointer(_window, this);
        glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
        glfwSetKeyCallback(_window, keyCallback);
        glfwSetScrollCallback(_window, scrollCallback);
    }

    void RenderpassesApplication::initVulkan(const char* title)
    {
        ZoneScopedN("InitVulkan");
        {
            {
                ZoneScopedN("Create Instance");
                _instance.create(title);
            }
            {
                ZoneScopedN("Create Surface");
                _surface.create(_window);
            }
            {
                ZoneScopedN("Create Device");
                _device.create();
            }
            {
                ZoneScopedN("Create swapchain");
                _swapChain.create(_config.EnableVsync);
            }
        }

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
                .pipelineGroups =
                {
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
                    },
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
                .pipelineGroups =
                {
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
                .pipelineGroups =
                {
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
                .pipelineGroups =
                {
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
            }
        };

        {
            ZoneScopedN("GenerateRenderGraph");
            _renderGraph = new hl::GeneratedRenderGraph(
                _device,
                _swapChain,
                renderpasses,
                _resourceManager);
        }

        {
            ZoneScopedN("PoolBufferSyncContext");

            _commandPool.create();
            _oneTimeCommandPool.createTransferPool();

            createCommandBuffers();
            _syncContext.create();
        }

        hl::ResourceContext resourceContext
        {
            .device = &_device,
            .pool = &_oneTimeCommandPool, 
            .rootPath = rp::RenderpassesConfig::RootPath
        };

        {
            ZoneScopedN("LoadResources");
            _modelHandle = _resourceManager.Load<hl::BasicModelResource>(
                "viking_room",
                resourceContext);
            _resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
                "viking_room",
                resourceContext);
            _resourceManager.LoadAs<hl::CubemapTextureResource, hl::ImageSamplerResource>(
                "skybox_texture",
                resourceContext);
            _modelMatrixHandle = _resourceManager.Load<hl::UniformBufferResource>(
                "model_matrix_ubo",
                resourceContext,
                sizeof(UniformBufferObject),
                MAX_FRAMES_IN_FLIGHT);
        }

        {
            ZoneScopedN("updateAllOutputResources");
            _renderGraph->updateAllOutputResources();
        }

        {
            ZoneScopedN("updateAllDescriptorSets");
            _renderGraph->updateAllDescriptorSets();
        }
    }

    void RenderpassesApplication::createCommandBuffers()
    {
        _frameResources.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = _commandPool._commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)_frameResources.size();

        std::vector<VkCommandBuffer> perFrameCommandBuffers(MAX_FRAMES_IN_FLIGHT);

        CHECK_VK_RESULT(vkAllocateCommandBuffers(_device._device, &allocInfo, perFrameCommandBuffers.data()));

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            auto& frame = _frameResources[i];
            frame.primaryCmd = perFrameCommandBuffers[i];

            for (uint32_t layer = 0; layer < _renderGraph->getNumberLayers(); layer++)
            {
                const auto& renderpassesForLayer = _renderGraph->getSortedNodesByNameForLayer(layer);
                auto& secondaryCommandsForLayerAndGroups = frame.secondaryCmdsByLayerAndPipelineGroup[layer];

                secondaryCommandsForLayerAndGroups.resize(renderpassesForLayer.size());

                for (size_t rpIndex = 0; rpIndex < renderpassesForLayer.size(); ++rpIndex)
                {
                    const auto& renderpass = _renderGraph->getRenderpassByName(renderpassesForLayer[rpIndex]);
                    auto& secondaryCommandsForGroups = secondaryCommandsForLayerAndGroups[rpIndex];

                    secondaryCommandsForGroups.resize(renderpass->getPipelineGroups().size());

                    for (auto& secondaryCommand : secondaryCommandsForGroups)
                    {
                        VkCommandBufferAllocateInfo allocInfo{};
                        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                        allocInfo.commandPool = _commandPool._commandPool;
                        allocInfo.commandBufferCount = 1;

                        vkAllocateCommandBuffers(_device._device, &allocInfo, &secondaryCommand);
                    }
                }
            }
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
        _swapChain.create(_config.EnableVsync);

        _renderGraph->recreate((uint32_t)width, (uint32_t)height);

        _renderGraph->updateAllDescriptorSets();
    }

    void RenderpassesApplication::updateUniformBuffer(hl::VulkanUniformBuffer& uniformBuffer)
    {
        UniformBufferObject ubo{};

        ubo.view = _camera->getViewMatrix();
        ubo.proj = _camera->getProjectionMatrix(_swapChain._swapChainExtent.width / (float)_swapChain._swapChainExtent.height);

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

    void RenderpassesApplication::recordCommandBuffer(FrameResources& frame, uint32_t imageIndex)
    {
        const auto& lastRenderpassName = _renderGraph->getResources().back()->Name;

        CHECK_VK_RESULT(vkResetCommandBuffer(frame.primaryCmd, 0));
        for (auto& [_, secondaries] : frame.secondaryCmdsByLayerAndPipelineGroup)
        {
            for (auto& secondaryGroup : secondaries)
            {
                for (auto& secondary : secondaryGroup)
                {
                    CHECK_VK_RESULT(vkResetCommandBuffer(secondary, 0));
                }
            }
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        CHECK_VK_RESULT(vkBeginCommandBuffer(frame.primaryCmd, &beginInfo));

        for (uint32_t layer = 0; layer < _renderGraph->getNumberLayers(); ++layer)
        {
            ZoneScoped;
            ZoneNameF("record command buffers for layer %s", std::to_string(layer).c_str());

            assert(frame.secondaryCmdsByLayerAndPipelineGroup.at(layer).size() == _renderGraph->getSortedNodesByNameForLayer(layer).size());

            const auto& secondaryBuffersPerRenderpass = frame.secondaryCmdsByLayerAndPipelineGroup.at(layer);

            size_t renderpassIndex = 0;
            // FUTURE WORK: Multithreaded secondary command buffer recording
            //
            // Current design records all pipeline groups in each renderpass sequentially.
            // Potential optimization: record pipeline groups in parallel on worker threads.
            //
            // Plan:
            // 1. For each renderpass, if it has >1 pipeline group, dispatch each group to a worker thread.
            // 2. Each thread records its secondary command buffer independently.
            // 3. Wait for all threads to finish.
            // 4. Accumulate all secondary command buffers and call vkCmdExecuteCommands once to submit them to the primary command buffer.
            //
            // Notes:
            // - Only safe for independent pipeline groups within a renderpass (no ordering dependencies).
            //      - This should be fine since renderpasses in the same layer should be independant
            // - Avoid multithreading overhead if there is only one pipeline group.
            // - Later extension: consider parallel renderpasses if layers or renderpasses are independent.
            for (const auto& renderpassName : _renderGraph->getSortedNodesByNameForLayer(layer))
            {
                ZoneScoped;
                ZoneNameF("record command buffer for %s", renderpassName.c_str());

                const auto& renderpass = _renderGraph->getRenderpassByName(renderpassName);
                const auto& clearValues = renderpass->getClearValues();
                const auto& framebuffer = renderpass->getFramebuffer(lastRenderpassName == renderpass->Name ? imageIndex : currentFrame);

                VkRenderPassBeginInfo renderpassBegin
                {
                    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                    .renderPass = renderpass->getRenderPass(),
                    .framebuffer = framebuffer,
                    .renderArea =
                    {
                        .offset = { 0,0 },
                        .extent = getExtent(renderpass->getExtent(), _swapChain._swapChainExtent),
                    },
                    .clearValueCount = (uint32_t)clearValues.size(),
                    .pClearValues = clearValues.data()
                };

                vkCmdBeginRenderPass(frame.primaryCmd, &renderpassBegin, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

                VkCommandBufferInheritanceInfo inheritanceInfo{};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = renderpass->getRenderPass();
                inheritanceInfo.subpass = 0;
                inheritanceInfo.framebuffer = framebuffer;
                inheritanceInfo.occlusionQueryEnable = VK_FALSE;
                inheritanceInfo.queryFlags = 0;
                inheritanceInfo.pipelineStatistics = 0;

                VkCommandBufferBeginInfo secondaryBeginInfo{};
                secondaryBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                secondaryBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                secondaryBeginInfo.pInheritanceInfo = &inheritanceInfo;

                const auto& secondaryBuffersForGroup = secondaryBuffersPerRenderpass[renderpassIndex];

                size_t pipelineGroupIndex = 0;
                for (const auto& pg : renderpass->getPipelineGroups())
                {
                    // TODO: Here create a job and put it into a worker thread queue to be picked up
                    const auto secondaryBuffer = secondaryBuffersForGroup[pipelineGroupIndex];

                    CHECK_VK_RESULT(vkBeginCommandBuffer(secondaryBuffer, &secondaryBeginInfo));
                    for (const auto& p : pg)
                    {
                        vkCmdBindPipeline(secondaryBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p->getPipeline());
                        {   //  TODO: This can be wasteful if nothing has changed.
                            VkViewport viewport
                            {
                                .x = 0.0f,
                                .y = 0.0f,
                                .width = (float)getExtent(renderpass->getExtent(), _swapChain._swapChainExtent).width,
                                .height = (float)getExtent(renderpass->getExtent(), _swapChain._swapChainExtent).height,
                                .minDepth = 0.0f,
                                .maxDepth = 1.0f
                            };
                            vkCmdSetViewport(secondaryBuffer, 0, 1, &viewport);

                            VkRect2D scissor
                            {
                                .offset = { 0, 0 } ,
                                .extent = getExtent(renderpass->getExtent(), _swapChain._swapChainExtent)
                            };
                            vkCmdSetScissor(secondaryBuffer, 0, 1, &scissor);
                        }

                        renderPipelineDraw(secondaryBuffer, p);
                    }

                    CHECK_VK_RESULT(vkEndCommandBuffer(secondaryBuffer));

                    // TODO: Join/wait for all the thread jobs to complete here
                    // TODO: Accumulate all the secondary buffers and call vkCmdExecuteCommands just before vkCmdEndRenderPass
                    vkCmdExecuteCommands(frame.primaryCmd, 1, &secondaryBuffer);

                    pipelineGroupIndex++;
                }

                vkCmdEndRenderPass(frame.primaryCmd);

                ++renderpassIndex;
            }
        }        

        CHECK_VK_RESULT(vkEndCommandBuffer(frame.primaryCmd));
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
            glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

            vkCmdPushConstants(
                commandBuffer,
                pipeline->getPipelineLayout(),
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(glm::mat4),
                &model
            );

            auto modelResource = _modelHandle.Get();

            VkBuffer vertexBuffers[] = { modelResource->getVertexBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(
                commandBuffer,
                0,
                1,
                vertexBuffers,
                offsets);

            vkCmdBindIndexBuffer(
                commandBuffer,
                modelResource->getIndexBuffer(),
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

            vkCmdDrawIndexed(commandBuffer, modelResource->getIndexCount(), 1, 0, 0, 0); // viking model
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