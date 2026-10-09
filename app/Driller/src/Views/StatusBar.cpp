#include <Views/StatusBar.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Theme.hpp>
#include <helsinki/Ui/Widget.hpp>
#include <algorithm>
#include <limits>

namespace drl
{

	namespace
	{
		constexpr float kTexWhite = 0.0f;
		constexpr float kTexRoboto = 1.0f;
		constexpr unsigned kBarFontSize = 18;
		constexpr glm::vec2 kIconSize{ 24.0f, 24.0f };
		constexpr float kChipGap = 8.0f;
		constexpr float kIconTextGap = 6.0f;
		constexpr glm::vec2 kBarPad{ 16.0f, 8.0f };

		glm::vec3 placeholderTint(std::size_t index)
		{
			constexpr glm::vec3 tints[] = {
				{ 0.72f, 0.48f, 0.28f },
				{ 0.86f, 0.72f, 0.22f },
				{ 0.32f, 0.72f, 0.42f },
				{ 0.38f, 0.58f, 0.86f },
				{ 0.72f, 0.42f, 0.68f }
			};
			return tints[index % (sizeof(tints) / sizeof(tints[0]))];
		}

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

	void StatusBar::initialise(
		hl::FontResource* font,
		const IEconomyResourceService& economy,
		const IUpgradeService& upgrades)
	{
		_typeface = std::make_unique<SceneFontTypeface>(font);
		_root = std::make_unique<hl::ui::Node>();
		_root->setFillParent();

		_panel = std::make_unique<hl::ui::Panel>(_root->addChild());
		_panel->color = { 0.08f, 0.09f, 0.12f };
		_panel->opacity = 0.92f;
		_panel->hitTestEnabled = true;

		const auto specs = collectStatusBarChips(economy, upgrades);
		for (std::size_t i = 0; i < specs.size(); ++i)
		{
			ChipWidgets chip;
			chip.spec = specs[i];
			chip.panel = std::make_unique<hl::ui::Panel>(_panel->node().addChild());
			chip.panel->opacity = 0.0f;
			chip.panel->hitTestEnabled = true;

			chip.icon = std::make_unique<hl::ui::Image>(chip.panel->node().addChild());
			chip.icon->size = kIconSize;
			chip.icon->color = placeholderTint(i);

			chip.value = std::make_unique<hl::ui::Label>(chip.panel->node().addChild(), *_typeface);
			chip.value->setText("0", kBarFontSize);

			_chips.push_back(std::move(chip));
		}

		_tooltip = std::make_unique<hl::ui::Tooltip>(_root->addChild(), *_typeface);
		_tooltip->delay = 0.0f;
		syncValues(economy, upgrades);
	}

	void StatusBar::syncValues(
		const IEconomyResourceService& economy,
		const IUpgradeService& upgrades)
	{
		for (ChipWidgets& chip : _chips)
		{
			const std::string value = formatStatusValue(chip.spec, economy, upgrades);
			chip.value->setText(value, kBarFontSize);
			chip.panel->tooltip = formatStatusTooltip(chip.spec.label, value, chip.spec.description);
		}
	}

	void StatusBar::tick(
		hl::UiBatch& batch,
		glm::vec2 framebufferSize,
		const hl::ui::Pointer& pointer,
		float dt,
		const IEconomyResourceService& economy,
		const IUpgradeService& upgrades)
	{
		if (_root == nullptr)
		{
			return;
		}

		syncValues(economy, upgrades);
		hl::ui::prepareTree(*_root);

		float chipsHeight = kIconSize.y;
		std::vector<glm::vec2> chipSizes(_chips.size());
		for (std::size_t i = 0; i < _chips.size(); ++i)
		{
			const glm::vec2 textSize = _chips[i].value->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
			chipSizes[i] = {
				kIconSize.x + kIconTextGap + textSize.x,
				std::max(kIconSize.y, textSize.y)
			};
			chipsHeight = std::max(chipsHeight, chipSizes[i].y);
		}

		const float barHeight = chipsHeight + kBarPad.y * 2.0f;
		_panel->node().setTopLeft({ framebufferSize.x, barHeight });
		_panel->node().relative = { 0.0f, 0.0f };

		float chipX = kBarPad.x;
		for (std::size_t i = 0; i < _chips.size(); ++i)
		{
			ChipWidgets& chip = _chips[i];
			chip.panel->node().setTopLeft(chipSizes[i]);
			chip.panel->node().relative = { chipX, kBarPad.y };
			chip.panel->node().intrinsicSize.reset();

			chip.icon->node().setCenterLeft(kIconSize);
			chip.icon->node().relative = { 0.0f, 0.0f };
			chip.icon->node().intrinsicSize.reset();

			const glm::vec2 textSize = chip.value->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
			chip.value->node().setCenterLeft(textSize);
			chip.value->node().relative = { kIconSize.x + kIconTextGap, 0.0f };
			chip.value->node().intrinsicSize.reset();

			chipX += chipSizes[i].x + kChipGap;
		}

		hl::ui::layout(*_root, hl::ui::Box{ 0.0f, 0.0f, framebufferSize.x, framebufferSize.y });
		hl::ui::dispatch(*_root, pointer);
		if (_tooltip != nullptr)
		{
			_tooltip->tick(dt);
		}

		batch.setFullScissor(VkRect2D{
			{ 0, 0 },
			{ static_cast<uint32_t>(std::max(0.0f, framebufferSize.x)),
			  static_cast<uint32_t>(std::max(0.0f, framebufferSize.y)) }
		});
		BatchPaint paint(batch);
		hl::ui::paintTree(*_root, paint);
	}

	bool StatusBar::hits(glm::vec2 position) const
	{
		if (_root == nullptr)
		{
			return false;
		}

		return hl::ui::hitTest(*_root, position) != nullptr;
	}

}
