#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Renderer/Common/VertexArray.h"

namespace Hazel
{
    class HAZEL_API FullscreenTriangle
    {
    public:
        FullscreenTriangle();
        void Draw() const;
    private:
        Ref<VertexArray> m_VertexArray;
    };
}