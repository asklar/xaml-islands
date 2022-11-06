#include <winrt/Windows.System.h>

#include "XamlApplication.h"
#include <winrt/Windows.UI.Core.h>
#include <CoreWindow.h>

namespace winrt::Microsoft::Toolkit::Win32::UI::XamlHost::implementation {
  XamlApplication::XamlApplication(winrt::Windows::Foundation::Collections::IVector<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> providers) {
    for (auto provider : providers) {
      m_providers.Append(provider);
    }

    Initialize();
  }

  XamlApplication::XamlApplication() {}

  void XamlApplication::Initialize() {
    const auto out = outer();
    if (out) {
      winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider provider(nullptr);
      winrt::check_hresult(out->QueryInterface(
        winrt::guid_of<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider>(),
        reinterpret_cast<void**>(winrt::put_abi(provider))));

      winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider selfProvider(nullptr);
      winrt::check_hresult(this->NonDelegatingQueryInterface(
        winrt::guid_of<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider>(),
        reinterpret_cast<void**>(winrt::put_abi(selfProvider))));

      if (get_abi(selfProvider) != get_abi(provider)) {
        m_providers.Append(provider);
      }
    }

    const auto dispatcherQueue = winrt::Windows::System::DispatcherQueue::GetForCurrentThread();
    if (!dispatcherQueue) {
      m_executionMode = ExecutionMode::Win32;
      m_windowsXamlManager = ::winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();

      if (auto coreWindow = winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread()) {
        auto coreWindowInterop = coreWindow.as<ICoreWindowInterop>();
        HWND coreWindowHwnd = nullptr;
        coreWindowInterop->get_WindowHandle(&coreWindowHwnd);
        ::ShowWindow(coreWindowHwnd, SW_HIDE);
      }
    } else {
      m_executionMode = ExecutionMode::UWP;
    }
  }

  //void XamlApplication::Run(winrt::Microsoft::Toolkit::Win32::UI::XamlHost::DesktopWindow window) {
  //  window.Show();
  //  Run();
  //}

  void XamlApplication::Run() {
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    return; //(int)msg.wParam;
  }

  winrt::Windows::Foundation::IClosable XamlApplication::WindowsXamlManager() const {
    return m_windowsXamlManager;
  }

  void XamlApplication::Close() {
    if (m_bIsClosed) {
      return;
    }

    m_bIsClosed = true;

    m_windowsXamlManager.Close();
    m_providers.Clear();
    m_windowsXamlManager = nullptr;

    Exit();
    {
      MSG msg = {};
      while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        ::DispatchMessageW(&msg);
      }
    }
  }

  XamlApplication::~XamlApplication() {
    Close();
  }

  ::winrt::Windows::UI::Xaml::Markup::IXamlType XamlApplication::GetXamlType(::winrt::Windows::UI::Xaml::Interop::TypeName const& type) {
    for (const auto& provider : m_providers) {
      const auto xamlType = provider.GetXamlType(type);
      if (xamlType != nullptr) {
        return xamlType;
      }
    }

    return nullptr;
  }

  ::winrt::Windows::UI::Xaml::Markup::IXamlType XamlApplication::GetXamlType(winrt::hstring const& fullName) {
    for (const auto& provider : m_providers) {
      const auto xamlType = provider.GetXamlType(fullName);
      if (xamlType != nullptr) {
        return xamlType;
      }
    }

    return nullptr;
  }

  winrt::com_array<::winrt::Windows::UI::Xaml::Markup::XmlnsDefinition> XamlApplication::GetXmlnsDefinitions() {
    std::list<::winrt::Windows::UI::Xaml::Markup::XmlnsDefinition> definitions;
    for (const auto& provider : m_providers) {
      auto defs = provider.GetXmlnsDefinitions();
      for (const auto& def : defs) {
        definitions.insert(definitions.begin(), def);
      }
    }

    return winrt::com_array<::winrt::Windows::UI::Xaml::Markup::XmlnsDefinition>(definitions.begin(), definitions.end());
  }

  winrt::Windows::Foundation::Collections::IVector<::winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> XamlApplication::MetadataProviders() {
    return m_providers;
  }
}

namespace winrt::Microsoft::Toolkit::Win32::UI::XamlHost::factory_implementation {
  XamlApplication::XamlApplication() {
    // Workaround a bug where twinapi.appcore.dll and threadpoolwinrt.dll gets loaded after it has been unloaded
    // because of a call to GetActivationFactory
    const wchar_t* preloadDlls[] = {
        L"twinapi.appcore.dll",
        L"threadpoolwinrt.dll",
    };
    for (size_t i = 0; i < m_preloadInstances.size(); i++) {
      const auto module = ::LoadLibraryExW(preloadDlls[i], nullptr, 0);
      m_preloadInstances[i] = module;
    }
  }

  XamlApplication::~XamlApplication() {
    for (auto module : m_preloadInstances) {
      ::FreeLibrary(module);
    }
  }
}

void* __stdcall winrt_get_activation_factory([[maybe_unused]] std::wstring_view const& name) {
  auto requal = [](std::wstring_view const& left, std::wstring_view const& right) noexcept {
    return std::equal(left.rbegin(), left.rend(), right.rbegin(), right.rend());
  };

  if (requal(name, L"Microsoft.Toolkit.Win32.UI.XamlHost.XamlApplication")) {
    return winrt::detach_abi(winrt::make<winrt::Microsoft::Toolkit::Win32::UI::XamlHost::factory_implementation::XamlApplication>());
  }

  return nullptr;
}
