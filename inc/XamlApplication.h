#pragma once

#include "CppXaml.XamlApplication.g.h"
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <appmodel.h>
#include <strsafe.h>

namespace winrt::CppXaml::implementation
{
    struct XamlApplication : XamlApplicationT<XamlApplication>
    {
        XamlApplication() = default;

        XamlApplication(std::initializer_list<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> providers)
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

namespace winrt {
  template<typename... TProviders>
  winrt::CppXaml::XamlApplication make_application(TProviders&&... providers)
  {
    return winrt::make<winrt::CppXaml::implementation::XamlApplication>(std::initializer_list<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider>{providers...});
  }
}

#pragma comment(lib, "onecoreuap_apiset.lib")

namespace cppxaml {
  namespace details {
    PWSTR g_winUIDepId{};
    PACKAGEDEPENDENCY_CONTEXT g_ctx{};
  }

  /// <summary>
  /// Dynamically initializes WinUI 2. This API works on Windows 11 and newer.
  /// Adds the WinUI 2 framework package to the process's package graph.
  /// This enables unpackaged applications to use the stable (non-prerelease) WinUI 2 NuGet package.
  /// </summary>
  /// <param name="minorVersion">Optional minor version. Defaults to 8 (for 2.8)</param>
  void InitializeWinUI(uint8_t minorVersion = 8) {

    if (details::g_winUIDepId == nullptr) {
      PACKAGE_VERSION minVersion{};
      minVersion.Major = minorVersion;

      constexpr const std::wstring_view winuiPackageFamilyNameTemplate{ L"Microsoft.UI.Xaml.2.%d_8wekyb3d8bbwe" };
      wchar_t pfn[PACKAGE_FAMILY_NAME_MAX_LENGTH]{};
      winrt::check_hresult(StringCchPrintf(pfn, std::size(pfn), winuiPackageFamilyNameTemplate.data(), minorVersion));

      winrt::check_hresult(TryCreatePackageDependency(nullptr, pfn, minVersion,
        PackageDependencyProcessorArchitectures_None, PackageDependencyLifetimeKind_Process,
        nullptr, CreatePackageDependencyOptions_None, &details::g_winUIDepId));
    }

    PWSTR packageFullName{};

    winrt::check_hresult(AddPackageDependency(details::g_winUIDepId, 0, AddPackageDependencyOptions_None, &details::g_ctx, &packageFullName));
  }

  /// <summary>
  /// Removes a reference to WinUI from the process's package graph.
  /// </summary>
  void UninitializeWinUI() {
    winrt::check_hresult(RemovePackageDependency(details::g_ctx));
    // should HeapFree g_winUIDepId - but do it in a refcounted way
  }
}