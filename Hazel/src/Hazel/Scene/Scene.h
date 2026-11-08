#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Hazel/Core/Core.h"
#include "Hazel/Core/UUID.h"
#include "SceneEnvironment.h"

namespace Hazel
{
    class Entity;
    class HAZEL_API Scene
    {
        public:
            Scene() = default;
            ~Scene() = default;

            // Entity 内部保存 Scene*，因此 Scene 的地址保持固定。
            Scene(const Scene&) = delete;
            Scene& operator=(const Scene&) = delete;
            Scene(Scene&&) = delete;
            Scene& operator=(Scene&&) = delete;

            Entity CreateEntity(std::string name = "Entity");

            // 用于反序列化，恢复原来的实体身份。
            Entity CreateEntityWithUUID(UUID id, std::string name = "Entity");

            bool DestroyEntity(Entity entity);
            bool IsEntityValid(Entity entity) const;
            bool ContainsEntity(UUID id) const;

            Entity FindEntityByUUID(UUID id);

            // 返回 ID 快照，不暴露场景内部容器。
            std::vector<UUID> GetEntityIDs() const;

            std::size_t GetEntityCount() const noexcept
            {
                return m_Entities.size();
            }

            const std::string& GetEntityName(UUID id) const;
            void SetEntityName(UUID id, std::string name);

            bool IsEntityEnabled(UUID id) const;
            void SetEntityEnabled(UUID id, bool enabled);

            // 删除全部实体和组件，保留环境配置。
            void Clear();

            SceneEnvironment& GetEnvironment() noexcept
            {
                return m_Environment;
            }

            const SceneEnvironment& GetEnvironment() const noexcept
            {
                return m_Environment;
            }

        public:
            template<typename T, typename... Args>
            T& AddComponent(UUID id, Args&&... args)
            {
                RequireEntity(id);

                auto& components = GetOrCreateStorage<T>().Components;

                auto [it, inserted] = components.try_emplace(id, std::forward<Args>(args)...);

                if (!inserted)
                {
                    throw std::logic_error("Entity already has this component");
                }

                return it->second;
            }

            template<typename T>
            T& GetComponent(UUID id)
            {
                auto* storage = FindStorage<T>();

                if (!storage)
                {
                    throw std::out_of_range(
                    "Entity does not have this component");
                }

                return storage->Components.at(id);
            }

            template<typename T>
            const T& GetComponent(UUID id) const
            {
                const auto* storage = FindStorage<T>();

                if (!storage)
                {
                    throw std::out_of_range(
                    "Entity does not have this component");
                }

                return storage->Components.at(id);
            }

            template<typename T>
            T* TryGetComponent(UUID id)
            {
                auto* storage = FindStorage<T>();

                if (!storage)
                    return nullptr;

                auto it = storage->Components.find(id);

                return it != storage->Components.end() ? &it->second : nullptr;
            }

            template<typename T>
            const T* TryGetComponent(UUID id) const
            {
                const auto* storage = FindStorage<T>();

                if (!storage) return nullptr;

                auto it = storage->Components.find(id);

                return it != storage->Components.end() ? &it->second : nullptr;
            }

            template<typename T>
            bool HasComponent(UUID id) const
            {
                return TryGetComponent<T>(id) != nullptr;
            }

            template<typename T>
            bool RemoveComponent(UUID id)
            {
                auto* storage = FindStorage<T>();

                return storage && storage->Components.erase(id) != 0;
            }

            // 回调参数为 UUID 和 T&。
            // 遍历期间不要增删当前类型的组件或销毁实体。
            template<typename T, typename Func>
            void ForEach(Func&& function)
            {
                auto* storage = FindStorage<T>();

                if (!storage) return;

                for (auto& [id, component] : storage->Components)
                {
                    function(id, component);
                }
            }

            // 回调参数为 UUID 和 const T&。
            template<typename T, typename Func>
            void ForEach(Func&& function) const
            {
                const auto* storage = FindStorage<T>();

                if (!storage) return;

                for (const auto& [id, component] : storage->Components)
                {
                    function(id, component);
                }
            }

        private:
            struct EntityRecord
            {
                std::string Name;
                bool Enabled = true;
            };

            // 让销毁实体的操作能够统一通知所有组件容器。
            class IComponentStorage
            {
                public:
                    virtual ~IComponentStorage() = default;
                    virtual void Remove(UUID id) = 0;
            };

            template<typename T>
            class ComponentStorage final : public IComponentStorage
            {
                public:
                    void Remove(UUID id) override
                    {
                        Components.erase(id);
                    }

                    std::unordered_map<UUID, T> Components;
            };

            void RequireEntity(UUID id) const;

            template<typename T>
            ComponentStorage<T>& GetOrCreateStorage()
            {
                const std::type_index type{ typeid(T) };

                auto it = m_ComponentStorages.find(type);
                if (it != m_ComponentStorages.end())
                {
                    return static_cast<ComponentStorage<T>&>(*it->second);
                }

                auto storage = std::make_unique<ComponentStorage<T>>();
                auto* result = storage.get();
                m_ComponentStorages.emplace(type, std::move(storage));

                return *result;
            }

            template<typename T>
            ComponentStorage<T>* FindStorage()
            {
                const std::type_index type{ typeid(T) };
                auto it = m_ComponentStorages.find(type);
                if (it == m_ComponentStorages.end())
                    return nullptr;

                return static_cast<ComponentStorage<T>*>(it->second.get());
            }

            template<typename T>
            const ComponentStorage<T>* FindStorage() const
            {
                const std::type_index type{ typeid(T) };

                auto it = m_ComponentStorages.find(type);

                if (it == m_ComponentStorages.end())
                    return nullptr;

                return static_cast<const ComponentStorage<T>*>(it->second.get());
            }

        private:
            std::unordered_map<UUID, EntityRecord> m_Entities;
            // 按照组件类型寻找各种容器
            std::unordered_map<std::type_index, Scope<IComponentStorage>> m_ComponentStorages;
            SceneEnvironment m_Environment;
    };
}