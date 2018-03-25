#pragma once

#include <array>
#include <Windows.h>
#include <winerror.h>

namespace windows
{
    //! Get the name of the `HRESULT` (for example, `S_OK`) as a string
    //! or convert the numerical value to a string if the name is not specified.
    inline std::wstring hresult_to_wstring(HRESULT hr)
    {
        switch (hr)
        {
        case S_OK: return L"S_OK";
        case E_ACCESSDENIED: return L"E_ACCESSDENIED";
        case E_POINTER: return L"E_POINTER";
        case E_NOINTERFACE: return L"E_NOINTERFACE";
        case E_UNEXPECTED: return L"E_UNEXPECTED";
        case E_FAIL: return L"E_FAIL";
        case REGDB_E_CLASSNOTREG: return L"REGDB_E_CLASSNOTREG";
        default: return std::to_wstring(hr);
        }
    }

    //! Look up the error message for a given error code.
    //! Wraps a call to `FormatMessage` with the `FORMAT_MESSAGE_FROM_SYSTEM` flag set.
    inline std::wstring system_error_message(DWORD error)
    {
        constexpr size_t max_size = 2 << 10;
        std::array<wchar_t, max_size> buffer;

        const auto size = ::FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr,
            error,
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
            buffer.data(),
            static_cast<DWORD>(buffer.size()),
            nullptr);

        if (size == 0)
        {
            return L"No error message is available";
        }
        else
        {
            return{ buffer.data(), buffer.data() + size };
        }
    }

    //! Look up the error message for a given error code.
    //! Wraps a call to `FormatMessage` with the `FORMAT_MESSAGE_FROM_SYSTEM` flag set.
    inline std::wstring system_error_message(HRESULT hr)
    {
        return system_error_message(static_cast<DWORD>(HRESULT_CODE(hr)));
    }

    //! A base class for exceptions that contain wide-character error messages.
    //! By design, this does not derive from `std::exception`.
    //! This encourages `wexception`s to be handled in a separate `catch` block
    //! so that its wide-character `message` function is called for the error message.
    class wexception
    {
    public:
        wexception() :
            _message{}
        {
        }

        wexception(const std::wstring& message) :
            _message{ message }
        {
        }

        virtual ~wexception()
        {
        }

    public:
        virtual const wchar_t* message() const noexcept
        {
            return _message.c_str();
        }


    private:
        std::wstring _message;
    };

    //! Exception class that generalizes the various error codes used in Windows APIs
    //! and maps them to `HRESULT`.
    //! The constructors perform a dynamic string allocation and so can cause a second exception
    //! to be thrown while this one is being constructed if the system is out of memory.
    //! That condition is assumed to be sufficiently rare that it may be ignored.
    class win32_exception : public wexception
    {
    public:
        // `HRESULT` and `LSTATUS`
        win32_exception(long error_code) :
            wexception{ error_message(hr) },
            hr{ HRESULT_FROM_WIN32(error_code) } // if `error_code` is already an `HRESULT`, this will leave it unchanged
        {
        }

        // Separate `DWORD` constructor to avoid C++11's "narrowing conversion" error
        win32_exception(DWORD error_code) :
            wexception{ error_message(hr) },
            hr{ HRESULT_FROM_WIN32(error_code) }
        {
        }

    public:
        //! Get an `HRESULT` for diagnosing the error.
        HRESULT hresult() const
        {
            return hr;
        }

    private:
        HRESULT hr;

        template <typename Error>
        static std::wstring error_message(Error error)
        {
            const auto hr = HRESULT_FROM_WIN32(error);
            return hresult_to_string(hr) + L": " + system_error_message(hr);
        }
    };

    //! Test an `HRESULT`, `LSTATUS`, or Win32 error code for failure and throw a `win32_exception` if it failed.
    inline void throw_if_failed(long error_code)
    {
        if (HRESULT_FROM_WIN32(error_code) != S_OK)
        {
            throw win32_exception{ error_code };
        }
    }

    //! Test a `BOOL` for failure and throw `GetLastError` as a `win32_exception` if it failed.
    inline void throw_if_failed(BOOL success)
    {
        if (!success)
        {
            throw win32_exception{ ::GetLastError() };
        }
    }
}