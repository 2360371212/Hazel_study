#pragma once
#include <string>
#include "Hazel/Core/Core.h"


namespace Hazel{

    enum class TextureFormat
    {
        RGB8, RGBA8, SRGBA8, RG16F, RGB16F, RGBA16F
    };

    enum class TextureWrap
    {
        Repeat, ClampToEdge
    };

    enum class CubeFace : uint32_t
    {
        PositiveX = 0, NegativeX, PositiveY, NegativeY, PositiveZ, NegativeZ
    };

    struct TextureSpecification
    {
        uint32_t Width = 1;
        uint32_t Height = 1;

        TextureFormat Format = TextureFormat::RGBA8;

        // 1：仅基础层。
        // 0：分配完整 mip 链。
        // 其他值：分配指定数量的层级。
        uint32_t MipLevels = 1;

        TextureWrap Wrap = TextureWrap::ClampToEdge;
    };

    class Texture{
        public:
            Texture() = default;
            virtual ~Texture() = default;

            // GPU 资源通过 Ref 共享，不直接复制纹理对象。
            Texture(const Texture&) = delete;
            Texture& operator=(const Texture&) = delete;

            virtual uint32_t GetWidth() const = 0;
            virtual uint32_t GetHeight() const = 0;

            virtual uint32_t GetRendererID() const = 0;

            virtual const TextureSpecification& GetSpecification() const = 0;

            uint32_t GetMipLevelCount() const
            {
                return GetSpecification().MipLevels;
            }

            TextureFormat GetFormat() const
            {
                return GetSpecification().Format;
            }

            // 上传基础层数据。
            // Texture2D：一张图像。
            // TextureCube：按 CubeFace 顺序紧密排列的六张图像。
            virtual void SetData(const void* data, uint32_t size) = 0;

            // 根据基础层生成普通 mip。
            virtual void GenerateMipmaps() = 0;

            virtual void Bind(uint32_t slot) const = 0;

            virtual bool operator==(const Texture& other) const
            {
                return GetRendererID() == other.GetRendererID();
            }
    };

    class Texture2D : public Texture{
        public:
            static Ref<Texture2D> Create(const std::string& path);

            static Ref<Texture2D> Create(uint32_t width, uint32_t height);

            static Ref<Texture2D> Create(const TextureSpecification& specification);
    };

    // CubeMap
    class TextureCube : public Texture
    {
        public:
            // Width 和 Height 必须相等。
            static Ref<TextureCube> Create(const TextureSpecification& specification);

            // 上传指定面、指定 mip。
            virtual void SetFaceData(CubeFace face, const void* data, std::size_t size, uint32_t mipLevel = 0) = 0;
    };


}