#pragma once

#include <optional>

#include <winreg.h>

#include "handle.hpp"
#include "ktm.hpp"
#include "locale.hpp"
#include "path.hpp"

namespace windows
{
    namespace registry
    {
        enum class hive
        {
            local_machine, //!< `HKEY_LOCAL_MACHINE`
            current_user //!< `HKEY_CURRENT_USER`
        };

        inline HKEY hkey(hive hive)
        {
            switch (hive)
            {
            default:
            case hive::local_machine: return HKEY_LOCAL_MACHINE;
            case hive::current_user: return HKEY_CURRENT_USER;
            }
        }

        namespace detail
        {
            //! Traits class for `unique_handle`
            struct registry_key_traits
            {
                using pointer = HKEY;

                static pointer invalid() throw()
                {
                    return nullptr;
                }

                static void close(pointer value) throw()
                {
                    ::RegCloseKey(value);
                }
            };
        }

        using unique_key = windows::unique_handle<detail::registry_key_traits>;

        //! Wraps a call to `RegCreateKeyTransacted`.
        inline unique_key create_key(hive parent, const windows::path& path, REGSAM access_rights, const windows::ktm::transaction& transaction)
        {
            HKEY result;

            windows::throw_if_failed(::RegCreateKeyTransacted(
                hkey(parent),
                path.c_str(),
                0,
                nullptr,
                REG_OPTION_NON_VOLATILE,
                access_rights,
                nullptr,
                &result,
                nullptr,
                transaction.get(),
                nullptr));

            return unique_key{ result };
        }

        //! Wraps a call to `RegOpenKeyTransacted`.
        //! Returns `nullopt` if the key does not exist.
        //! Throws `hresult_exception` on any other error.
        inline std::optional<unique_key> open_key(hive parent, const windows::path& path, REGSAM access_rights, const windows::ktm::transaction& transaction)
        {
            HKEY result;

            const auto status = ::RegOpenKeyTransacted(
                hkey(parent),
                path.c_str(),
                0,
                access_rights,
                &result,
                transaction.get(),
                nullptr);

            if (status == ERROR_FILE_NOT_FOUND)
            {
                return std::nullopt;
            }
            else if (status != ERROR_SUCCESS)
            {
                throw win32_wexception{ status };
            }
            else
            {
                return unique_key{ result };
            }
        }

        //! Set the value `value_name` under key `key` to `value_data`, or set the default value if the value is not given.
        //! Wraps a call to `RegSetValueEx`.
        inline void set_value_string(HKEY key, const std::optional<std::wstring>& value_name, const std::wstring& value_data)
        {
            windows::throw_if_failed(::RegSetValueEx(
                key,
                !value_name ? nullptr : value_name->c_str(),
                0,
                REG_SZ,
                reinterpret_cast<const BYTE*>(value_data.c_str()),
                static_cast<DWORD>(value_data.size())));
        }

        //! Opens the given key and calls `RegDeleteTree`.
        inline void delete_subtree(hive parent, const windows::path& path, const windows::ktm::transaction& transaction)
        {
            const auto access_rights = DELETE | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE; // access rights for `RegDeleteTree`
            const auto key = registry::open_key(parent, path, access_rights, transaction);

            // We are deleting keys, so if the key does not exist it is not an error.
            if (!key)
            {
                return;
            }
            else
            {
                throw_if_failed(::RegDeleteTree(key->get(), nullptr));
            }
        }
    }
}