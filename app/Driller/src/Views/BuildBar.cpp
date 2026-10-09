#include <Views/BuildBar.hpp>
#include <Services/PrototypeService.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Theme.hpp>
#include <helsinki/Ui/Widget.hpp>
#include <algorithm>
#include <format>
#include <limits>

namespace drl
{

	namespace
	{
		constexpr float kTexWhite = 0.0f;
		constexpr float kTexRoboto = 1.0f;
		constexpr unsigned kBarFontSize = 18;

		class SceneFontTypeface : public hl::ui::ITypeface
		{
		public:
			explicit SceneFontTypeface(hl::FontResource* font) : _font(font) {}

			glm::vec2 layoutText(
				std::string_view text,
				unsigned fontSize,
				std::vector<hl::ui::GlyphVertex>& out) const override
			{
				out.clear();
				if (_font == nullptr)
				{
					return { 0.0f, 0.0f };
				}

				const auto generated = _font->generateTextVertexes(std::string(text), fontSize);
				if (generated.empty())
				{
					return { 0.0f, 0.0f };
				}

				glm::vec2 minPos{ std::numeric_limits<float>::max() };
				glm::vec2 maxPos{ std::numeric_limits<float>::lowest() };
				for (const auto& v : generated)
				{
					minPos = glm::min(minPos, v.pos);
					maxPos = glm::max(maxPos, v.pos);
				}

				out.reserve(generated.size());
				for (const auto& v : generated)
				{
					out.push_back(hl::ui::GlyphVertex{ .pos = v.pos - minPos, .uv = v.texCoord });
				}

				return maxPos - minPos;
			}

		private:
			hl::FontResource* _font = nullptr;
		};

		class BatchPaint : public hl::ui::IPaint
		{
		public:
			explicit BatchPaint(hl::UiBatch& batch) : _batch(&batch) {}

			void fill(const hl::ui::Box& box, glm::vec4 color) override
			{
				_batch->addQuad(box, color, glm::vec4{ 0.0f, 0.0f, 1.0f, 1.0f }, kTexWhite);
			}

			void sprite(const hl::ui::Box& box, glm::vec4 uvRect, glm::vec3 color) override
			{
				const auto texCoords = glm::vec4(
					uvRect.x,
					uvRect.y,
					uvRect.z - uvRect.x,
					uvRect.w - uvRect.y);
				_batch->addQuad(box, glm::vec4{ color, 1.0f }, texCoords, kTexWhite);
			}

			void glyphs(
				const std::vector<hl::ui::GlyphVertex>& verts,
				glm::vec2 origin,
				glm::vec3 color) override
			{
				glyphs(verts, origin, glm::vec4{ color, 1.0f });
			}

			void glyphs(
				const std::vector<hl::ui::GlyphVertex>& verts,
				glm::vec2 origin,
				glm::vec4 color) override
			{
				std::vector<hl::Vertex22D> converted;
				converted.reserve(verts.size());
				for (const auto& v : verts)
				{
					converted.push_back(hl::Vertex22D{ .pos = v.pos, .texCoord = v.uv });
				}

				_batch->addGlyphs(converted, origin, color, kTexRoboto);
			}

		private:
			hl::UiBatch* _batch = nullptr;
		};
	}

	void BuildBar::initialise(
		hl::FontResource* font,
		const BuildingPrototypeService& buildingPrototypes,
		IUiService& ui)
	{
		_ui = &ui;
		_typeface = std::make_unique<SceneFontTypeface>(font);
		_root = std::make_unique<hl::ui::Node>();
		_root->setFillParent();

		_panel = std::make_unique<hl::ui::Panel>(_root->addChild());
		_panel->color = { 0.08f, 0.09f, 0.12f };
		_panel->opacity = 0.92f;
		_panel->hitTestEnabled = true;

		for (const std::string& name : buildingPrototypes.registeredNames())
		{
			const BuildingPrototype& prototype =
				buildingPrototypes.getPrototype(prototypeIdFromName(name));
			auto button = std::make_unique<hl::ui::Button>(
				_panel->node().addChild(),
				*_typeface);
			button->setText(std::format("{} ({})", prototype.label, prototype.cost), kBarFontSize);
			button->onClick = [this, name]()
			{
				if (_ui != nullptr)
				{
					_ui->selectBuilding(name);
				}
			};

			Slot slot{};
			slot.cost = prototype.cost;
			slot.button = std::move(button);
			_slots.push_back(std::move(slot));
		}
	}

	void BuildBar::syncEnabled(const IEconomyResourceService& economy)
	{
		for (Slot& slot : _slots)
		{
			if (slot.button != nullptr)
			{
				slot.button->enabled = economy.canAfford(ResourceMoney, slot.cost);
			}
		}
	}

	void BuildBar::tick(hl::UiBatch& batch, glm::vec2 framebufferSize, const hl::ui::Pointer& pointer)
	{
		if (_root == nullptr)
		{
			return;
		}

		hl::ui::prepareTree(*_root);

		constexpr glm::vec2 barPad{ 16.0f, 8.0f };
		constexpr float chipGap = 8.0f;
		float chipsWidth = 0.0f;
		float chipsHeight = 0.0f;
		std::vector<glm::vec2> chipSizes(_slots.size());
		for (std::size_t i = 0; i < _slots.size(); ++i)
		{
			chipSizes[i] = _slots[i].button->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
			chipsWidth += chipSizes[i].x;
			chipsHeight = std::max(chipsHeight, chipSizes[i].y);
		}
		if (!_slots.empty())
		{
			chipsWidth += chipGap * static_cast<float>(_slots.size() - 1);
		}

		_panel->node().setBottomCenter(
			{ chipsWidth + barPad.x * 2.0f, chipsHeight + barPad.y * 2.0f });
		_panel->node().relative = { 0.0f, 0.0f };

		float chipX = barPad.x;
		for (std::size_t i = 0; i < _slots.size(); ++i)
		{
			_slots[i].button->node().setTopLeft(chipSizes[i]);
			_slots[i].button->node().relative = { chipX, barPad.y };
			_slots[i].button->node().intrinsicSize.reset();
			chipX += chipSizes[i].x + chipGap;
		}

		hl::ui::layout(*_root, hl::ui::Box{ 0.0f, 0.0f, framebufferSize.x, framebufferSize.y });
		hl::ui::dispatch(*_root, pointer);

		batch.setFullScissor(VkRect2D{
			{ 0, 0 },
			{ static_cast<uint32_t>(std::max(0.0f, framebufferSize.x)),
			  static_cast<uint32_t>(std::max(0.0f, framebufferSize.y)) }
		});
		batch.begin();
		BatchPaint paint(batch);
		hl::ui::paintTree(*_root, paint);
	}

	bool BuildBar::hits(glm::vec2 position) const
	{
		if (_root == nullptr)
		{
			return false;
		}

		return hl::ui::hitTest(*_root, position) != nullptr;
	}

}
