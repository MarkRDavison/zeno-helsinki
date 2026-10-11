#pragma once

#include <helsinki/System/Utils/NonCopyable.hpp>
#include <functional>
#include <vulkan/vulkan.h>

namespace hl
{
	class Engine;
}

namespace hl::imgui
{

	class Context : public hl::NonCopyable
	{
	public:
		Context();
		~Context() override;

		void init(Engine& engine);
		void shutdown();
		void setBuildUi(std::function<void()> buildUi);

		static bool wantCaptureMouse();
		static bool wantCaptureKeyboard();

	private:
		void beginFrame();
		void record(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void onSwapchainRecreated();
		void destroyVulkan();

		Engine* _engine{ nullptr };
		std::function<void()> _buildUi;
		VkDescriptorPool _descriptorPool{ VK_NULL_HANDLE };
		VkFormat _colorFormat{ VK_FORMAT_UNDEFINED };
		bool _initialized{ false };
	};

}
