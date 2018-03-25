#pragma once

#include <string>
#include <utility>
#include <Windows.h>

namespace win64
{
    namespace gui
    {
        enum class window_style
        {
            overlapped, //!< `WS_OVERLAPPEDWINDOW`
            popup, //!< `WS_POPUPWINDOW`
            child //!< `WS_CHILDWINDOW`
        };

        namespace detail
        {
            inline uint32_t map_style(window_style style)
            {
                switch (style)
                {
                case window_style::overlapped: return WS_OVERLAPPEDWINDOW;
                case window_style::popup: return WS_POPUPWINDOW;
                case window_style::child: return WS_CHILDWINDOW;
                default: throw std::runtime_error{ "Unexpected" };
                }
            }

            template <typename WinProcLogic>
            WinProcLogic& extract_from_lparam(HWND window, LPARAM lparam)
            {
                const auto create = reinterpret_cast<CREATESTRUCT*>(lparam);
                WinProcLogic* result = reinterpret_cast<WinProcLogic*>(create->lpCreateParams);

                ::SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(result));
                return *result;
            }

            template <typename WinProcLogic>
            WinProcLogic& retrieve(HWND window)
            {
                const auto result = reinterpret_cast<WinProcLogic*>(::GetWindowLongPtr(window, GWLP_USERDATA));

                if (result == nullptr)
                {
                    // TODO throw GetLastError
                    throw std::runtime_error{ "Error" };
                }
                else
                {
                    return *result;
                }
            }

            //! Extract the `WinProcLogic` object and call its `handle_message` function.
            template <typename WinProcLogic>
            LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM arg1, LPARAM arg2)
            {
                try
                {
                    auto& logic = (message == WM_CREATE) ?
                        extract_from_lparam<WinProcLogic>(window, arg2) :
                        retrieve<WinProcLogic>(window);

                    return logic(window, message, arg1, arg2);
                }
                catch (...)
                {
                    return ::DefWindowProc(
                        window,
                        message,
                        arg1,
                        arg2);
                }
            }
        }

        //! A class template for handling windows.
        //! The `WinProcLogic` class must implement an `operator()`
        //! of type `(HWND, message, WPARAM, LPARAM) -> LRESULT`.
        //! Use `WinProcLogic` to store state for the window.
        template <typename WinProcLogic>
        class window
        {
        public:
            //! Register the window's `WNDCLASS` using `RegisterClass`
            window(
                std::wstring window_class_name,
                std::wstring window_name,
                window_style style,
                WinProcLogic logic,
                HINSTANCE module,
                HWND parent,
                HMENU menu,
                std::pair<int, int> position = { CW_USEDEFAULT, CW_USEDEFAULT },
                std::pair<int, int> dimensions = { CW_USEDEFAULT, CW_USEDEFAULT }) :
                module{ module },
                class_name{ std::move(window_class_name) },
                logic{ std::move(logic) }
            {
                WNDCLASSEX window_class{ sizeof(WNDCLASSEX) };

                window_class.lpfnWndProc = detail::window_procedure<WinProcLogic>;
                window_class.hInstance = module;
                window_class.lpszClassName = class_name.c_str();

                ::RegisterClassEx(&window_class);
                // TODO check result and call `::GetLastError()`

                window_handle = ::CreateWindowEx(
                    0,
                    class_name.c_str(),
                    window_name.c_str(),
                    detail::map_style(style),
                    position.first,
                    position.second,
                    dimensions.first,
                    dimensions.second,
                    parent,
                    menu,
                    module,
                    &(this->logic));

                if (window_handle == nullptr)
                {
                    throw std::runtime_error{ "CreateWindow returned an error" };
                }
            }

        public:
            void show(int show_window)
            {
                ::ShowWindow(window_handle, show_window);
            }

        private:
            HINSTANCE module;
            std::wstring class_name;
            HWND window_handle;
            WinProcLogic logic;
        };
    }
}