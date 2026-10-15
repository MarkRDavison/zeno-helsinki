#include <Views/SystemMapCamera.hpp>
#include <algorithm>
#include <cmath>

namespace slr
{
namespace
{
	constexpr float MinZoom = 0.25f;
	constexpr float MaxZoom = 4.0f;
	constexpr float ZoomStep = 1.1f;
}

	float SystemMapCamera::computeBaseFitScale(const SystemLayout& layout, const ImVec2& canvasSize)
	{
		const float fitRadius = (std::max)(outRadiusForLayout(layout), DefaultFitRadius);
		const float usable = (std::min)(canvasSize.x, canvasSize.y) - CanvasPad * 2.0f;
		return usable > 0.0f ? usable / (fitRadius * 2.0f) : 1.0f;
	}

	void SystemMapCamera::setCanvas(const ImVec2& screenOrigin, const ImVec2& canvasSize)
	{
		_screenOrigin = screenOrigin;
		_canvasSize = canvasSize;
	}

	void SystemMapCamera::resetToFit(const SystemLayout& layout)
	{
		_baseFitScale = computeBaseFitScale(layout, _canvasSize);
		_zoom = 1.0f;
		_panCenter = starPositionInSystemSpace(layout);
	}

	float SystemMapCamera::effectiveScale() const
	{
		return _baseFitScale * _zoom;
	}

	ImVec2 SystemMapCamera::systemSpaceToScreen(glm::vec2 systemPos) const
	{
		const float scale = effectiveScale();
		const ImVec2 centre(
			_screenOrigin.x + _canvasSize.x * 0.5f,
			_screenOrigin.y + _canvasSize.y * 0.5f);
		const glm::vec2 delta = systemPos - _panCenter;
		return ImVec2(centre.x + delta.x * scale, centre.y - delta.y * scale);
	}

	glm::vec2 SystemMapCamera::screenToSystemSpace(ImVec2 screen) const
	{
		const float scale = effectiveScale();
		if (scale <= 0.0f)
		{
			return _panCenter;
		}

		const ImVec2 centre(
			_screenOrigin.x + _canvasSize.x * 0.5f,
			_screenOrigin.y + _canvasSize.y * 0.5f);
		return glm::vec2(
			_panCenter.x + (screen.x - centre.x) / scale,
			_panCenter.y - (screen.y - centre.y) / scale);
	}

	void SystemMapCamera::panByScreenDelta(const ImVec2& screenDelta)
	{
		const float scale = effectiveScale();
		if (scale <= 0.0f)
		{
			return;
		}

		_panCenter.x += screenDelta.x / scale;
		_panCenter.y -= screenDelta.y / scale;
	}

	void SystemMapCamera::zoomAtScreen(const ImVec2& screen, int scrollY)
	{
		if (scrollY == 0)
		{
			return;
		}

		const glm::vec2 before = screenToSystemSpace(screen);
		if (scrollY > 0)
		{
			_zoom = (std::min)(_zoom * ZoomStep, MaxZoom);
		}
		else
		{
			_zoom = (std::max)(_zoom / ZoomStep, MinZoom);
		}

		const glm::vec2 after = screenToSystemSpace(screen);
		_panCenter += before - after;
	}

}
