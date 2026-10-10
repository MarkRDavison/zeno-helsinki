#include <Views/Hud.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Widget.hpp>
#include <algorithm>
#include <limits>
#include <utility>

namespace drl
{

	namespace
	{
		constexpr float kTexWhite = 0.0f;
		constexpr float kTexRoboto = 1.0f;

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

	void Hud::initialise(
		hl::FontResource* font,
		const IEconomyResourceService& economy,
		const IUpgradeService& upgrades,
		const BuildingPrototypeService& buildingPrototypes,
		IUiService& ui)
	{
		_typeface = std::make_unique<SceneFontTypeface>(font);
		_root = std::make_unique<hl::ui::Node>();
		_root->setFillParent();

		_statusBar.initialise(*_typeface, *_root, economy, upgrades);
		_buildBar.initialise(*_typeface, *_root, buildingPrototypes, ui);
		_snackbar = std::make_unique<hl::ui::SnackbarHost>(*_root, *_typeface);
		_snackbar->corner = hl::ui::SnackbarCorner::TopRight;
	}

	void Hud::tick(
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

		_statusBar.syncValues(economy, upgrades);
		_buildBar.syncEnabled(economy);
		hl::ui::prepareTree(*_root);
		_statusBar.layout(framebufferSize);
		_buildBar.layout();
		hl::ui::layout(*_root, hl::ui::Box{ 0.0f, 0.0f, framebufferSize.x, framebufferSize.y });
		hl::ui::dispatch(*_root, pointer);
		_statusBar.tickOverlays(dt);
		if (_snackbar != nullptr)
		{
			_snackbar->tick(dt);
		}

		batch.setFullScissor(VkRect2D{
			{ 0, 0 },
			{ static_cast<uint32_t>(std::max(0.0f, framebufferSize.x)),
			  static_cast<uint32_t>(std::max(0.0f, framebufferSize.y)) }
		});
		BatchPaint paint(batch);
		hl::ui::paintTree(*_root, paint);
	}

	bool Hud::hits(glm::vec2 position) const
	{
		if (_root == nullptr)
		{
			return false;
		}

		return hl::ui::hitTest(*_root, position) != nullptr;
	}

	void Hud::show(hl::ui::SnackbarItem item)
	{
		if (_snackbar == nullptr)
		{
			return;
		}

		_snackbar->show(std::move(item));
	}

}
