#pragma once

#include <string>
#include <utility>

#include "Hazel/Core/Core.h"
#include "Hazel/Core/UUID.h"
#include "Scene.h"

namespace Hazel
{
    class HAZEL_API Entity
    {
        public:
            Entity() = default;

            UUID GetUUID() const noexcept
            {
                return m_UUID;
            }

            Scene* GetScene() const noexcept
            {
                return m_Scene;
            }

            bool IsValid() const;

            explicit operator bool() const
            {
                return IsValid();
            }

            const std::string& GetName() const;
            void SetName(std::string name);

            bool IsEnabled() const;
            void SetEnabled(bool enabled);

            bool operator==(const Entity& other) const noexcept
            {
                return m_UUID == other.m_UUID && m_Scene == other.m_Scene;
            }

            bool operator!=(const Entity& other) const noexcept
            {
                return !(*this == other);
            }

        public:
            template<typename T, typename... Args>
            T& AddComponent(Args&&... args)
            {
                // 下面的组件跟随Entity的UUID
                return RequireScene().AddComponent<T>(m_UUID, std::forward<Args>(args)...);
            }

            template<typename T>
            T& GetComponent()
            {
                return RequireScene().GetComponent<T>(m_UUID);
            }

            template<typename T>
            const T& GetComponent() const
            {
                const Scene& scene = RequireScene();
                return scene.GetComponent<T>(m_UUID);
            }

            template<typename T>
            T* TryGetComponent()
            {
                return m_Scene ? m_Scene->TryGetComponent<T>(m_UUID) : nullptr;
            }

            template<typename T>
            const T* TryGetComponent() const
            {
                if (!m_Scene) return nullptr;
                const Scene& scene = *m_Scene;
                return scene.TryGetComponent<T>(m_UUID);
            }

            template<typename T>
            bool HasComponent() const
            {
                return TryGetComponent<T>() != nullptr;
            }

            template<typename T>
            bool RemoveComponent()
            {
                return m_Scene && m_Scene->RemoveComponent<T>(m_UUID);
            }

        private:
            friend class Scene;

            // 构造函数私有化
            Entity(UUID id, Scene* scene) noexcept 
                : m_UUID(id), m_Scene(scene) {}

            Scene& RequireScene() const;

        private:
            UUID m_UUID{ 0 };
            Scene* m_Scene = nullptr;
    };
}