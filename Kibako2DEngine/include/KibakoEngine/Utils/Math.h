#pragma once

#include <cmath>
#include <type_traits>

namespace KibakoEngine::Math {

    template <typename T>
    [[nodiscard]] constexpr T Clamp(const T& value, const T& minValue, const T& maxValue)
    {
        return value < minValue ? minValue : (value > maxValue ? maxValue : value);
    }

    template <typename T>
    [[nodiscard]] constexpr T Lerp(const T& a, const T& b, float t)
    {
        return static_cast<T>(a + (b - a) * t);
    }

    template <typename T>
    [[nodiscard]] constexpr T Saturate(const T& value)
    {
        return Clamp(value, T(0), T(1));
    }

    [[nodiscard]] inline float ToRadians(float degrees)
    {
        return degrees * (3.1415926535f / 180.0f);
    }

    [[nodiscard]] inline float ToDegrees(float radians)
    {
        return radians * (180.0f / 3.1415926535f);
    }

    template <typename T>
    [[nodiscard]] inline T Wrap(T value, T minValue, T maxValue)
    {
        const T range = maxValue - minValue;
        if (range == T(0))
            return minValue;
        while (value < minValue) value += range;
        while (value >= maxValue) value -= range;
        return value;
    }

} // namespace KibakoEngine::Math

