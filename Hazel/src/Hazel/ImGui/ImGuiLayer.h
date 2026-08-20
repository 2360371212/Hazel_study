#pragma once
#include "Hazel/Layer.h"
#include "Hazel/Core.h"


namespace Hazel {
    class HAZEL_API ImGuiLayer : public Layer{
        public:
            ImGuiLayer();
            ~ImGuiLayer();

            void OnAttach() override;
            void OnDetach() override;
            void OnUpdate() override;
            void OnEvent(Event& event) override;
        private:
            float m_Time = 0;

    };
}