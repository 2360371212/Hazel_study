#include "hzpch.h"
#include "Entity.h"
#include "Hazel/Core/Log.h"
#include <stdexcept>
#include <utility>

namespace Hazel
{
    bool Entity::IsValid() const
    {
        return m_Scene && m_Scene->ContainsEntity(m_UUID);
    }

    // 获取之前的前提是场景被创建而且绑定该实例
    const std::string& Entity::GetName() const
    {
        return RequireScene().GetEntityName(m_UUID);
    }

    void Entity::SetName(std::string name)
    {
        RequireScene().SetEntityName(m_UUID,std::move(name));
    }

    bool Entity::IsEnabled() const
    {
        return RequireScene().IsEntityEnabled(m_UUID);
    }

    void Entity::SetEnabled(bool enabled)
    {
        RequireScene().SetEntityEnabled(m_UUID, enabled);
    }

    Scene& Entity::RequireScene() const
    {
        if (!m_Scene)
        {
            HZ_CORE_WARN("Entity does not belong to a scene");
        }
        return *m_Scene;
    }
}