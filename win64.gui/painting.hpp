#pragma once

#include <functional>
#include <Windows.h>

namespace win64
{
    namespace gui
    {
        //! RAII class for `BeginPaint` and `EndPaint`.
        class painter
        {
        public:
            using paint_t = std::function<void(HWND, const PAINTSTRUCT&, HDC)>;

        public:
            painter(HWND window) :
                window{ window },
                paint_info{},
                device_context{ ::BeginPaint(window, &paint_info) }
            {
            }

            ~painter()
            {
                ::EndPaint(window, &paint_info);
            }

        public:
            void paint(const paint_t& paint_callback) const
            {
                paint_callback(window, paint_info, device_context);
            }

        private:
            HWND window;
            PAINTSTRUCT paint_info;
            HDC device_context;
        };
    }
}