#pragma once

#include <cstdint>

#include <glm/glm.hpp>

#include "Hazel/Camera/PerspectiveCamera.h"

namespace Hazel
{
    struct RenderView
    {
        glm::mat4 View{ 1.0f };
        glm::mat4 Projection{ 1.0f };
        glm::mat4 ViewProjection{ 1.0f };

        glm::vec3 Position{ 0.0f };

        uint32_t ViewportWidth = 0;
        uint32_t ViewportHeight = 0;

        static RenderView FromCamera(PerspectiveCamera& camera, uint32_t viewportWidth, uint32_t viewportHeight)
        {
            RenderView view;
            view.View = camera.GetViewMatrix();
            view.Projection = camera.GetProjectionMatrix();
            view.ViewProjection = camera.GetViewProjectionMatrix();
            view.Position = camera.GetPosition();
            view.ViewportWidth = viewportWidth;
            view.ViewportHeight = viewportHeight;
            return view;
        }
    };
}