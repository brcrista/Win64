#pragma once

#include <codecvt>
#include <string>
#include <Windows.h>

namespace windows
{
    namespace locale
    {
        inline std::wstring multibyte_to_wide(const std::string& multibyte)
        {
            return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes(multibyte);
        }

        inline std::string wide_to_multibyte(const std::wstring& wide)
        {
            return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.to_bytes(wide);
        }
    }
}