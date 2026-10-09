#pragma once

#include <Entities/Data/ShuttleData.hpp>
#include <Services/ShuttlePrototypeService.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>

namespace drl
{

	class ShuttleView
	{
	public:
		ShuttleView(
			const ShuttleData& shuttleData,
			const IShuttlePrototypeService& shuttlePrototypes,
			float originX,
			float originY,
			float tileSize);

		void draw(hl::PipelineDrawData& pdd) const;
		void setCameraIndex(int cameraIndex) { _cameraIndex = cameraIndex; }

	private:
		void drawCell(hl::PipelineDrawData& pdd, float tileX, float tileY, int frameIndex) const;

		const ShuttleData& _shuttleData;
		const IShuttlePrototypeService& _shuttlePrototypes;
		float _originX;
		float _originY;
		float _tileSize;
		int _cameraIndex{ 0 };
	};

}
