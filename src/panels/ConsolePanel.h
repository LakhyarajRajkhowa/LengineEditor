#pragma once
#include <imgui/imgui.h>
#include "logging/OutputRedirect.h"

namespace Lengine {

    class ConsolePanel {
    public:
        ConsolePanel(LogBuffer& buffer)
            : m_Buffer(buffer) {
        }

        void OnImGuiRender();

    private:
        LogBuffer& m_Buffer;
    };

}
