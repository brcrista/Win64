#include <CppUnitTest.h>

#include "Windows\error.hpp"

using namespace std;
using namespace windows;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define TEST(func, err, str) \
do \
{ \
    const auto result = func(err); \
    LOG_TRACE(result); \
    Assert::AreEqual(wstring{ str }, result); \
} while (false)

#define TEST_HRESULT(err, str) TEST(hresult_to_wstring, err, str)
#define TEST_WIN32(err, str) TEST(system_error_message, err, str)

TEST_CLASS(hresult_to_wstring_test)
{
public:

    TEST_METHOD(s_ok)
    {
        TEST_HRESULT(S_OK, L"S_OK");
    }
};

TEST_CLASS(win32_error_to_string_test)
{
public:

    TEST_METHOD(error_success)
    {
        TEST_WIN32(ERROR_SUCCESS, L"The operation completed successfully.\r\n");
    }

    TEST_METHOD(hresult)
    {
        TEST_WIN32(static_cast<DWORD>(S_OK), L"The operation completed successfully.\r\n");
        TEST_WIN32(static_cast<DWORD>(E_FAIL), L"Unspecified error\r\n");
        TEST_WIN32(static_cast<DWORD>(E_POINTER), L"Invalid pointer\r\n");
        TEST_WIN32(static_cast<DWORD>(E_NOINTERFACE), L"No such interface supported\r\n");
        TEST_WIN32(static_cast<DWORD>(E_UNEXPECTED), L"Catastrophic failure\r\n");
        TEST_WIN32(static_cast<DWORD>(E_NOTIMPL), L"Not implemented\r\n");
    }
};