#pragma once

#include <functional>
#include <Windows.h> // required for ktmw32.h
#include <ktmw32.h>
#pragma comment(lib, "ktmw32.lib")

#include "handle.hpp"
#include "error.hpp"

namespace windows
{
    namespace ktm
    {
        namespace detail
        {
            //! Create a transaction that is not promotable to a distributed transaction and will not time out.
            //! Throws if the creation fails.
            inline HANDLE create_default_transaction()
            {
                const auto handle = ::CreateTransaction(nullptr, nullptr, TRANSACTION_DO_NOT_PROMOTE, 0, 0, INFINITE, nullptr);
                if (handle == INVALID_HANDLE_VALUE)
                {
                    throw windows::win32_wexception{ ::GetLastError() };
                }
                else
                {
                    return handle;
                }
            }
        }

        //! Represents an atomic transaction handled by the Kernel Transaction Manager
        class transaction
        {
        public:
            //! Wraps a call to `CreateTransaction`.
            //! If the operation fails, the error code from `GetLastError` is converted to an `HRESULT` and thrown.
            transaction() :
                handle{ detail::create_default_transaction() }
            {
            }

            //! Wraps a call to `CommitTransaction`.
            //! If the operation fails, the error code from `GetLastError` is converted to an `HRESULT` and thrown.
            void commit() const
            {
                throw_if_failed(::CommitTransaction(get()));
            }

            const HANDLE get() const
            {
                return handle.get();
            }

            HANDLE get()
            {
                return handle.get();
            }

        private:
            windows::invalid_handle handle;
        };

        //! Execute a `transaction` -> `void` function as a transaction.
        //! If the function throws an exception, roll it back.
        //! Otherwise, commit it.
        inline void transact(const std::function<void(const transaction&)>& action)
        {
            const windows::ktm::transaction t{};
            action(t);
            t.commit();
        }
    }
}