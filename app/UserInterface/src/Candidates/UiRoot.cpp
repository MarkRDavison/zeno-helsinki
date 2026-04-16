#include <Candidates/UiRoot.hpp>
#include <helsinki/Engine/Input/InputManager.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <helsinki/UserInterface/Widgets/RectWidget.hpp>
#include <helsinki/UserInterface/Widgets/ContainerWidget.hpp>
#include <iostream>

constexpr auto MAX_UI_VERTEXES = 1024;

namespace hl
{
	UiRoot::UiRoot(
		InputManager& inputManager
	) :
		_inputManager(inputManager)
	{

	}
	UiRoot::~UiRoot()
	{

	}

	void UiRoot::initialise(VulkanDevice& device)
	{
		const auto size = _inputManager.getWindowSize();
		_width = size.x;
		_height = size.y;

		for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			_mappedBuffers.emplace_back(device);
			_mappedBuffers.back().create(sizeof(hl::VertexUi) * MAX_UI_VERTEXES, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		}

		WidgetID trueRoot = _scene.CreateRootWidget<ContainerWidget>();
		LayoutNode* trueRootNode = _scene.Layouts.Get(_scene.GetWidget(trueRoot)->GetLayoutID());
		trueRootNode->Style.LayoutType = ELayoutType::Horizontal;
		trueRootNode->Style.WidthMode = ESizingMode::Flex;
		trueRootNode->Style.HeightMode = ESizingMode::Flex;

		{
			WidgetID mainPanel = _scene.CreateWidget<ContainerWidget>(trueRoot);
			LayoutNode* mainPanelNode = _scene.Layouts.Get(_scene.GetWidget(mainPanel)->GetLayoutID());
			mainPanelNode->Style.LayoutType = ELayoutType::Vertical;
			mainPanelNode->Style.WidthMode = ESizingMode::Flex;
			mainPanelNode->Style.HeightMode = ESizingMode::Flex;

			WidgetID mainPanelTopRect = _scene.CreateWidget<RectWidget>(mainPanel, hl::Vec4f(0.4f, 0.4f, 0.4f, 1.0f));
			LayoutNode* mainPanelTopRectNode = _scene.Layouts.Get(_scene.GetWidget(mainPanelTopRect)->GetLayoutID());
			mainPanelTopRectNode->Style.LayoutType = ELayoutType::Vertical;
			mainPanelTopRectNode->Style.WidthMode = ESizingMode::Flex;
			mainPanelTopRectNode->Style.HeightMode = ESizingMode::Fixed;
			mainPanelTopRectNode->Style.FixedHeight = 64.0f;

			WidgetID mainPanelContentRect = _scene.CreateWidget<ContainerWidget>(mainPanel);
			LayoutNode* mainPanelContentRectNode = _scene.Layouts.Get(_scene.GetWidget(mainPanelContentRect)->GetLayoutID());
			mainPanelContentRectNode->Style.LayoutType = ELayoutType::Vertical;
			mainPanelContentRectNode->Style.WidthMode = ESizingMode::Flex;
			mainPanelContentRectNode->Style.HeightMode = ESizingMode::Flex;

			WidgetID mainPanelBottomRect = _scene.CreateWidget<RectWidget>(mainPanel, hl::Vec4f(0.4f, 0.4f, 0.4f, 1.0f));
			LayoutNode* mainPanelBottomRectNode = _scene.Layouts.Get(_scene.GetWidget(mainPanelBottomRect)->GetLayoutID());
			mainPanelBottomRectNode->Style.LayoutType = ELayoutType::Vertical;
			mainPanelBottomRectNode->Style.WidthMode = ESizingMode::Flex;
			mainPanelBottomRectNode->Style.HeightMode = ESizingMode::Fixed;
			mainPanelBottomRectNode->Style.FixedHeight = 96.0f;
		}

		{
			WidgetID sidePanel = _scene.CreateWidget<ContainerWidget>(trueRoot);
			LayoutNode* sidePanelNode = _scene.Layouts.Get(_scene.GetWidget(sidePanel)->GetLayoutID());
			sidePanelNode->Style.LayoutType = ELayoutType::Vertical;
			sidePanelNode->Style.WidthMode = ESizingMode::Fixed;
			sidePanelNode->Style.FixedWidth = 240.0f;
			sidePanelNode->Style.HeightMode = ESizingMode::Flex;

			WidgetID sidePanelRect = _scene.CreateWidget<RectWidget>(sidePanel, hl::Vec4f(0.7f, 0.7f, 0.7f, 1.0f));
			LayoutNode* sidePanelRectNode = _scene.Layouts.Get(_scene.GetWidget(sidePanelRect)->GetLayoutID());
			sidePanelRectNode->Style.LayoutType = ELayoutType::Vertical;
			sidePanelRectNode->Style.WidthMode = ESizingMode::Flex;
			sidePanelRectNode->Style.HeightMode = ESizingMode::Flex;
		}

		_scene.UpdateLayout({ (float)_width, (float)_height });
	}

	static void addQuad(std::vector<hl::VertexUi>& vertices, float x, float y, float w, float h, glm::vec3 colour)
	{
		vertices.push_back(hl::VertexUi{ .pos = { x, y }, .color = colour });
		vertices.push_back(hl::VertexUi{ .pos = { x + w, y }, .color = colour });
		vertices.push_back(hl::VertexUi{ .pos = { x + w, y + h }, .color = colour });

		vertices.push_back(hl::VertexUi{ .pos = { x, y }, .color = colour });
		vertices.push_back(hl::VertexUi{ .pos = { x + w, y + h }, .color = colour });
		vertices.push_back(hl::VertexUi{ .pos = { x, y + h }, .color = colour });
	}

	void UiRoot::update(float delta)
	{
		//auto be = ButtonEvent();
		//auto ie = InputEvent{ .Device = EDeviceID::Keyboard, .Payload = be };
		//_scene.DispatchInputEvent(ie);

		// TODO: Handle events rather than polling... ^^
		
		_drawlist = {};
		_scene.Render(_drawlist); // TODO: Do this here or in updateGpuResources???
	}
	void UiRoot::updateGpuResources(uint32_t currentFrame)
	{
		// _drawlist = {};
		// _scene.Render(_drawlist); // TODO: Do this here or in update???

		_vertices.clear();
		for (const auto& cmd : _drawlist.Commands)
		{
			if (Holds<DrawCmd::RectCmd>(cmd.Payload))
			{
				const auto& rectCmd = Get<DrawCmd::RectCmd>(cmd.Payload);
				RenderFillRoundedRect(rectCmd.Rect, rectCmd.Rounding, rectCmd.Color, cmd.Transform);
			}
		}

		assert(_vertices.size() <= MAX_UI_VERTEXES);

		const auto size = getDataSize();

		if (size > 0)
		{
			_mappedBuffers[currentFrame].write(getData(), size);
		}
	}
	void UiRoot::draw(PipelineDrawData& pdd) const
	{
		const auto vertexCount = getVertexCount();

		if (vertexCount > 0)
		{
			VkBuffer vertexBuffers[] = { _mappedBuffers[pdd.currentFrame].getBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(
				pdd.commandBuffer,
				0,
				1,
				vertexBuffers,
				offsets);

			auto descriptorSet = pdd.pipeline->getDescriptorSet(pdd.currentFrame);
			vkCmdBindDescriptorSets(
				pdd.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pdd.pipeline->getPipelineLayout(),
				0,
				1,
				&descriptorSet,
				0,
				nullptr);

			vkCmdDraw(pdd.commandBuffer, vertexCount, 1, 0, 0);
		}
	}
	void UiRoot::destroy()
	{
		for (auto& mb : _mappedBuffers)
		{
			mb.destroy();
		}

		_mappedBuffers.clear();
	}
	
	void UiRoot::OnEvent(const hl::Event& event)
	{
		if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
		{
			_width = wre->GetWidth();
			_height = wre->GetHeight();

			_scene.UpdateLayout({ (float)_width, (float)_height });
		}
	}

	void* UiRoot::getData() const
	{
		return (void*)_vertices.data();
	}
	size_t UiRoot::getDataSize() const
	{
		return _vertices.size() * sizeof(hl::VertexUi);
	}
	size_t UiRoot::getVertexCount() const
	{
		return _vertices.size();
	}

	void UiRoot::RenderFillRoundedRect(Rectf rect, CornerRounding rounding, Vec4f colour, Mat3f transform)
	{
		addQuad(_vertices, rect.Left(), rect.Top(), rect.Width(), rect.Height(), { colour.x(), colour.y(), colour.z()});
	}
}