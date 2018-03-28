#pragma once

#include <optional>
#include <vector>

#include "error.hpp"

namespace windows
{
    namespace synchronization
    {
        struct timeout_wexception : public windows::wexception {};

        inline void wait(HANDLE object, unsigned long timeout = INFINITE)
        {
            const auto result = ::WaitForSingleObject(object, timeout);

            if (result == WAIT_ABANDONED)
            {
                // TODO we lose information here
                // caller needs to know that the mutex was abandoned
                return;
            }
            else if (result == WAIT_TIMEOUT)
            {
                throw timeout_wexception{};
            }
            else if (result == WAIT_FAILED)
            {
                throw windows::win32_wexception{ ::GetLastError() };
            }
        }

        inline void wait_for_all(const std::vector<HANDLE>& objects, unsigned long timeout = INFINITE)
        {
            const auto events = ::WaitForMultipleObjects(
                objects.size(),
                objects.data(),
                true,
                timeout);

            if (events < WAIT_ABANDONED_0 + objects.size())
            {
                // All objects signaled (possibly with an abandoned mutex)
                return;
            }
            else if (events == WAIT_TIMEOUT)
            {
                throw timeout_wexception{};
            }
            else
            {
                throw windows::win32_wexception{ ::GetLastError() };
            }
        }

        //! Wait on a `vector` of objects and return the first object that signals
        //! (abandoned mutexes also count).
        inline HANDLE wait_for_any(const std::vector<HANDLE>& objects, unsigned long timeout = INFINITE)
        {
            const auto events = ::WaitForMultipleObjects(
                objects.size(),
                objects.data(),
                false,
                timeout);

            if (events < WAIT_OBJECT_0 + objects.size())
            {
                return objects[events - WAIT_OBJECT_0];
            }
            else if (events < WAIT_ABANDONED_0 + objects.size())
            {
                // TODO we lose information here
                // caller needs to know that the mutex was abandoned
                return objects[events - WAIT_ABANDONED_0];
            }
            else if (events == WAIT_TIMEOUT)
            {
                throw timeout_wexception{};
            }
            else
            {
                throw windows::win32_wexception{ ::GetLastError() };
            }
        }
    }
}