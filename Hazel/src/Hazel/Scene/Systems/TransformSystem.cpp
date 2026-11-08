#include "hzpch.h"
#include "TransformSystem.h"

#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Components/TransformComponent.h"

namespace Hazel
{
    void TransformSystem::Update(const Scene& scene)
    {
        // 当前采用每帧完整重建。
        // 已删除实体、已移除组件的数据也会随之清理。
        m_WorldTransforms.clear();  // 清楚所有之前保存的Entity的位置
        m_WorldTransforms.reserve(scene.GetEntityCount());

        scene.ForEach<TransformComponent>(
            [this](UUID id, const TransformComponent& transform)
            {
                WorldTransformData world;

                // 当前没有父子层级：
                // 世界矩阵 = 局部矩阵。
                world.Matrix = transform.GetLocalMatrix();
                world.Position = transform.Position;
                world.Rotation = transform.GetRotationQuaternion();

                m_WorldTransforms.emplace(id, world);
            });
    }

    const WorldTransformData& TransformSystem::GetWorldTransform(UUID id) const
    {
        return m_WorldTransforms.at(id);
    }

    void TransformSystem::Clear()
    {
        m_WorldTransforms.clear();
    }
}