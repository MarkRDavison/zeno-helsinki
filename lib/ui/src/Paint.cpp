#include <helsinki/Ui/Paint.hpp>

namespace hl::ui
{
	namespace
	{
		float splitPair(float first, float second, float size)
		{
			if (first + second <= size)
			{
				return first;
			}

			if (first + second <= 0.0f)
			{
				return 0.0f;
			}

			return first * (size / (first + second));
		}

		void emitPatch(
			IPaint& paint,
			float x,
			float y,
			float w,
			float h,
			float u0,
			float v0,
			float u1,
			float v1,
			glm::vec3 color)
		{
			if (w <= 0.0f || h <= 0.0f)
			{
				return;
			}

			paint.sprite(Box{ x, y, w, h }, glm::vec4{ u0, v0, u1, v1 }, color);
		}
	}

	void IPaint::nineSlice(const Box& dest, const NineSlice& spec)
	{
		const float left = splitPair(spec.slice.left, spec.slice.right, dest.size.x);
		const float right = dest.size.x - left >= spec.slice.right
			? spec.slice.right
			: dest.size.x - left;
		const float top = splitPair(spec.slice.top, spec.slice.bottom, dest.size.y);
		const float bottom = dest.size.y - top >= spec.slice.bottom
			? spec.slice.bottom
			: dest.size.y - top;

		const float x0 = dest.pos.x;
		const float x1 = x0 + left;
		const float x3 = x0 + dest.size.x;
		const float x2 = x3 - right;
		const float y0 = dest.pos.y;
		const float y1 = y0 + top;
		const float y3 = y0 + dest.size.y;
		const float y2 = y3 - bottom;

		const float su = spec.sourceSize.x > 0.0f ? spec.sourceSize.x : 1.0f;
		const float sv = spec.sourceSize.y > 0.0f ? spec.sourceSize.y : 1.0f;
		const float u0 = spec.uvRect.x;
		const float v0 = spec.uvRect.y;
		const float u3 = spec.uvRect.z;
		const float v3 = spec.uvRect.w;
		const float du = u3 - u0;
		const float dv = v3 - v0;
		const float uLeft = splitPair(spec.slice.left, spec.slice.right, su) / su * du;
		const float uRight = (su - spec.slice.left >= spec.slice.right
			? spec.slice.right
			: su - spec.slice.left) / su * du;
		const float vTop = splitPair(spec.slice.top, spec.slice.bottom, sv) / sv * dv;
		const float vBottom = (sv - spec.slice.top >= spec.slice.bottom
			? spec.slice.bottom
			: sv - spec.slice.top) / sv * dv;

		const float u1 = u0 + uLeft;
		const float u2 = u3 - uRight;
		const float v1 = v0 + vTop;
		const float v2 = v3 - vBottom;

		const float xs[4] = { x0, x1, x2, x3 };
		const float ys[4] = { y0, y1, y2, y3 };
		const float us[4] = { u0, u1, u2, u3 };
		const float vs[4] = { v0, v1, v2, v3 };

		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				emitPatch(
					*this,
					xs[col],
					ys[row],
					xs[col + 1] - xs[col],
					ys[row + 1] - ys[row],
					us[col],
					vs[row],
					us[col + 1],
					vs[row + 1],
					spec.color);
			}
		}
	}
}
