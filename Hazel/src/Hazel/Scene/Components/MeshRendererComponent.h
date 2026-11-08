#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Renderer/Geometry/Mesh.h"
#include "Hazel/Renderer/Material/MaterialInstance.h"

namespace Hazel
{
    struct MeshRendererComponent
    {
        // 网格资源
        Ref<Mesh> MeshResource;
        // 网格对应的材质
        Ref<MaterialInstance> Material;

        bool Visible = true;
    };
}