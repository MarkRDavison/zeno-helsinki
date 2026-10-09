#pragma once

#include <Entities/Data/WorkerData.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>

namespace drl
{

	class WorkerView
	{
	public:
		WorkerView(const WorkerData& workerData, float originX, float originY, float tileSize);

		void draw(hl::PipelineDrawData& pdd) const;

	private:
		const WorkerData& _workerData;
		float _originX;
		float _originY;
		float _tileSize;
	};

}
