#include <CppUnitTest.h>

#include "Windows\locale.hpp"

using namespace std;
using namespace windows::locale;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(locale_test)
{
public:

    TEST_METHOD(encoding_conversion_test)
    {
        const string multibyte{ "Hello!" };
        const wstring wide{ L"Hello!" };

        Assert::AreEqual(wide, multibyte_to_wide(multibyte));
        Assert::AreEqual(multibyte, wide_to_multibyte(wide));
    }
};