#include "hzpch.h"
#include "UUID.h"

#include <random>

namespace Hazel
{
    namespace
    {
        UUID::ValueType GenerateUUIDValue()
        {
            // 每个线程维护自己的随机引擎。
            // 引擎只初始化一次，不在每次生成 ID 时重新播种。
            static thread_local std::mt19937_64 engine = []
            {
                std::random_device randomDevice;

                std::seed_seq seed{
                    randomDevice(),
                    randomDevice(),
                    randomDevice(),
                    randomDevice(),
                    randomDevice(),
                    randomDevice(),
                    randomDevice(),
                    randomDevice()
                };

                return std::mt19937_64(seed);
            }();

            UUID::ValueType value = 0;

            // 0 保留给无效 ID。
            do
            {
                value = engine();
            }
            while (value == 0);

            return value;
        }
    }

    UUID::UUID() : m_Value(GenerateUUIDValue()){}
}