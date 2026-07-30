#pragma once

#include <string>
#include <helsinki/Engine/ECS/Component.hpp>

namespace hur
{
	// TODO: In general should they just be structs with public variables?
	class HealthComponent : public hl::Component
	{
	public:
		HealthComponent(
			int max, 
			int current
		) :
			_maxHealth(max), 
			_currentHealth(current)
		{
		}

		int getMaxHealth() const { return _maxHealth; }
		int getCurrentHealth() const { return _currentHealth; }
		void setCurrentHealth(int health) { _currentHealth = health; }

	private:
		int _maxHealth;
		int _currentHealth;
	};

}