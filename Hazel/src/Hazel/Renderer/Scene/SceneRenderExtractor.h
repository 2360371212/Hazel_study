#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Systems/TransformSystem.h"
#include "RenderSceneData.h"

namespace Hazel
{
    // 从场景中找出需要渲染的对象，把分散在组件里的数据整理成渲染器可以直接使用的形式。
    class HAZEL_API SceneRenderExtractor
    {
        public:
            // 前置条件：
            // 1. transforms 已对同一个 scene 执行本帧 Update。
            // 2. Update 后没有继续修改变换或增删实体、组件。
            //
            // output 可以跨帧复用，每次提取会覆盖上一次结果。
            static void Extract(const Scene& scene, const TransformSystem& transforms, RenderSceneData& output);

        private:
            static void ExtractMeshes(const Scene& scene, const TransformSystem& transforms, RenderSceneData& output);
            static void ExtractLights(const Scene& scene, const TransformSystem& transforms, RenderSceneData& output);
            
            static void ExtractEnvironment(const Scene& scene, RenderSceneData& output);
    };
}