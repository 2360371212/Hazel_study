#include "hzpch.h"
#include "EnvironmentMapLoader.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "stb_image.h"

namespace Hazel
{
    namespace
    {
        // 面内 s、t 均从 -1 到 1，按 Cubemap 纹理坐标约定映射到世界方向。
        glm::vec3 FaceDirection(CubeFace face, float s, float t)
        {
            switch (face)
            {
                case CubeFace::PositiveX: return { 1.0f, -t, -s };
                case CubeFace::NegativeX: return { -1.0f, -t, s };
                case CubeFace::PositiveY: return { s, 1.0f, t };
                case CubeFace::NegativeY: return { s, -1.0f, -t };
                case CubeFace::PositiveZ: return { s, -t, 1.0f };
                case CubeFace::NegativeZ: return { -s, -t, -1.0f };
            }
            return {};
        }

        glm::vec4 SamplePanorama(const float* pixels, int width, int height, const glm::vec3& direction)
        {
            const glm::vec3 d = glm::normalize(direction);
            const float u = std::atan2(d.z, d.x) / glm::two_pi<float>() + 0.5f;
            const float v = std::asin(std::clamp(d.y, -1.0f, 1.0f)) / glm::pi<float>() + 0.5f;
            const float x = u * width - 0.5f;
            const float y = v * height - 0.5f;
            const int x0 = static_cast<int>(std::floor(x));
            const int y0 = static_cast<int>(std::floor(y));
            const auto texel = [&](int column, int row)
            {
                column = (column % width + width) % width;
                row = std::clamp(row, 0, height - 1);
                const float* p = pixels + (static_cast<std::size_t>(row) * width + column) * 4;
                return glm::vec4(p[0], p[1], p[2], p[3]);
            };
            return glm::mix(
                glm::mix(texel(x0, y0), texel(x0 + 1, y0), x - x0),
                glm::mix(texel(x0, y0 + 1), texel(x0 + 1, y0 + 1), x - x0),
                y - y0);
        }
    }

    Ref<TextureCube> EnvironmentMapLoader::LoadHDR(const std::string& path, uint32_t faceSize)
    {
        if (faceSize == 0)
            throw std::invalid_argument("Cubemap face size must be nonzero");
        if (!stbi_is_hdr(path.c_str()))
            throw std::invalid_argument("Expected a Radiance HDR environment: " + path);

        // 与天空原有全景采样约定一致：V 向上增大。
        stbi_set_flip_vertically_on_load_thread(1);
        int width = 0, height = 0, channels = 0;
        std::unique_ptr<float, decltype(&stbi_image_free)> pixels(
            stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb_alpha),
            &stbi_image_free);
        if (!pixels)
            throw std::runtime_error("Failed to load HDR environment: " + path);

        TextureSpecification specification;
        specification.Width = faceSize;
        specification.Height = faceSize;
        specification.Format = TextureFormat::RGBA16F;
        specification.Wrap = TextureWrap::ClampToEdge;
        specification.MipLevels = 1;
        auto cubemap = TextureCube::Create(specification);

        std::vector<float> facePixels(static_cast<std::size_t>(faceSize) * faceSize * 4);
        for (uint32_t faceIndex = 0; faceIndex < 6; ++faceIndex)
        {
            const auto face = static_cast<CubeFace>(faceIndex);
            for (uint32_t y = 0; y < faceSize; ++y)
            {
                for (uint32_t x = 0; x < faceSize; ++x)
                {
                    const float s = 2.0f * (x + 0.5f) / faceSize - 1.0f;
                    const float t = 2.0f * (y + 0.5f) / faceSize - 1.0f;
                    const glm::vec4 color = SamplePanorama(
                        pixels.get(), width, height, FaceDirection(face, s, t));
                    const std::size_t offset = (static_cast<std::size_t>(y) * faceSize + x) * 4;
                    facePixels[offset] = color.r;
                    facePixels[offset + 1] = color.g;
                    facePixels[offset + 2] = color.b;
                    facePixels[offset + 3] = 1.0f;
                }
            }
            cubemap->SetFaceData(face, facePixels.data(), facePixels.size() * sizeof(float));
        }
        return cubemap;
    }
}
