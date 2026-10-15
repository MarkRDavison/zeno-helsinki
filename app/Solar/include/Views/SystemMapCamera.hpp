#pragma once

#include <World/SystemPlaces.hpp>
#include <helsinki/System/glm.hpp>
#include <imgui.h>

namespace slr
{

	class SystemMapCamera
	{
	public:
		static constexpr float DefaultFitRadius = 120.0f;
		static constexpr float CanvasPad = 48.0f;

		static float computeBaseFitScale(const SystemLayout& layout, const ImVec2& canvasSize);

		void setCanvas(const ImVec2& screenOrigin, const ImVec2& canvasSize);
		void resetToFit(const SystemLayout& layout);

		float effectiveScale() const;
		const glm::vec2& panCenter() const { return _panCenter; }

		ImVec2 systemSpaceToScreen(glm::vec2 systemPos) const;
		glm::vec2 screenToSystemSpace(ImVec2 screen) const;

		void panByScreenDelta(const ImVec2& screenDelta);
		void zoomAtScreen(const ImVec2& screen, int scrollY);

	private:
		float _baseFitScale{ 1.0f };
		float _zoom{ 1.0f };
		glm::vec2 _panCenter{ 0.0f };
		ImVec2 _screenOrigin{ 0.0f, 0.0f };
		ImVec2 _canvasSize{ 0.0f, 0.0f };
	};

}
