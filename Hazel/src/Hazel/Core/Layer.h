#pragma once


#include "Hazel/Core/Core.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Core/Timestep.h"


namespace Hazel{
    class HAZEL_API Layer{
        public:
            Layer(const std::string& debugName = "Layer");
            virtual ~Layer();

            virtual void OnAttach(){}
            virtual void OnDetach(){}
            virtual void OnUpdate(Timestep timestep){}
            virtual void OnImGuiRender(){}
            virtual void OnEvent(Event& event){}

            inline const std::string& GetName() const {return m_DebugName;}
        private:
            std::string m_DebugName;
    };
}

