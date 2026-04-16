#pragma once

#include <cmath>
#include <limits>
#include <variant>
#include <cstdint>
#include <algorithm>
#include <type_traits>

namespace hl
{
    template<typename>
    inline constexpr bool AlwaysFalse = false;

    template<typename _Container>
    struct CoreTraits
    {
        static_assert(AlwaysFalse<_Container>,
            "No CoreTraits specialization found for this type. "
            "Please either remove RATUI_OVERRIDE_*_IMPL to use the default implementation based on standard library types, "
            "or provide a specialization of CoreTraits for your custom type.");
    };

    using f32 = float;
    using i32 = std::int32_t;
    using u32 = std::uint32_t;
    using u8 = std::uint8_t;

    template <unsigned N, class S>
    struct vbase
    {
        using Scalar = S;

        S data[N];

        constexpr vbase() : data{} {}

        template <class... Args>
        constexpr vbase(Args... args) : data{ args... }
        {
            static_assert(sizeof...(args) == N, "Wrong number of initializers.");
        }

        S& operator[](const int i) { return data[i]; }

        constexpr const S& operator[](const int i) const { return data[i]; }
    };

    template <class S, unsigned N>
    struct vec;

    template <class S>
    struct vec<S, 2> : public vbase<2, S>
    {
        vec() = default;
        explicit constexpr vec(const S v) : vbase<2, S>{ v, v } {}
        constexpr vec(const S x, const S y) : vbase<2, S>{ x, y } {}

        constexpr S x() const { return this->data[0]; }
        constexpr S y() const { return this->data[1]; }
    };

    template <class S>
    struct vec<S, 3> : public vbase<3, S>
    {
        vec() = default;
        explicit constexpr vec(const S v) : vbase<3, S>{ v, v, v } {}
        constexpr vec(const S x, const S y, const S z) : vbase<3, S>{ x, y, z } {}
        constexpr vec(const vec<S, 2>& xy, const S z) : vbase<3, S>{ xy[0], xy[1], z } {}
        constexpr vec(const S x, const vec<S, 2>& yz) : vbase<3, S>{ x, yz[0], yz[1] } {}

        constexpr S x() const { return this->data[0]; }
        constexpr S y() const { return this->data[1]; }
        constexpr S z() const { return this->data[2]; }
    };

    template <class S>
    struct vec<S, 4> : public vbase<4, S>
    {
        vec() = default;
        constexpr vec(const vbase<4, S>& b) : vbase<4, S>(b) {}
        explicit constexpr vec(const S v) : vbase<4, S>{ v, v, v, v } {}
        constexpr vec(const S x, const S y, const S z, const S w) : vbase<4, S>{ x, y, z, w } {}
        constexpr vec(const S x, const S y, const vec<S, 2>& zw) : vbase<4, S>{ x, y, zw[0], zw[1] } {}
        constexpr vec(const S x, const vec<S, 3>& yzw) : vbase<4, S>{ x, yzw[0], yzw[1], yzw[2] } {}
        constexpr vec(const S x, const vec<S, 2>& yz, const S w) : vbase<4, S>{ x, yz[0], yz[1], w } {}
        constexpr vec(const vec<S, 2>& xy, const S z, const S w) : vbase<4, S>{ xy[0], xy[1], z, w } {}
        constexpr vec(const vec<S, 2>& xy, const vec<S, 2>& zw) : vbase<4, S>{ xy[0], xy[1], zw[0], zw[1] } {}
        constexpr vec(const vec<S, 3>& xyz, const S w) : vbase<4, S>{ xyz[0], xyz[1], xyz[2], w } {}

        constexpr S x() const { return this->data[0]; }
        constexpr S y() const { return this->data[1]; }
        constexpr S z() const { return this->data[2]; }
        constexpr S w() const { return this->data[3]; }
    };

    template <class S, unsigned N>
    inline constexpr vec<S, N> operator+(const vec<S, N>& lhs,
        const vec<S, N>& rhs)
    {
        vec<S, N> result{};
        for (unsigned i = 0; i < N; ++i)
        {
            result.data[i] = lhs.data[i] + rhs.data[i];
        }
        return result;
    }

    template <class S, unsigned N>
    inline constexpr vec<S, N> operator-(const vec<S, N>& lhs,
        const vec<S, N>& rhs)
    {
        vec<S, N> result{};
        for (unsigned i = 0; i < N; ++i)
        {
            result.data[i] = lhs.data[i] - rhs.data[i];
        }
        return result;
    }

    template<typename T, std::size_t Dim > requires std::is_arithmetic_v<T>
    using Vec = vec<T, Dim>;

    template<typename T> using Vec2 = Vec<T, 2>;
    template<typename T> using Vec3 = Vec<T, 3>;

    using Vec2f = Vec<f32, 2>;
    using Vec2i = Vec<i32, 2>;
    using Vec2u = Vec<u32, 2>;
    using Vec3f = Vec<f32, 3>;
    using Vec3i = Vec<i32, 3>;
    using Vec3u = Vec<u32, 3>;
    using Vec4f = Vec<f32, 4>;
    using Vec4i = Vec<i32, 4>;
    using Vec4u = Vec<u32, 4>;

    template<typename T> requires std::is_arithmetic_v<T>
    struct Rect
    {
        using ValueType = T;
        using Vec2Type = vec<T, 2>;

        Vec2<T> Origin{ static_cast<T>(0), static_cast<T>(0) };
        Vec2<T> Size{ static_cast<T>(0), static_cast<T>(0) };

        constexpr T Width() const { return Size[0]; }
        constexpr T Height() const { return Size[1]; }
        constexpr T Top() const { return Origin[1]; }
        constexpr T Bottom() const { return Origin[1] + Size[1]; }
        constexpr T Left() const { return Origin[0]; }
        constexpr T Right() const { return Origin[0] + Size[0]; }

        constexpr Vec2<T> TopLeft() const { return Origin; }
        constexpr Vec2<T> TopRight() const { return Vec2<T>{ Origin[0] + Size[0], Origin[1] }; }
        constexpr Vec2<T> BottomLeft() const { return Vec2<T>{ Origin[0], Origin[1] + Size[1] }; }
        constexpr Vec2<T> BottomRight() const { return Origin + Size; }
        constexpr Vec2<T> Center() const { return Origin + Size * static_cast<T>(0.5); }

        constexpr Vec2<T> Min() const { return TopLeft(); }
        constexpr Vec2<T> Max() const { return BottomRight(); }

        constexpr bool Intersects(const Rect<T>& a_Other) const
        {
            return
                Left() < a_Other.Right() &&
                Right() > a_Other.Left() &&
                Top() < a_Other.Bottom() &&
                Bottom() > a_Other.Top();
        }

        constexpr Rect<T> Expanded(T a_Amount) const
        {
            return {
                Vec2<T>{ Origin[0] - a_Amount, Origin[1] - a_Amount },
                Vec2<T>{ Size[0] + static_cast<T>(2) * a_Amount, Size[1] + static_cast<T>(2) * a_Amount }
            };
        }

        constexpr Rect<T> Intersection(const Rect<T>& a_Other) const
        {
            if (!Intersects(a_Other))
                return Rect<T>{};

            Vec2f newMin{
                std::max(Left(), a_Other.Left()),
                std::max(Top(), a_Other.Top())
            };

            Vec2f newMax{
                std::min(Right(), a_Other.Right()),
                std::min(Bottom(), a_Other.Bottom())
            };

            return FromMinMax(newMin, newMax);
        }

        constexpr bool Contains(Vec2<T> a_Point) const
        {
            return a_Point[0] >= Left() && a_Point[0] <= Right() &&
                a_Point[1] >= Top() && a_Point[1] <= Bottom();
        }

        static constexpr Rect FromMinMax(Vec2<T> a_Min, Vec2<T> a_Max)
        {
            return { a_Min, a_Max - a_Min };
        }

        static constexpr Rect FromCenter(Vec2<T> a_Center, Vec2<T> a_Size)
        {
            Vec2<T> halfSize = a_Size * static_cast<T>(0.5);
            return { a_Center - halfSize, a_Size };
        }

        static constexpr Rect Infinite()
        {
            return {
                Vec2<T>{ std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest() },
                Vec2<T>{ std::numeric_limits<T>::max(), std::numeric_limits<T>::max() }
            };
        }
    };

    using Rectf = Rect<f32>;
    using Recti = Rect<i32>;
    using Rectu = Rect<u32>;

    template <class S, unsigned N>
    struct mat
    {
        /** Underlying column type. */
        using ColumnT = vec<S, N>;

        /** Number of rows/columns. */
        static constexpr int Size = N;

        ColumnT column[Size];

        constexpr mat() {};

        template<class... Args>
        static constexpr auto from_columns(const Args&... cols)
        {
            static_assert(sizeof...(cols) == N, "Wrong number of columns given.");
            return mat{ cols... };
        }

        template<class... Args>
        static constexpr mat from_rows(const Args&... rows)
        {
            static_assert(sizeof...(rows) == Size, "Wrong number of rows given.");
            mat result;
            for (unsigned i = 0u; i < N; ++i)
            {
                result.column[i] = ColumnT{ rows.data[i]... };
            }
            return result;
        }

        static constexpr mat identity()
        {
            mat result;
            for (unsigned i = 0u; i < N; ++i)
                for (unsigned j = 0u; j < N; ++j)
                    result.column[i].data[j] = (S)(i == j ? 1.0f : 0.0f);
            return result;
        }

        constexpr ColumnT& operator[](const unsigned i) { return column[i]; }

        constexpr const ColumnT& operator[](const int i) const { return column[i]; }

    private:
        template<class... Args>
        constexpr mat(const Args&... cols) : column{ cols... }
        {
            static_assert(sizeof...(cols) == N, "Wrong number of columns given.");
        }
    };

    template <class S>
    using mat2x2 = mat<S, 2>;

    using float2x2 = mat2x2<float>;

    template <class S>
    using mat3x3 = mat<S, 3>;

    using float3x3 = mat3x3<float>;

    template <class S>
    using mat4x4 = mat<S, 4>;

    using float4x4 = mat4x4<float>;

    template<typename T, std::size_t Dim> requires std::is_arithmetic_v<T>
    using Mat = mat<T, Dim>;

    template<typename T> using Mat2 = Mat<T, 2>;
    template<typename T> using Mat3 = Mat<T, 3>;
    template<typename T> using Mat4 = Mat<T, 4>;

    using Mat2f = Mat<f32, 2>;
    using Mat3f = Mat<f32, 3>;
    using Mat4f = Mat<f32, 4>;
    using Mat2i = Mat<i32, 2>;
    using Mat3i = Mat<i32, 3>;
    using Mat4i = Mat<i32, 4>;
    using Mat2u = Mat<u32, 2>;
    using Mat3u = Mat<u32, 3>;
    using Mat4u = Mat<u32, 4>;

    template<class S, unsigned N>
    constexpr vec<S, N> operator*(const mat<S, N>& lhs, const vec<S, N>& rhs)
    {
        vec<S, N> result{ (S)0.0 };
        for (unsigned c = 0; c < N; ++c)
        {
            for (unsigned i = 0; i < N; ++i)
                result.data[i] += lhs.column[c].data[i] * rhs.data[c];
        }
        return result;
    }

    template <class S, unsigned N>
    inline constexpr vec<S, N> operator/(const vec<S, N>& lhs, const vec<S, N>& rhs)
    {
        vec<S, N> result{};
        for (unsigned i = 0; i < N; ++i)
            result.data[i] = lhs.data[i] / rhs.data[i];
        return result;
    }

    template <class S, unsigned N>
    inline constexpr vec<S, N> operator*(const vec<S, N>& lhs, const S rhs)
    {
        vec<S, N> result = lhs;
        for (unsigned i = 0; i < N; ++i) result.data[i] *= rhs;
        return result;
    }

    template <class S, unsigned N>
    inline constexpr vec<S, N> operator*(const S lhs, const vec<S, N>& rhs)
    {
        return rhs * lhs;
    }

    template <class S, unsigned N>
    inline constexpr vec<S, N> operator/(const vec<S, N>& lhs, const S rhs)
    {
        const S inv = (S)1.0 / rhs;
        return lhs * inv;
    }

    template <class S, unsigned N>
    inline vec<S, N>& operator*=(vec<S, N>& v, const S s)
    {
        v = v * s;
        return v;
    }

    template <class S, unsigned N>
    inline vec<S, N>& operator/=(vec<S, N>& v, const S s)
    {
        v = v / s;
        return v;
    }

    template<class S, unsigned N>
    constexpr mat<S, N> operator*(const mat<S, N>& lhs, const mat<S, N>& rhs)
    {
        mat<S, N> result;
        for (unsigned c = 0; c < N; ++c)
        {
            for (unsigned r = 0; r < N; ++r)
            {
                result.column[c].data[r] = 0.0f;
                for (unsigned i = 0; i < N; ++i)
                {
                    result.column[c].data[r] +=
                        (lhs.column[i].data[r]) * (rhs.column[c].data[i]);
                }
            }
        }
        return result;
    }

    template<typename T> requires requires { T::identity(); }
    constexpr T c_Identity = T::identity();

    template <unsigned N, class S>
    inline constexpr S dot(const vec<S, N>& lhs, const vec<S, N>& rhs)
    {
        S result = (S)0.0f;
        for (unsigned i = 0u; i < N; ++i) result += lhs.data[i] * rhs.data[i];
        return result;
    }

    template <unsigned N, class S>
    inline constexpr S lengthsq(const vec<S, N>& v) { return dot(v, v); }

    template <unsigned N, class S>
    inline constexpr S length(const vec<S, N>& v)
    {
        return (S)std::sqrt(lengthsq(v));
    }

    template<typename... Types>
    using VariantImpl = std::variant<Types...>;

    using MonostateImpl = std::monostate;
      
    template<typename... Types>
    struct CoreTraits<VariantImpl<Types...>>
    {
        using Type = VariantImpl<Types...>;
        using SizeType = size_t;
            
        template<SizeType I>
		using ElementType = std::variant_alternative_t<I, Type>;

        static constexpr SizeType Index(const Type& a_Variant)
        {
            return a_Variant.index();
		}

        template<typename T>
        static constexpr bool Holds(const Type& a_Variant)
        {
            return std::holds_alternative<T>(a_Variant);
        }

        template<SizeType I>
        static constexpr bool Holds(const Type& a_Variant)
        {
			return std::holds_alternative<ElementType<I>>( a_Variant );
        }

        template<typename T>
        static constexpr T& Get(Type& a_Variant)
        {
            return std::get<T>(a_Variant);
        }

        template<typename T>
        static constexpr const T& Get(const Type& a_Variant)
        {
            return std::get<T>(a_Variant);
        }

        template<SizeType I>
        static constexpr auto& Get(Type& a_Variant)
        {
            return std::get<I>(a_Variant);
        }

        template<SizeType I>
        static constexpr const auto& Get(const Type& a_Variant)
        {
            return std::get<I>(a_Variant);
        }
    };

    template<typename Container>
    constexpr decltype(auto) Index(const Container& a_Container)
    {
        return CoreTraits<Container>::Index(a_Container);
    }

    template<typename T, typename Container>
    constexpr bool Holds(const Container& a_Container)
    {
        return CoreTraits<Container>::template Holds<T>(a_Container);
    }

    template<auto I, typename Container>
    constexpr bool Holds(const Container& a_Container)
    {
        return CoreTraits<Container>::template Holds<I>(a_Container);
    }

    template<typename T, typename Container>
    constexpr decltype(auto) Get(Container&& a_Container)
    {
        return CoreTraits<std::remove_cvref_t<Container>>::template Get<T>(std::forward<Container>(a_Container));
    }

    template<auto I, typename Container>
    constexpr decltype(auto) Get(Container&& a_Container)
    {
        return CoreTraits<std::remove_cvref_t<Container>>::template Get<I>(std::forward<Container>(a_Container));
    }
}