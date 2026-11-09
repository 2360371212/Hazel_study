#pragma once

#include <cstdint>

#include <glm/glm.hpp>

#include "Hazel/Core/Core.h"
#include "Hazel/Renderer/Common/RenderState.h"
#include "Hazel/Renderer/Common/Shader.h"
#include "Hazel/Renderer/RenderView.h"
#include "Hazel/Renderer/Scene/RenderSceneData.h"

namespace Hazel
{
    struct Renderer3DSettings
    {
        bool ClearTarget = true;
        glm::vec4 ClearColor{
            0.035f,
            0.045f,
            0.065f,
            1.0f
        };

        float Exposure = 1.0f;
        float ReflectionStrength = 0.2f;
    };

    class HAZEL_API Renderer3D
    {
    public:
        static void Render(
            const RenderSceneData& scene,
            const RenderView& view,
            const Renderer3DSettings& settings = {});

    private:
        static void ApplyRenderState(
            const RenderState& state);

        static void UploadFrameParameters(
            Shader& shader,
            const RenderSceneData& scene,
            const RenderView& view,
            const Renderer3DSettings& settings);
    };
}
