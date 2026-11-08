#include "hzpch.h"
#include "Scene.h"

#include <stdexcept>
#include <utility>
#include "Hazel/Core/Log.h"

#include "Entity.h"

namespace Hazel
{
    Entity Scene::CreateEntity(std::string name)
    {
        UUID id;

        // UUID 生成器负责提供候选值，
        // 场景负责保证当前场景中没有重复 ID。
        while (ContainsEntity(id)) 
            id = UUID();

        return CreateEntityWithUUID(id, std::move(name));
    }

    Entity Scene::CreateEntityWithUUID(UUID id, std::string name)
    {
        if (!id.IsValid())
        {
            HZ_CORE_ERROR("Entity UUID must not be zero");
        }

        auto [it, inserted] = m_Entities.try_emplace(id, EntityRecord{std::move(name), true});

        if (!inserted)
        {
            HZ_CORE_ERROR("Entity UUID already exists in this scene");
        }
        // 友元关系直接调用私有构造函数
        return Entity(id, this);
    }

    bool Scene::DestroyEntity(Entity entity)
    {
        if (!IsEntityValid(entity))
            return false;

        const UUID id = entity.GetUUID();

        for (auto& entry : m_ComponentStorages)
            entry.second->Remove(id);

        m_Entities.erase(id);

        return true;
    }

    bool Scene::IsEntityValid(Entity entity) const
    {
        return
            entity.GetScene() == this &&
            ContainsEntity(entity.GetUUID());
    }

    bool Scene::ContainsEntity(UUID id) const
    {
        return m_Entities.find(id) != m_Entities.end();
    }

    Entity Scene::FindEntityByUUID(UUID id)
    {
        if (!ContainsEntity(id))
            return {};

        return Entity(id, this);
    }

    std::vector<UUID> Scene::GetEntityIDs() const
    {
        std::vector<UUID> ids;

        ids.reserve(m_Entities.size());

        for (const auto& entry : m_Entities)
            ids.push_back(entry.first);

        return ids;
    }

    const std::string& Scene::GetEntityName(
        UUID id) const
    {
        return m_Entities.at(id).Name;
    }

    void Scene::SetEntityName(
        UUID id,
        std::string name)
    {
        m_Entities.at(id).Name = std::move(name);
    }

    bool Scene::IsEntityEnabled(UUID id) const
    {
        return m_Entities.at(id).Enabled;
    }

    void Scene::SetEntityEnabled(
        UUID id,
        bool enabled)
    {
        m_Entities.at(id).Enabled = enabled;
    }

    void Scene::Clear()
    {
        // 先释放组件持有的资源，再删除实体记录。
        m_ComponentStorages.clear();
        m_Entities.clear();
    }

    void Scene::RequireEntity(UUID id) const
    {
        if (!ContainsEntity(id))
        {
            throw std::out_of_range(
                "Entity does not exist in this scene");
        }
    }
}