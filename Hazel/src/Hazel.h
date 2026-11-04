#pragma once
// Logging
#include "Hazel/Core/Log.h"
#include "Hazel/Core/Layer.h"
#include "Hazel/ImGui/ImGuiLayer.h"

#include "Hazel/Input/Input.h"
#include "Hazel/Input/KeyCodes.h"
#include "Hazel/Input/MouseButtonCodes.h"

// For use by Hazel applications
#include "Hazel/Core/Application.h"

//Render
#include "Hazel/Renderer/Common/Buffer.h"
#include "Hazel/Renderer/Common/VertexArray.h"
#include "Hazel/Renderer/Common/Shader.h"
#include "Hazel/Renderer/Common/Texture.h"
#include "Hazel/Renderer/Common/RenderCommand.h"
#include "Hazel/Renderer/Common/RendererAPI.h"
#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Camera/OrthographicCameraController.h"
#include "Hazel/Camera/FlyCameraController.h"

#include "Hazel/Core/Timestep.h"

// OpenGL
// #include "Platform/OpenGL/OpenGLBuffer.h"
// #include "Platform/OpenGL/OpenGLVertexArray.h"
