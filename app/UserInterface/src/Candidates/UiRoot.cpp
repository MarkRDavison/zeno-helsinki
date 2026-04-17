#include <Candidates/UiRoot.hpp>
#include <helsinki/Engine/Input/InputManager.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <helsinki/UserInterface/Widgets/RectWidget.hpp>
#include <helsinki/UserInterface/Widgets/ContainerWidget.hpp>
#include <helsinki/UserInterface/Widgets/ButtonBaseWidget.hpp>
#include <helsinki/UserInterface/Widgets/TextWidget.hpp>
#include <helsinki/System/Events/MouseEvents.hpp>
#include <iostream>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>

constexpr auto MAX_UI_VERTEXES = 1024;
constexpr auto MAX_UI_TEXTURES = 64;

namespace hl
{
	UiRoot::UiRoot(
		InputManager& inputManager,
		ResourceManager& resourceManager
	) :
		_inputManager(inputManager),
		_scene(resourceManager)
	{

	}
	UiRoot::~UiRoot()
	{

	}

	void UiRoot::initialise(VulkanDevice& device, GeneratedRenderGraph& renderGraph, ResourceManager& resourceManager)
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
			sidePanelNode->Style.FixedWidth = 320.0f;
			sidePanelNode->Style.HeightMode = ESizingMode::Flex;

			WidgetID sidePanelRect = _scene.CreateWidget<RectWidget>(sidePanel, hl::Vec4f(0.7f, 0.7f, 0.7f, 1.0f));
			LayoutNode* sidePanelRectNode = _scene.Layouts.Get(_scene.GetWidget(sidePanelRect)->GetLayoutID());
			sidePanelRectNode->Style.LayoutType = ELayoutType::Vertical;
			sidePanelRectNode->Style.WidthMode = ESizingMode::Flex;
			sidePanelRectNode->Style.HeightMode = ESizingMode::Flex;
			sidePanelRectNode->Style.Padding = Edges{ 16.0f };
			sidePanelRectNode->Style.ChildAlign = EAlignment::TopCenter;

			WidgetID buttonBase = _scene.CreateWidget<ButtonBaseWidget>(sidePanelRect, hl::Vec4f(0.0f, 0.0f, 1.0f, 1.0f));
			LayoutNode* buttonBaseNode = _scene.Layouts.Get(_scene.GetWidget(buttonBase)->GetLayoutID());
			buttonBaseNode->Style.LayoutType = ELayoutType::Vertical;
			buttonBaseNode->Style.WidthMode = ESizingMode::Fixed;
			buttonBaseNode->Style.FixedWidth = 256.0f;
			buttonBaseNode->Style.HeightMode = ESizingMode::Fixed;
			buttonBaseNode->Style.FixedHeight = 64.0f;
			buttonBaseNode->Style.Padding = Edges{ 8.0f };
			buttonBaseNode->Style.StopsHitTests = true;
			buttonBaseNode->Style.ChildAlign = EAlignment::Center;
			

			_scene.GetWidget<ButtonBaseWidget>(buttonBase)->OnClick = 
				[]() -> void 
				{
					std::cout << "Button is clicked!!!!" << std::endl;
				};

			WidgetID textContent = _scene.CreateWidget<TextWidget>(buttonBase);
			LayoutNode* textContentNode = _scene.Layouts.Get(_scene.GetWidget(textContent)->GetLayoutID());
			textContentNode->Style.WidthMode = ESizingMode::Flex;
			textContentNode->Style.HeightMode = ESizingMode::Flex;
			//textContentNode->Style.PositionMode = EPositioningMode::Anchored;
			//textContentNode->Style.Anchor = Anchor::Center();
			_scene.GetWidget<TextWidget>(textContent)->setText(_scene, "Click me!", "roboto");
		}

		_scene.UpdateLayout({ (float)_width, (float)_height });

		{   // TODO: Need to do this maybe dynamically? But not every frame...
			auto info = resourceManager
				.GetResource<hl::ImageSamplerResource>(
					"white")
				->getDescriptorInfo(0); 
			
			auto text = resourceManager
				.GetResourceAs<hl::TextureResource, hl::ImageSamplerResource>(
					"roboto")
				->getDescriptorInfo(0);

			std::vector<VkDescriptorImageInfo> imageInfos;

			while (imageInfos.size() < MAX_UI_TEXTURES)
			{
				imageInfos.push_back(
					VkDescriptorImageInfo
					{
						.sampler = info.first,
						.imageView = info.second,
						.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
					});
			}
			imageInfos[1].sampler = text.first;
			imageInfos[1].imageView = text.second;

			for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
			{
				auto descriptorSet = renderGraph.getDescriptorSet("ui_renderpass", "ui_pipeline", i);

				auto descriptorWrite = VkWriteDescriptorSet
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = descriptorSet,
					.dstBinding = 1,
					.dstArrayElement = 0, // TODO: WIth this offset you can add one at a time etc...
					.descriptorCount = static_cast<uint32_t>(imageInfos.size()),
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.pImageInfo = imageInfos.data()
				};

				vkUpdateDescriptorSets(
					device._device,
					1,
					&descriptorWrite,
					0,
					nullptr);
			}
		}
	}

	static void addQuad(std::vector<hl::VertexUi>& vertices, float x, float y, float w, float h, glm::vec3 colour)
	{
		vertices.push_back(hl::VertexUi{ .pos = { x, y }, .texCoord = { 0.0f, 0.0f }, .color = colour, .renderType = 0 });
		vertices.push_back(hl::VertexUi{ .pos = { x + w, y }, .texCoord = { 1.0f, 0.0f }, .color = colour, .renderType = 0 });
		vertices.push_back(hl::VertexUi{ .pos = { x + w, y + h }, .texCoord = { 1.0f, 1.0f }, .color = colour, .renderType = 0 });

		vertices.push_back(hl::VertexUi{ .pos = { x, y }, .texCoord = { 0.0f, 0.0f }, .color = colour, .renderType = 0 });
		vertices.push_back(hl::VertexUi{ .pos = { x + w, y + h }, .texCoord = { 1.0f, 1.0f }, .color = colour, .renderType = 0 });
		vertices.push_back(hl::VertexUi{ .pos = { x, y + h }, .texCoord = { 0.0f, 1.0f }, .color = colour, .renderType = 0 });
	}

	void UiRoot::update(float delta)
	{
		_drawlist = {};
		_scene.Render(_drawlist);
	}
	void UiRoot::updateGpuResources(uint32_t currentFrame)
	{
		_vertices.clear();

		for (const auto& cmd : _drawlist.Commands)
		{
			if (Holds<DrawCmd::RectCmd>(cmd.Payload))
			{
				const auto& rectCmd = Get<DrawCmd::RectCmd>(cmd.Payload);
				RenderFillRoundedRect(rectCmd.Rect, rectCmd.Rounding, rectCmd.Color, cmd.Transform);
			}
			else if (Holds<DrawCmd::TextCmd>(cmd.Payload))
			{
				const auto& textCmd = Get<DrawCmd::TextCmd>(cmd.Payload);
				{
					for (const auto& v : textCmd.PosTexVertices)
					{
						_vertices.push_back(hl::VertexUi
						{ 
							.pos = { 
								v.x() + textCmd.Offset.x(),
								v.y() + textCmd.Offset.y()
							}, 
							.texCoord = {
								v.z(),
								v.w()
							},
							.color = {
								textCmd.Color.x(), 
								textCmd.Color.y(),
								textCmd.Color.z()
							}, 
							.renderType = 1
						});
					}
				}
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
			// TODO: Need to record clipping and not draw everything in one call? :(
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
		const auto buttonToName =
			[](int _button) -> std::string
			{
				switch (_button)
				{
				case 0:
					return "Left";
				case 1:
					return "Right";
				default:
					return "Unknown";
				}
			};
		if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
		{
			_width = wre->GetWidth();
			_height = wre->GetHeight();

			_scene.UpdateLayout({ (float)_width, (float)_height });
		}
		else if (auto ke = dynamic_cast<const hl::MouseButtonPressEvent*>(&event))
		{
			_scene.DispatchInputEvent(InputEvent
				{
					.Device = EDeviceID::Mouse,
					.Payload = ButtonEvent
					{
						.Button = (EButtonID)(ke->GetKeyCode() + 1),
						.Pressed = true,
						.Released = false,
						.Held = false
					}
				});
		}
		else if (auto ke = dynamic_cast<const hl::MouseButtonReleaseEvent*>(&event))
		{
			_scene.DispatchInputEvent(InputEvent
				{
					.Device = EDeviceID::Mouse,
					.Payload = ButtonEvent
					{
						.Button = (EButtonID)(ke->GetKeyCode() + 1),
						.Pressed = false,
						.Released = true,
						.Held = false
					}
				});
		}
		else if (auto ke = dynamic_cast<const hl::MousePositionEvent*>(&event))
		{
			_scene.DispatchInputEvent(InputEvent
				{
					.Device = EDeviceID::Mouse,
					.Payload = PointerEvent
					{
						.Position = Vec2f{ (float)ke->getX(), (float)ke->getY() },
						.Type = EPointerType::Mouse
					}
				});
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