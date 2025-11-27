#include "SkeletonConfig.hpp"
#include <SkeletonEngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>

#define ROOT_PATH(x) (std::string(sk::SkeletonConfig::RootPath) + std::string(x))

int main()
{
	hl::EventBus eventBus;

	hl::Engine engine(eventBus);

	auto engineConfig = hl::EngineConfiguration::Load(ROOT_PATH("/data/config.json"));
	engineConfig.RootPath = std::string(sk::SkeletonConfig::RootPath);

	engine.init(engineConfig);

	engine.setScene(new sk::SkeletonEngineScene(std::string(sk::SkeletonConfig::RootPath)));

	engine.run();

	return 0;
}