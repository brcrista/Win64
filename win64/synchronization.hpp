#pragma once

#include <vector>

namespace windows
{
    namespace synchronization
    {
        template <typename T>
        void wait_for_all_objects(const std::vector<T>& objs, unsigned long timeout)
        {
            // TODO error handling
            ::WaitForMultipleObjects(
                objs.size(),
                objs.data(),
                true,
                timeout);
        }

        template <typename T>
        void wait_for_any_object(const std::vector<T>& objs, unsigned long timeout)
        {
            // TODO error handling
            ::WaitForMultipleObjects(
                objs.size(),
                objs.data(),
                false,
                timeout);
        }
    }
}