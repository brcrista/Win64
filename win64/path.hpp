#pragma once

#include <string>
#include <utility>

namespace windows
{
    //! A strongly typed, immutable wrapper for a string representing an NTFS path.
    class path
    {
    public:
        path(std::wstring str) :
            str{ std::move(str) }
        {
        }

        path(const path&) = default;
        path(path&&) = default;

        path& operator=(const path&) = delete;
        path& operator=(path&&) = delete;

        const wchar_t* c_str() const
        {
            return str.c_str();
        }

    public:
        const std::wstring str;
    };

    inline path operator/(const path& path, const std::wstring& str)
    {
        return path.str + L"\\" + str;
    }
}
