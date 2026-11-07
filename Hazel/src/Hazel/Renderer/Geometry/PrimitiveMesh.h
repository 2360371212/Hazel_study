#pragma once


#include "Hazel/Core/Core.h"
#include "Mesh.h"

namespace Hazel
{
    class PrimitiveMesh
    {
    public:
        static Ref<Mesh> CreateCube(float size = 1.0f);

        static Ref<Mesh> CreatePlane(float width = 1.0f, float depth = 1.0f);

        static Ref<Mesh> CreateSphere(float radius = 1.0f, uint32_t longitudeSegments = 64, uint32_t latitudeSegments = 32);
    };
}