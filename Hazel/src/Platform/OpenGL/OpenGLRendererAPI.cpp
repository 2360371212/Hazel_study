#include "hzpch.h"
#include "OpenGLRendererAPI.h"
#include "glad/glad.h"

namespace Hazel{
    void OpenGLRendererAPI::SetClearColor(const glm::vec4& color){
        glClearColor(color.r, color.g, color.b, color.a);
    }
    void OpenGLRendererAPI::Clear(){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    } 
    
    void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

    void OpenGLRendererAPI::Init() {
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
    }

    void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height){
        glViewport(x, y, width, height);
    }


    void OpenGLRendererAPI::SetSRGBWrite(bool enabled){
        if (enabled)
            glEnable(GL_FRAMEBUFFER_SRGB);
        else
            glDisable(GL_FRAMEBUFFER_SRGB);
    }

    void OpenGLRendererAPI::SetScissorTest(bool enabled){
        if (enabled)
            glEnable(GL_SCISSOR_TEST);
        else
            glDisable(GL_SCISSOR_TEST);
    }

    void OpenGLRendererAPI::SetColorWrite(bool red, bool green, bool blue, bool alpha){
        glColorMask(
        red   ? GL_TRUE : GL_FALSE,
        green ? GL_TRUE : GL_FALSE,
        blue  ? GL_TRUE : GL_FALSE,
        alpha ? GL_TRUE : GL_FALSE);
    }

    void OpenGLRendererAPI::SetDepthWrite(bool enabled){
        glDepthMask(enabled ? GL_TRUE : GL_FALSE);
    }

    void OpenGLRendererAPI::SetClearDepth(float depth)
    {
        glClearDepth(static_cast<GLdouble>(depth));
    }

    void OpenGLRendererAPI::SetRenderState(const RenderState& state)
    {
        if (state.DepthTest)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);

        glDepthMask(state.DepthWrite ? GL_TRUE : GL_FALSE);

        switch (state.Cull)
        {
            case CullMode::None:
                glDisable(GL_CULL_FACE);
                break;
            case CullMode::Front:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
                break;
            case CullMode::Back:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                break;
        }

        switch (state.Blend)
        {
            case BlendMode::Opaque:
                glDisable(GL_BLEND);
                break;
            case BlendMode::AlphaBlend:
                glEnable(GL_BLEND);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;

            case BlendMode::Additive:
                glEnable(GL_BLEND);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                break;
        }
    }
}
