#pragma once

#include <optional>
#include <vector>

#include <Windows.h>

namespace windows
{
    namespace synchronization
    {
        template <typename T>
        void wait_for_all(const std::vector<T>& objects, unsigned long timeout = INFINITE)
        {
            // TODO error handling
            const auto events = ::WaitForMultipleObjects(
                objects.size(),
                objects.data(),
                true,
                timeout);
        }

        template <typename T>
        void wait_for_any(const std::vector<T>& objects, unsigned long timeout = INFINITE)
        {
            // TODO error handling
            const auto events = ::WaitForMultipleObjects(
                objects.size(),
                objects.data(),
                false,
                timeout);
        }
    }
}