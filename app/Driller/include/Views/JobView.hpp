#pragma once

#include <Entities/Data/JobData.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>

namespace drl
{

	class JobView
	{
	public:
		JobView(const JobData& jobData, float originX, float originY, float tileSize);

		void draw(hl::PipelineDrawData& pdd) const;
		void setCameraIndex(int cameraIndex) { _cameraIndex = cameraIndex; }

	private:
		const JobData& _jobData;
		float _originX;
		float _originY;
		float _tileSize;
		int _cameraIndex{ 0 };
	};

}
