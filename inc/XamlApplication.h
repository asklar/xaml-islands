#pragma once

#include "CppXaml.XamlApplication.g.h"
#include <winrt/Windows.UI.Xaml.Hosting.h>

namespace winrt::CppXaml::implementation
{
    struct XamlApplication : XamlApplicationT<XamlApplication>
    {
        XamlApplication() = default;

        XamlApplication(winrt::Windows::Foundation::Collections::IVector<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> const& providers)
        {
            for (auto&& provider : providers)
            {
                m_providers.Append(provider);
            }
        }

        ~XamlApplication()
        {
            Close();
        }

        void InitializeComponent()  // C++ WinRT 2 phase construction
        {
            m_windowsXamlManager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
        }

        winrt::Windows::Foundation::Collections::IVector<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> MetadataProviders()
        {
            return m_providers;
        }

        winrt::Windows::Foundation::IClosable WindowsXamlManager()
        {
            return m_windowsXamlManager;
        }

        // ICloseable
        void Close()
        {
            if (b_closed) return;

            b_closed = true;

            m_windowsXamlManager.Close();
            m_providers.Clear();
            m_windowsXamlManager = nullptr;

            // Toolkit has a message loop here.
        }

        // IXamlMetadataProvider
        winrt::Windows::UI::Xaml::Markup::IXamlType GetXamlType(winrt::Windows::UI::Xaml::Interop::TypeName const& type)
        {
            for (const auto& provider : m_providers)
            {
                if (const auto xamlType = provider.GetXamlType(type))
                {
                    return xamlType;
                }
            }
            return nullptr;
        }

        winrt::Windows::UI::Xaml::Markup::IXamlType GetXamlType(hstring const& fullName)
        {
            for (const auto& provider : m_providers)
            {
                if (const auto xamlType = provider.GetXamlType(fullName))
                {
                    return xamlType;
                }
            }
            return nullptr;
        }

        com_array<winrt::Windows::UI::Xaml::Markup::XmlnsDefinition> GetXmlnsDefinitions()
        {
            std::list<winrt::Windows::UI::Xaml::Markup::XmlnsDefinition> definitions;
            for (const auto& provider : m_providers)
            {
                auto defs = provider.GetXmlnsDefinitions();
                for (const auto& def : defs)
                {
                    definitions.insert(definitions.begin(), def);
                }
            }

            return winrt::com_array<winrt::Windows::UI::Xaml::Markup::XmlnsDefinition>(definitions.begin(), definitions.end());
        }

    private:
        winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager m_windowsXamlManager{ nullptr };
        winrt::Windows::Foundation::Collections::IVector<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> m_providers
            = winrt::single_threaded_vector<Windows::UI::Xaml::Markup::IXamlMetadataProvider>();
        bool b_closed{};
    };
}

namespace winrt::CppXaml::factory_implementation
{
    struct XamlApplication : XamlApplicationT<XamlApplication, implementation::XamlApplication>
    {
    };
}
