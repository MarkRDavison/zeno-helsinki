#pragma once

#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Engine/ECS/Components/TextComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <helsinki/Renderer/Resource/SignedDistanceFieldFontResource.hpp>
#include <helsinki/Renderer/Resource/TextSystem.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <helsinki/System/glm.hpp>
#include <string>

namespace phys
{
	inline void loadTextResources(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext)
	{
		resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
			hl::MaterialSystem::FallbackTextureName,
			resourceContext);
		resourceManager.LoadAs<hl::SignedDistanceFieldFontResource, hl::FontResource>(
			"roboto",
			resourceContext);
		resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
			"roboto",
			resourceContext);

		resourceManager.LoadLogical(
			hl::ResourceDefinition{
				.name = hl::TextSystem::RasterAtlasName,
				.type = "logical",
				.resources = { { .name = hl::MaterialSystem::FallbackTextureName, .type = "texture" } }
			},
			[&](const hl::ResourceDefinition::Child& child)
			{
				return resourceManager.HasResource<hl::ImageSamplerResource>(child.name);
			});
		resourceManager.LoadLogical(
			hl::ResourceDefinition{
				.name = hl::TextSystem::SdfAtlasName,
				.type = "logical",
				.resources = { { .name = "roboto", .type = "texture" } }
			},
			[&](const hl::ResourceDefinition::Child& child)
			{
				return resourceManager.HasResource<hl::ImageSamplerResource>(child.name);
			});
	}

	inline void addTextEntity(
		hl::Scene& scene,
		hl::Engine& engine,
		const std::string& name,
		const std::string& text,
		int size,
		const glm::vec4& colour = glm::vec4(1.f, 1.f, 1.f, 1.f))
	{
		auto entity = scene.addEntity(name);
		entity->AddTag("TEXT");
		entity->AddComponent<hl::TransformComponent>();
		entity->AddComponent<hl::TextComponent>()->setString(
			engine.getTextSystem(),
			text,
			"roboto",
			size);
		entity->GetComponent<hl::TextComponent>()->setColour(colour);
	}

	inline void centerTextAt(
		hl::Scene& scene,
		hl::Engine& engine,
		int width,
		int height,
		const std::string& entityName,
		float yOffset)
	{
		auto desiredCenter = glm::vec2(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 3.0f + yOffset);
		auto entity = scene.getEntity(entityName);
		const auto& size = engine.getTextSystem().getTextSize(
			entity->GetComponent<hl::TextComponent>()->getTextSystemId());
		desiredCenter.x += size.x - size.z / 2.0f;
		desiredCenter.y += size.y - size.w / 2.0f;
		entity->GetComponent<hl::TransformComponent>()->SetPosition(glm::vec3(desiredCenter, 0.0f));
	}
}
