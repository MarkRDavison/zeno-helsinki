#pragma once


#define HELSINKI_NODISCARD [[nodiscard]]

#define HELSINKI_TRY_EXPR( _Expr ) \
    if constexpr ( requires { _Expr; } ) \
    { \
        return _Expr; \
    }

#define HELSINKI_ENUM_ENABLE_BITMASK_OPERATORS( EnumName, UnderlyingType ) \
    enum class EnumName : UnderlyingType; \
    inline constexpr EnumName operator|( EnumName a, EnumName b ) \
    { \
        return static_cast<EnumName>( static_cast<UnderlyingType>( a ) | static_cast<UnderlyingType>( b ) ); \
    } \
    inline constexpr EnumName operator&( EnumName a, EnumName b ) \
    { \
        return static_cast<EnumName>( static_cast<UnderlyingType>( a ) & static_cast<UnderlyingType>( b ) ); \
    } \
    inline constexpr EnumName operator^( EnumName a, EnumName b ) \
    { \
        return static_cast<EnumName>( static_cast<UnderlyingType>( a ) ^ static_cast<UnderlyingType>( b ) ); \
    } \
    inline constexpr EnumName operator~( EnumName a ) \
    { \
        return static_cast<EnumName>( ~static_cast<UnderlyingType>( a ) ); \
    } \
    inline constexpr EnumName& operator|=( EnumName& a, EnumName b ) \
    { \
        return a = a | b; \
    } \
    inline constexpr EnumName& operator&=( EnumName& a, EnumName b ) \
    { \
        return a = a & b; \
    } \
    inline constexpr EnumName& operator^=( EnumName& a, EnumName b ) \
    { \
        return a = a ^ b; \
    }
