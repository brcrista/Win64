#pragma once

#include <array>
#include <functional>
#include <optional>
#include <vector>

#include <wrl.h>

#include "Core\log.hpp" // TODO from Essentials of COM 2 solution

#include "error.hpp"
#include "registry.hpp"

namespace windows
{
    namespace com
    {
        namespace server
        {
            //! Template for creating an instance of a given COM interface using an instance of some COM class
            //! implementing that interface.
            //! `Class` must inherit `Interface`.
            //! If the call succeeds, `result` will be an instance of `Class` with a reference count of 1.
            template <typename Interface, typename Class>
            HRESULT __stdcall instantiate(Interface** result)
            {
                Microsoft::WRL::ComPtr<Interface> obj = Microsoft::WRL::Make<Class>();

                if (obj == nullptr)
                {
                    result = nullptr;
                    return E_OUTOFMEMORY;
                }
                else
                {
                    *result = obj.Detach();
                    return S_OK;
                }
            }

            //! Information for a registry entry that is created upon registration of this COM server
            struct registry_entry
            {
                windows::path path;
                bool delete_on_unregister;
                std::optional<std::wstring> name; //!< Default keys in the registry do not have names.
                std::optional<std::wstring> value; //!< COM doesn't need this, just for humans
            };

            //! Handles registration and unregistration for the DLL.
            class server_registrar
            {
            public:
                // TODO template for any 'range' type
                server_registrar(std::vector<registry_entry> entries, windows::registry::hive hive = windows::registry::hive::local_machine) :
                    entries{ entries.begin(), entries.end() },
                    hive{ hive }
                {
                }

                // TODO why is this ambiguous?
                /*server_registrar(std::vector<registry_entry>&& entries) :
                entries{ std::forward<std::vector<registry_entry>>(entries) }
                {
                }*/

                //! Transactionally remove all entries marked for deletion on unregistration.
                void unregister_entries() const
                {
                    windows::ktm::transact([this](const windows::ktm::transaction& t) { _unregister_entries(t); });
                }

                //! Transactionally create all entries specified in `entries`.
                void register_entries() const
                {
                    windows::ktm::transact([this](const windows::ktm::transaction& t) { _register_entries(t); });
                }

            private:
                void _unregister_entries(const windows::ktm::transaction& transaction) const
                {
                    for (const auto& entry : entries)
                    {
                        if (entry.delete_on_unregister) windows::registry::delete_subtree(hive, entry.path, transaction);
                        else continue;
                    }
                }

                void _register_entries(const ktm::transaction& transaction) const
                {
                    _unregister_entries(transaction);

                    for (const auto& entry : entries)
                    {
                        const auto key = windows::registry::create_key(hive, entry.path, KEY_WRITE, transaction);

                        if (entry.value)
                        {
                            windows::registry::set_value_string(key.get(), entry.name, *entry.value);
                        }
                    }
                }

            private:
                std::vector<registry_entry> entries;
                windows::registry::hive hive;
            };

            //! Execute a function and convert exceptions to `HRESULT`s.
            inline HRESULT entry_point(const std::function<void(void)>& function)
            {
                try
                {
                    function();
                    return S_OK;
                }
                catch (const win32_wexception& e)
                {
                    LOG_ERROR(e.message());
                    return e.hresult();
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR(windows::locale::multibyte_to_wide(e.what()));
                    return E_FAIL;
                }
                catch (...)
                {
                    LOG_ERROR(L"Unexpected error");
                    return E_UNEXPECTED;
                }
            }
        }

        namespace client
        {
            //! Wraps a call to `CoCreateInstance`
            template <typename Interface, typename Class>
            Microsoft::WRL::ComPtr<Interface> __stdcall create_instance(DWORD class_context = CLSCTX_INPROC_SERVER)
            {
                Microsoft::WRL::ComPtr<Interface> instance;
                throw_if_failed(::CoCreateInstance(__uuidof(Class), nullptr, class_context, __uuidof(Interface), &instance));
                return instance;
            }

            //! Wraps a call to `CoGetClassObject`
            template <typename Class, typename IClassObject>
            Microsoft::WRL::ComPtr<IClassObject> __stdcall get_class_object(DWORD class_context = CLSCTX_INPROC_SERVER)
            {
                Microsoft::WRL::ComPtr<IClassObject> class_object;
                throw_if_failed(::CoGetClassObject(__uuidof(Class), class_context, nullptr, __uuidof(IClassObject), &class_object));
                return class_object;
            }
        }
    }
}