#pragma once

#include <Services/BuildingPrototypeService.hpp>
#include <string>

namespace drl
{

	inline constexpr int kUiKeyEscape = 256;

	enum class UiState
	{
		Default = 0,
		PlacingBuilding = 1
	};

	class IUiInput
	{
	public:
		virtual ~IUiInput() = 0;
		virtual bool isKeyDown(int key) const = 0;
	};

	inline IUiInput::~IUiInput() = default;

	class IUiService
	{
	public:
		virtual ~IUiService() = 0;

		virtual void update(const IUiInput& input) = 0;
		virtual void selectBuilding(const std::string& prototypeName) = 0;
		virtual std::string getActiveBuildingType() const = 0;
		virtual UiState getCurrentState() const = 0;
		virtual void clearActiveBuilding() = 0;
	};

	inline IUiService::~IUiService() = default;

	class UiService : public IUiService
	{
	public:
		explicit UiService(const BuildingPrototypeService& buildingPrototypes);
		~UiService() override = default;

		void update(const IUiInput& input) override;
		void selectBuilding(const std::string& prototypeName) override;
		std::string getActiveBuildingType() const override;
		UiState getCurrentState() const override;
		void clearActiveBuilding() override;

	private:
		const BuildingPrototypeService& _buildingPrototypes;
		std::string _activeBuilding;
		UiState _currentState{ UiState::Default };
	};

}
