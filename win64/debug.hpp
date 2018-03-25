#pragma once

#include <Windows.h>

namespace windows
{
    namespace debug
    {
        inline void wait_for_debugger_attach()
        {
            while (!::IsDebuggerPresent())
            {
                ::Sleep(500);
            }
            ::DebugBreak();
        }
    }
}