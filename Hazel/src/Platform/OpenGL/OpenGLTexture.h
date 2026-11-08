#pragma once
#include "Hazel/Renderer/Common/Texture.h"

namespace Hazel{
    class OpenGLTexture2D final : public Texture2D{
        public:
            OpenGLTexture2D(const std::string& path);
            OpenGLTexture2D(const std::string& path, const TextureLoadOptions& options);
            // 兼容已有的后端加载入口，同样委托给 TextureLoadOptions。
            OpenGLTexture2D(const std::string& path, bool sRGB, bool flipVertically);
            OpenGLTexture2D(uint32_t width, uint32_t height);
            OpenGLTexture2D(const TextureSpecification& specification);

            virtual ~OpenGLTexture2D() override;
            virtual uint32_t GetWidth() const override;
            virtual uint32_t GetHeight() const override;
            virtual uint32_t GetRendererID() const override;
            virtual const TextureSpecification& GetSpecification() const override;
            virtual void GenerateMipmaps() override;

            virtual void Bind(uint32_t slot) const override;

            virtual void SetData(const void* data, std::size_t size) override;

        private:
            TextureSpecification m_Specification;
            uint32_t m_RendererID = 0;
    };

    class OpenGLTextureCube final : public TextureCube
    {
        public:
            OpenGLTextureCube(const TextureSpecification& specification);
            ~OpenGLTextureCube() override;

            uint32_t GetWidth() const override
            {
                return m_Specification.Width;
            }

            uint32_t GetHeight() const override
            {
                return m_Specification.Height;
            }

            uint32_t GetRendererID() const override
            {
                return m_RendererID;
            }

            const TextureSpecification& GetSpecification() const override
            {
                return m_Specification;
            }

            void SetData(const void* data, std::size_t size) override;

            void SetFaceData(CubeFace face, const void* data, std::size_t size, uint32_t mipLevel = 0) override;

            void GenerateMipmaps() override;

            void Bind(uint32_t slot = 0) const override;

        private:
            TextureSpecification m_Specification;
            uint32_t m_RendererID = 0;
    };
}
