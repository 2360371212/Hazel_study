#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

#include "Hazel/Core/Core.h"

namespace Hazel
{
    class HAZEL_API UUID
    {
    public:
        using ValueType = std::uint64_t;

    public:
        // 创建一个新的非零 ID。
        UUID();

        // 从已有数值恢复 ID；允许传入 0 表示无效 ID。
        explicit constexpr UUID(ValueType value) noexcept
            : m_Value(value)
        {
        }

        constexpr ValueType GetValue() const noexcept
        {
            return m_Value;
        }

        constexpr bool IsValid() const noexcept
        {
            return m_Value != 0;
        }

        constexpr bool operator==(const UUID& other) const noexcept
        {
            return m_Value == other.m_Value;
        }

        constexpr bool operator!=(
            const UUID& other) const noexcept
        {
            return !(*this == other);
        }

    private:
        ValueType m_Value = 0;
    };
}

// 特化类型
namespace std
{
    template<>
    struct hash<Hazel::UUID>
    {
        std::size_t operator()(const Hazel::UUID& uuid) const noexcept
        {
            // 调用std::hash<uint64_t>{}创建一个对象hashFunc, 然后再调用hashFunc(uuid.GetValue)
            return std::hash<Hazel::UUID::ValueType>{}(uuid.GetValue());
        }
    };
}