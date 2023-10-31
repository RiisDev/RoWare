#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <cmath>

#include <optional>

namespace math::types
{
	struct vector_t
	{
		float x, y, z;

		vector_t() = default;
		vector_t(float x, float y, float z) : x{ x }, y{ y }, z{ z } {}

		auto operator*(vector_t mul) const noexcept
		{
			vector_t ret;

			ret.x = x * mul.x;
			ret.y = y * mul.y;
			ret.z = z * mul.z;
			return ret;
		}

		vector_t operator*(float f) const {
			return vector_t(x * f, y * f, z * f);
		}

		vector_t& operator-= (const vector_t& rkVector) {
			x -= rkVector.x;
			y -= rkVector.y;
			z -= rkVector.z;
			return *this;
		}

		float unitize(float fTolerance) {
			float fMagnitude = magnitude();

			if (fMagnitude > fTolerance) {
				float fInvMagnitude = 1.0f / fMagnitude;
				x *= fInvMagnitude;
				y *= fInvMagnitude;
				z *= fInvMagnitude;
			}
			else {
				fMagnitude = 0.0f;
			}

			return fMagnitude;
		}

		float dot(const vector_t & rkVector) const {
			return x * rkVector.x + y * rkVector.y + z * rkVector.z;
		}

		float squaredMagnitude() const {
			return x * x + y * y + z * z;
		}

		vector_t direction() const {
			const float lenSquared = squaredMagnitude();
			const float invSqrt = 1.0f / sqrtf(lenSquared);
			return vector_t(x * invSqrt, y * invSqrt, z * invSqrt);
		}

		auto operator+(vector_t add) const noexcept
		{
			vector_t ret;

			ret.x = x + add.x;
			ret.y = y + add.y;
			ret.z = z + add.z;

			return ret;
		}

		auto operator-(vector_t sub) const noexcept
		{
			vector_t ret;

			ret.x = x - sub.x;
			ret.y = y - sub.y;
			ret.z = z - sub.z;

			return ret;
		}

		auto operator=(vector_t other) noexcept
		{
			this->x = other.x;
			this->y = other.y;
			this->z = other.z;
		}

		const float magnitude() const
		{
			return sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2));
		}

		const float distance(vector_t vector) const
		{
			return (*this - vector).magnitude();
		}

		const vector_t normalize() const
		{
			vector_t ret;
			float mag = this->magnitude();
			ret.x = x / mag;
			ret.y = y / mag;
			ret.z = z / mag;
			return ret;
		}

		auto cross(vector_t vec) const
		{
			vector_t ret;
			ret.x = y * vec.z - z * vec.y;
			ret.y = -(x * vec.z - z * vec.x);
			ret.z = x * vec.y - y * vec.x;
			return ret;
		}

		/*
			local watcher = game.Workspace.watcher

			function abc(a, b)
				local x = a.Y * b.Z - a.Z * b.Y
				local y = -(a.X * b.Z - a.Z * b.X)
				local z = a.X * b.Y - a.Y * b.X
				return Vector3.new(x, y, z)
			end

			function unit(a)
				local mag = math.sqrt(math.pow(a.X, 2) + math.pow(a.Y, 2) + math.pow(a.Z, 2));
				local x = a.X / mag;
				local y = a.Y / mag;
				local z = a.Z / mag;
				return Vector3.new(x,y,z)
			end

			while wait() do
				local lookvector = (watcher.position - game.Workspace.CurrentCamera.CFrame.position).Unit
				local right_vector = Vector3.new(0, 1, 0):Cross(lookvector)
				local up_vector = lookvector:Cross(right_vector)

				watcher.CFrame = CFrame.fromMatrix(
					watcher.position,
					right_vector,
					Vector3.new(0,1,0),
					lookvector
				)

				local a = watcher.position - game.Workspace.CurrentCamera.CFrame.position

				print(lookvector, unit(a), lookvector == unit(a))
			end
		*/
	};

	struct vector2_t
	{
		float x, y;

		vector2_t() = default;
		vector2_t(float x, float y) : x{ x }, y{ y } {}

		const float distance(const vector2_t& vector) const
		{
			return std::sqrtf(std::powf(x - vector.x, 2.0) + std::powf(y - vector.y, 2.0));
		}
	};

	struct vector4_t
	{
		float x, y, z, w;

		vector4_t() = default;
		vector4_t(float x, float y, float z, float w) : x{ x }, y{ y }, z{ z }, w{ w } {}
	};

	struct matrix4_t
	{
		float mat[4][4];

		constexpr auto operator[](int i) const noexcept { return mat[i]; }
		auto operator*(vector_t mul) const noexcept
		{
			vector4_t ret;

			ret.x = mat[0][0] * mul.x + mat[0][1] * mul.y + mat[0][2] * mul.z + mat[0][3];
			ret.y = mat[1][0] * mul.x + mat[1][1] * mul.y + mat[1][2] * mul.z + mat[1][3];
			ret.z = mat[2][0] * mul.x + mat[2][1] * mul.y + mat[2][2] * mul.z + mat[2][3];
			ret.w = mat[3][0] * mul.x + mat[3][1] * mul.y + mat[3][2] * mul.z + mat[3][3];

			return ret;
		}
	};

	struct cframe_t
	{
		vector_t right_vector = { 1, 0, 0 };
		vector_t up_vector = { 0, 1, 0 };
		vector_t back_vector = { 0, 0, 1 };
		vector_t position = { 0, 0, 0 };

		cframe_t() = default;
		cframe_t(vector_t position) : position{ position } {}
		cframe_t(vector_t right_vector, vector_t up_vector, vector_t back_vector, vector_t position) : right_vector{ right_vector }, up_vector{ up_vector }, back_vector{ back_vector }, position{ position } {}

	public:
		void look_at_locked(vector_t point) noexcept // NOTE: Y DIRECTION IS LOCKED
		{
			vector_t look_vector = (this->position - point).normalize() * vector_t{1, 0, 1}; // simulate as if aligned with y axis
			vector_t right_vector = vector_t{0, 1, 0}.cross(look_vector); // this is a must since I need a 90* angle still
			vector_t up_vector = { 0, 1, 0 }; // since we've simulated as if y was on same level, no point to do this cross calculation

			this->right_vector = right_vector;
			this->up_vector = up_vector;
			this->back_vector = look_vector * vector_t{ -1, -1, -1 };
		}

		cframe_t look_at(vector_t point) noexcept
		{
			vector_t look_vector = (position - point).normalize() * vector_t { -1, -1, -1 };
			vector_t right_vector = vector_t(0, 1, 0).cross(look_vector);
			vector_t up_vector = look_vector.cross(right_vector);

			return cframe_t{ right_vector, up_vector, look_vector, this->position };
		}

		auto operator*(cframe_t cframe) const noexcept
		{
			cframe_t ret;

			ret.right_vector =
			{
				right_vector.x * cframe.right_vector.x + right_vector.y * cframe.up_vector.x + right_vector.z * cframe.back_vector.x,
				right_vector.x * cframe.right_vector.y + right_vector.y * cframe.up_vector.y + right_vector.z * cframe.back_vector.y,
				right_vector.x * cframe.right_vector.z + right_vector.y * cframe.up_vector.z + right_vector.z * cframe.back_vector.z
			};
			ret.up_vector =
			{
				up_vector.x * cframe.right_vector.x + up_vector.y * cframe.up_vector.x + up_vector.z * cframe.back_vector.x,
				up_vector.x * cframe.right_vector.y + up_vector.y * cframe.up_vector.y + up_vector.z * cframe.back_vector.y,
				up_vector.x * cframe.right_vector.z + up_vector.y * cframe.up_vector.z + up_vector.z * cframe.back_vector.z
			};
			ret.back_vector =
			{
				back_vector.x * cframe.right_vector.x + back_vector.y * cframe.up_vector.x + back_vector.z * cframe.back_vector.x,
				back_vector.x * cframe.right_vector.y + back_vector.y * cframe.up_vector.y + back_vector.z * cframe.back_vector.y,
				back_vector.x * cframe.right_vector.z + back_vector.y * cframe.up_vector.z + back_vector.z * cframe.back_vector.z
			};
			ret.position =
			{
				right_vector.x * cframe.position.x + right_vector.y * cframe.position.y + right_vector.z * cframe.position.z + position.x,
				up_vector.x * cframe.position.x + up_vector.y * cframe.position.y + up_vector.z * cframe.position.z + position.y,
				back_vector.x * cframe.position.x + back_vector.y * cframe.position.y + back_vector.z * cframe.position.z + position.z
			};

			return ret;
		}

		auto operator*(vector_t vec) const noexcept
		{
			vector_t ret;

			ret.x = right_vector.x * vec.x + right_vector.y * vec.y + right_vector.z * vec.z + position.x;
			ret.y = up_vector.x * vec.x + up_vector.y * vec.y + up_vector.z * vec.z + position.y;
			ret.z = back_vector.x * vec.x + back_vector.y * vec.y + back_vector.z * vec.z + position.z;

			return ret;
		}
	};

	struct matrix3_t
	{
		float mat[3][3];

		constexpr auto operator[](int i) const noexcept { return mat[i]; }
	};
}

namespace math::functions
{
	std::optional<math::types::vector2_t> world_to_screen(const math::types::vector_t& in);
}