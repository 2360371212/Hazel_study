#include "hzpch.h"
#include "OpenGLTexture.h"
#include "stb_image.h"
#include <glad/glad.h>
#include "Hazel/Core/Log.h"
#include <algorithm>
#include <memory>
#include <stdexcept>

namespace Hazel{

    namespace
    {
        struct GLTextureFormatInfo
        {
            GLenum InternalFormat;
            GLenum DataFormat;
            GLenum DataType;
            std::size_t UploadBytesPerPixel;
        };

        GLTextureFormatInfo GetFormatInfo(TextureFormat format)
        {
            switch (format)
            {
                case TextureFormat::RGB8:
                    return {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, 3};
                case TextureFormat::RGBA8:
                    return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 4};
                case TextureFormat::SRGBA8:
                    return {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE, 4};
                case TextureFormat::RG16F:
                    return {GL_RG16F, GL_RG, GL_FLOAT, 2 * sizeof(float)};
                case TextureFormat::RGB16F:
                    return {GL_RGB16F, GL_RGB, GL_FLOAT, 3 * sizeof(float)};
                case TextureFormat::RGBA16F:
                    return {GL_RGBA16F, GL_RGBA, GL_FLOAT, 4 * sizeof(float)};
                case TextureFormat::Depth24Stencil8:
                    return {GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 4};
                case TextureFormat::Depth32F:
                    return {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, sizeof(float)};
            }
            
            HZ_CORE_ASSERT(false, "Unsupported texture format");
        }

        GLenum GetWrapMode(TextureWrap wrap)
        {
            switch (wrap)
            {
                case TextureWrap::Repeat:
                    return GL_REPEAT;
                case TextureWrap::ClampToEdge:
                    return GL_CLAMP_TO_EDGE;
            }
            HZ_CORE_ASSERT(false, "Unsupported texture wrap mode")
        }

        uint32_t CalculateFullMipCount(uint32_t width, uint32_t height)
        {
            uint32_t levels = 1;
            uint32_t size = (std::max)(width, height);
            while (size > 1)
            {
                size >>= 1;
                ++levels;
            }
            return levels;
        }

        void ValidateSpecification(TextureSpecification& specification, bool isCube)
        {
            if (specification.Width == 0 || specification.Height == 0)
            {
                throw std::invalid_argument("Texture dimensions must be greater than zero");
            }

            if (isCube && specification.Width != specification.Height)
            {
                throw std::invalid_argument("Cubemap faces must be square");
            }

            const uint32_t maxMipCount = CalculateFullMipCount(specification.Width, specification.Height);

            if (specification.MipLevels == 0)
                specification.MipLevels = maxMipCount;

            if (specification.MipLevels > maxMipCount)
            {
                throw std::invalid_argument("Too many mip levels for texture dimensions");
            }
        }

        void CreateTextureStorage(GLenum target, TextureSpecification& specification, uint32_t& rendererID)
        {
            const bool isCube = target == GL_TEXTURE_CUBE_MAP;
            ValidateSpecification(specification, isCube);
            const auto format = GetFormatInfo(specification.Format);
            const GLenum wrap = GetWrapMode(specification.Wrap);

            glCreateTextures(target, 1, &rendererID);

            glTextureStorage2D(
                rendererID,
                specification.MipLevels,
                format.InternalFormat,
                specification.Width,
                specification.Height);

            const GLenum minFilter = specification.MipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;

            glTextureParameteri(rendererID, GL_TEXTURE_MIN_FILTER, minFilter);
            glTextureParameteri(rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(rendererID, GL_TEXTURE_WRAP_S, wrap);
            glTextureParameteri(rendererID, GL_TEXTURE_WRAP_T, wrap);

            if (isCube)
            {
                glTextureParameteri(rendererID, GL_TEXTURE_WRAP_R, wrap);
            }

            glTextureParameteri(rendererID, GL_TEXTURE_BASE_LEVEL, 0);
            glTextureParameteri(rendererID, GL_TEXTURE_MAX_LEVEL, specification.MipLevels - 1);
        }

        void ValidateUpload(const void* data, std::size_t size, std::size_t expectedSize)
        {
            if (!data || size != expectedSize)
            {
                HZ_CORE_ASSERT(false, "Texture upload requires complete, tightly packed data")
            }
        }

        // 上传紧密排列的 CPU 像素，并恢复原来的对齐状态。
        class ScopedUnpackAlignment
        {
            public:
                ScopedUnpackAlignment()
                {
                    glGetIntegerv(GL_UNPACK_ALIGNMENT, &m_PreviousAlignment);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                }

                ~ScopedUnpackAlignment()
                {
                    glPixelStorei(GL_UNPACK_ALIGNMENT, m_PreviousAlignment);
                }
            private:
                GLint m_PreviousAlignment = 4;
        };
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
        : OpenGLTexture2D(path, TextureLoadOptions{})
    {
    }

    OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
        : OpenGLTexture2D(TextureSpecification{
            width, height, TextureFormat::RGBA8, 1, TextureWrap::Repeat})
    {
    }

    OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification)
        : m_Specification(specification) 
    {
        CreateTextureStorage(GL_TEXTURE_2D, m_Specification, m_RendererID);
    }


    OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool sRGB, bool flipVertically)
        : OpenGLTexture2D(path, TextureLoadOptions{sRGB, flipVertically, 0})
    {
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string& path, const TextureLoadOptions& options)
    {
        const bool isHDR = stbi_is_hdr(path.c_str()) != 0;

        if (isHDR && options.SRGB)
        {
            throw std::invalid_argument("HDR textures must use a linear format: " + path);
        }

        stbi_set_flip_vertically_on_load_thread(options.FlipVertically ? 1 : 0);

        int width = 0;
        int height = 0;
        int channels = 0;

        void* loadedData = nullptr;

        if (isHDR)
        {
            loadedData = stbi_loadf(
                path.c_str(),
                &width,
                &height,
                &channels,
                STBI_rgb_alpha);
        }
        else
        {
            loadedData = stbi_load(
                path.c_str(),
                &width,
                &height,
                &channels,
                STBI_rgb_alpha);
        }

        // 无论后续是否抛异常，都释放 stb_image 分配的内存。
        std::unique_ptr<void, decltype(&stbi_image_free)> pixels(loadedData, &stbi_image_free);

        if (!pixels)
        {
            HZ_CORE_ASSERT(false, "Failed to load texture: " + path)
        }

        m_Specification.Width = static_cast<uint32_t>(width);
        m_Specification.Height = static_cast<uint32_t>(height);
        m_Specification.Format = isHDR ? TextureFormat::RGBA16F : (options.SRGB ? TextureFormat::SRGBA8 : TextureFormat::RGBA8);
        m_Specification.Wrap = options.Wrap;
        m_Specification.MipLevels = options.MipLevels;

        CreateTextureStorage(GL_TEXTURE_2D, m_Specification, m_RendererID);
        const auto format = GetFormatInfo(m_Specification.Format);
        const std::size_t dataSize = static_cast<std::size_t>(GetWidth()) * GetHeight() * format.UploadBytesPerPixel;
        SetData(pixels.get(), dataSize);
        GenerateMipmaps();
    }

    void OpenGLTexture2D::SetData(const void* data, std::size_t size)
    {
        const auto format = GetFormatInfo(GetFormat());
        const std::size_t expectedSize = static_cast<std::size_t>(GetWidth()) * GetHeight() * format.UploadBytesPerPixel;
        ValidateUpload(data, size, expectedSize);

        ScopedUnpackAlignment alignment;
        glTextureSubImage2D(
            m_RendererID, 0, 0, 0, GetWidth(), GetHeight(),
            format.DataFormat, format.DataType, data);
    }

    OpenGLTexture2D::~OpenGLTexture2D(){
        glDeleteTextures(1, &m_RendererID);
    }

    uint32_t OpenGLTexture2D::GetWidth() const{
        return m_Specification.Width;
    } 

    uint32_t OpenGLTexture2D::GetHeight() const{
        return m_Specification.Height;
    }

    uint32_t OpenGLTexture2D::GetRendererID() const{
        return m_RendererID;
    } 
    const TextureSpecification& OpenGLTexture2D::GetSpecification() const{
        return m_Specification;
    }
    void OpenGLTexture2D::GenerateMipmaps()
    {
        if (GetMipLevelCount() > 1)
            glGenerateTextureMipmap(m_RendererID);
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const{
        glBindTextureUnit(slot, m_RendererID);
    }

    // --------------------------------------------------------------------------------------------

    OpenGLTextureCube::OpenGLTextureCube(const TextureSpecification& specification)
        : m_Specification(specification)
    {
        CreateTextureStorage(GL_TEXTURE_CUBE_MAP, m_Specification, m_RendererID);
    }

    OpenGLTextureCube::~OpenGLTextureCube()
    {
        glDeleteTextures(1, &m_RendererID);
    }

    void OpenGLTextureCube::SetData(const void* data, std::size_t size)
    {
        const auto format = GetFormatInfo(m_Specification.Format);
        const std::size_t faceSize = static_cast<std::size_t>(GetWidth()) * GetHeight() * format.UploadBytesPerPixel;
        ValidateUpload(data, size, faceSize * 6);
        const auto* bytes = static_cast<const std::uint8_t*>(data);

        for (uint32_t face = 0; face < 6; ++face)
        {
            SetFaceData(static_cast<CubeFace>(face), bytes + face * faceSize, faceSize, 0);
        }
    }

    void OpenGLTextureCube::SetFaceData(CubeFace face, const void* data, std::size_t size, uint32_t mipLevel)
    {
        const uint32_t faceIndex = static_cast<uint32_t>(face);

        if (faceIndex >= 6 || mipLevel >= GetMipLevelCount())
        {
            throw std::out_of_range("Invalid cubemap face or mip level");
        }
        const uint32_t mipSize = (std::max)(1u, GetWidth() >> mipLevel);

        const auto format = GetFormatInfo(m_Specification.Format);

        const std::size_t expectedSize = static_cast<std::size_t>(mipSize) * mipSize * format.UploadBytesPerPixel;

        ValidateUpload(data, size, expectedSize);

        ScopedUnpackAlignment alignment;

        glTextureSubImage3D(
            m_RendererID,
            mipLevel,
            0,
            0,
            faceIndex,
            mipSize,
            mipSize,
            1,
            format.DataFormat,
            format.DataType,
            data);
    }

    void OpenGLTextureCube::GenerateMipmaps()
    {
        if (GetMipLevelCount() > 1) glGenerateTextureMipmap(m_RendererID);
    }

    void OpenGLTextureCube::Bind(uint32_t slot) const
    {
        glBindTextureUnit(slot, m_RendererID);
    }
}
