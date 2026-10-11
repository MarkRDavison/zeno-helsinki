#include <helsinki/Imgui/Context.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandBuffer.hpp>
#include <helsinki/Renderer/Vulkan/VulkanQueue.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <cassert>

namespace hl::imgui
{

	static void checkVkResult(VkResult err)
	{
		CHECK_VK_RESULT(err);
	}

	Context::Context() = default;

	Context::~Context()
	{
		shutdown();
	}

	void Context::init(Engine& engine)
	{
		assert(!_initialized);
		_engine = &engine;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForVulkan(engine.window(), true);

		auto& device = engine.device();
		auto& swapChain = engine.swapChain();
		const auto queueFamilies = VulkanQueue::findQueueFamilies(
			device.physicalDevice(),
			device.surface().handle());
		assert(queueFamilies.graphicsFamily.has_value());

		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = 1000;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1000;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		CHECK_VK_RESULT(vkCreateDescriptorPool(device.handle(), &poolInfo, nullptr, &_descriptorPool));

		_colorFormat = swapChain.format();

		VkPipelineRenderingCreateInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachmentFormats = &_colorFormat;

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = engine.instance().handle();
		initInfo.PhysicalDevice = device.physicalDevice();
		initInfo.Device = device.handle();
		initInfo.QueueFamily = queueFamilies.graphicsFamily.value();
		initInfo.Queue = device.graphicsQueue()._queue;
		initInfo.DescriptorPool = _descriptorPool;
		initInfo.MinImageCount = static_cast<uint32_t>(swapChain.images().size());
		initInfo.ImageCount = static_cast<uint32_t>(swapChain.images().size());
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.UseDynamicRendering = true;
		initInfo.PipelineRenderingCreateInfo = renderingInfo;
		initInfo.CheckVkResultFn = checkVkResult;
		ImGui_ImplVulkan_Init(&initInfo);
		ImGui_ImplVulkan_CreateFontsTexture();

		_initialized = true;
		engine.setDebugOverlay({
			.beginFrame = [this] { beginFrame(); },
			.record = [this](VkCommandBuffer commandBuffer, uint32_t imageIndex) { record(commandBuffer, imageIndex); },
			.onSwapchainRecreated = [this] { onSwapchainRecreated(); },
			.shutdown = [this] { shutdown(); },
		});
	}

	void Context::shutdown()
	{
		if (!_initialized)
		{
			return;
		}

		_initialized = false;
		if (_engine != nullptr)
		{
			_engine->device().waitIdle();
		}
		destroyVulkan();
		_engine = nullptr;
	}

	void Context::destroyVulkan()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		if (_engine != nullptr && _descriptorPool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(_engine->device().handle(), _descriptorPool, nullptr);
			_descriptorPool = VK_NULL_HANDLE;
		}
	}

	void Context::setBuildUi(std::function<void()> buildUi)
	{
		_buildUi = std::move(buildUi);
	}

	bool Context::wantCaptureMouse()
	{
		if (ImGui::GetCurrentContext() == nullptr)
		{
			return false;
		}
		return ImGui::GetIO().WantCaptureMouse;
	}

	bool Context::wantCaptureKeyboard()
	{
		if (ImGui::GetCurrentContext() == nullptr)
		{
			return false;
		}
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	void Context::beginFrame()
	{
		if (!_initialized)
		{
			return;
		}

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		if (_buildUi)
		{
			_buildUi();
		}
	}

	void Context::record(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		if (!_initialized)
		{
			return;
		}

		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();
		if (drawData == nullptr || drawData->CmdListsCount == 0)
		{
			return;
		}

		auto& device = _engine->device();
		auto& swapChain = _engine->swapChain();
		assert(imageIndex < swapChain.images().size());
		assert(imageIndex < swapChain.imageViews().size());
		assert(device.cmdBeginRendering() != nullptr);
		assert(device.cmdEndRendering() != nullptr);

		VulkanCommandBuffer::presentToColorAttachment(
			device,
			commandBuffer,
			swapChain.images()[imageIndex]);

		VkRenderingAttachmentInfo color{};
		color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		color.imageView = swapChain.imageViews()[imageIndex];
		color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		color.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		VkRenderingInfo rendering{};
		rendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		rendering.renderArea.extent = swapChain.extent();
		rendering.layerCount = 1;
		rendering.colorAttachmentCount = 1;
		rendering.pColorAttachments = &color;
		device.cmdBeginRendering()(commandBuffer, &rendering);
		ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
		device.cmdEndRendering()(commandBuffer);

		VulkanCommandBuffer::colorAttachmentToPresent(
			device,
			commandBuffer,
			swapChain.images()[imageIndex]);
	}

	void Context::onSwapchainRecreated()
	{
		if (!_initialized)
		{
			return;
		}

		auto& swapChain = _engine->swapChain();
		_colorFormat = swapChain.format();
		ImGui_ImplVulkan_SetMinImageCount(static_cast<uint32_t>(swapChain.images().size()));
	}

}
