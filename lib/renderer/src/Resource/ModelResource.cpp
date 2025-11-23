#include <helsinki/Renderer/Resource/ModelResource.hpp>
#include <helsinki/System/Utils/String.hpp>
#include <unordered_set>
#include <fstream>
#include <cassert>
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

	static std::vector<Material> LoadMaterialFile(const std::string& path)
	{
		std::vector<Material> materials;

		std::ifstream file(path);
		if (!file.is_open())
		{
			return materials;
		}

		Material* current{ nullptr };

		std::string line;
		while (std::getline(file, line))
		{
			std::istringstream iss(line);
			std::string key;
			iss >> key;

			if (key.starts_with('#') ||
				line.empty())
			{
				continue;
			}
			else if (key == "newmtl")
			{
				std::string materialName; iss >> materialName;
				materials.push_back({ .name = materialName });
				current = &materials.back();
			}
			else if (key == "Kd")
			{
				assert(current != nullptr);
				iss >> current->diffuse.x >> current->diffuse.y >> current->diffuse.z;
			}
			else if (key == "map_Kd")
			{
				assert(current != nullptr);
				iss >> current->diffuseTex;
			}
			else
			{
				throw std::runtime_error("Unhandled material key.");
			}
		}

		return materials;
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
		Mesh* currentMesh{ nullptr };
		std::string currentMaterial;
		std::unordered_map<Vertex, uint32_t> vertexMap;

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;

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
				if (!_meshes.empty() && !_meshes.back().vertices.empty())
				{ 
					assert(currentMesh != nullptr);
					currentMesh->materialName = currentMaterial;
				}

				_meshes.push_back({});
				currentMesh = &_meshes.back();
				vertexMap.clear();

				iss >> currentMaterial;				
			}
			else if (key == "g")
			{
				// no-op
			}
			else if (key == "v")
			{
				glm::vec3 pos{};
				iss >> pos.x >> pos.y >> pos.z;
				positions.push_back(pos);
			}
			else if (key == "vn")
			{
				glm::vec3 normal{};
				iss >> normal.x >> normal.y >> normal.z;
				normals.push_back(normal);
			}
			else if (key == "vt")
			{
				glm::vec2 uv{};
				iss >> uv.x >> uv.y;
				uvs.push_back(uv);
			}
			else if (key == "f")
			{
				assert(currentMesh != nullptr);
				std::string vStr;
				std::vector<uint32_t> faceIndices;
				while (iss >> vStr)
				{
					std::istringstream viss(vStr);
					std::string idx[3];
					int i = 0;
					while (std::getline(viss, idx[i], '/')) i++;

					Vertex vert{};
					vert.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
					vert.pos = positions[std::stoi(idx[0]) - 1];
					if (i > 1 && !idx[1].empty()) vert.texCoord = uvs[std::stoi(idx[1]) - 1];
					if (i > 2 && !idx[2].empty()) vert.normal = normals[std::stoi(idx[2]) - 1];

					auto it = vertexMap.find(vert);
					if (it != vertexMap.end())
					{
						faceIndices.push_back(it->second);
					}
					else
					{
						uint32_t newIndex = static_cast<uint32_t>(currentMesh->vertices.size());
						currentMesh->vertices.push_back(vert);
						vertexMap[vert] = newIndex;
						faceIndices.push_back(newIndex);
					}
				}

				for (size_t i = 1; i + 1 < faceIndices.size(); ++i)
				{
					currentMesh->indices.push_back(faceIndices[0]);
					currentMesh->indices.push_back(faceIndices[i]);
					currentMesh->indices.push_back(faceIndices[i + 1]);
				}
			}
		}

		for (const auto& materialFile : materialFiles)
		{
			std::cout << "Material file: " << materialFile << std::endl;
			for (const auto& m : LoadMaterialFile(std::format("{}/data/models/{}", _rootPath, materialFile)))
			{
				_materials.emplace_back(m);
			}
		}

		return Resource::Load();

		return false;
	}

	void ModelResource::Unload()
	{
		if (IsLoaded())
		{
			Resource::Unload();
		}
	}
}