#include <helsinki/Renderer/Resource/ModelResource.hpp>
#include <helsinki/System/Utils/String.hpp>
#include <unordered_set>
#include <fstream>
#include <format>

namespace hl
{

	ModelResource::ModelResource(
		const std::string& id,
		ResourceContext& context
	) :
		Resource(id),
		_device(*context.device),
		_commandPool(*context.pool),
		_resourceManager(*context.resourceManager),
		_rootPath(context.rootPath)
	{

	}

	bool ModelResource::Load()
	{
		std::string modelPath = std::format("{}/data/models/{}.obj", _rootPath, GetId());
		
		std::ifstream file(modelPath);
		if (!file.is_open())
		{
			return false;
		}

		std::unordered_set<std::string> materialFiles;

		std::string line;
		while (std::getline(file, line))
		{
			std::istringstream iss(line);
			std::string key;
			iss >> key;

			if (key.starts_with('#'))
			{
				// Comment
				continue;
			}
			else if (line.empty())
			{
				continue;
			}
			else if (key == "mtllib")
			{
				// Define the material and path
				std::string mtlFile; iss >> mtlFile;

				materialFiles.insert(mtlFile);
			}
			else if (key == "usemtl")
			{
				// Start using the material
			}
		}

		for (const auto& materialFile : materialFiles)
		{
			std::cout << "Material file: " << materialFile << std::endl;
		}

		return Resource::Load();;
	}

	void ModelResource::Unload()
	{
		if (IsLoaded())
		{
			Resource::Unload();
		}
	}
}