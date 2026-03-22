#pragma once

namespace pong
{

	struct CollisionData
	{
		float X{ 0.0f };
		float Y{ 0.0f };
		float W{ 0.0f };
		float H{ 0.0f };

		float vX{ 0.0f };
		float vY{ 0.0f };

		bool Collides(const CollisionData& other)
		{
			return (
				this->X < other.X + other.W &&
				this->X + this->W > other.X &&
				this->Y < other.Y + other.H &&
				this->Y + this->H > other.Y
			);
		}
	};

}