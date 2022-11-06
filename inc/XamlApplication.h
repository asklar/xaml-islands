#pragma once

#include "XamlApplication.g.h"
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <Windows.h>
//#include <DesktopWindow.h>
namespace winrt::Microsoft::Toolkit::Win32::UI::XamlHost {
  struct XamlApplication;
}

namespace winrt::Microsoft::Toolkit::Win32::UI::XamlHost::implementation {
  enum ExecutionMode {
    UWP = 0,
    Win32 = 1,
  };
  using namespace winrt;

  class XamlApplication : public XamlApplicationT<XamlApplication, winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> {
  public:
    XamlApplication();
    XamlApplication(winrt::Windows::Foundation::Collections::IVector<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> providers);
    ~XamlApplication();

    void Initialize();

    void Close();

    //void Run(winrt::Microsoft::Toolkit::Win32::UI::XamlHost::DesktopWindow window);

    void Run();

    winrt::Windows::Foundation::IClosable WindowsXamlManager() const;

    winrt::Windows::UI::Xaml::Markup::IXamlType GetXamlType(winrt::Windows::UI::Xaml::Interop::TypeName const& type);
    winrt::Windows::UI::Xaml::Markup::IXamlType GetXamlType(winrt::hstring const& fullName);
    winrt::com_array<winrt::Windows::UI::Xaml::Markup::XmlnsDefinition> GetXmlnsDefinitions();

    winrt::Windows::Foundation::Collections::IVector<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> MetadataProviders();

    bool IsDisposed() const {
      return m_bIsClosed;
    }

  private:
    ExecutionMode m_executionMode = ExecutionMode::Win32;
    winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager m_windowsXamlManager = nullptr;
    winrt::Windows::Foundation::Collections::IVector<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> m_providers = winrt::single_threaded_vector<Windows::UI::Xaml::Markup::IXamlMetadataProvider>();
    bool m_bIsClosed = false;
  };
}

namespace winrt::Microsoft::Toolkit::Win32::UI::XamlHost::factory_implementation {
  class XamlApplication : public XamlApplicationT<XamlApplication, implementation::XamlApplication> {
  public:
    XamlApplication();
    ~XamlApplication();
  private:
    std::array<HMODULE, 2> m_preloadInstances;
  };
}



WINRT_EXPORT namespace winrt::Microsoft::Toolkit::Win32::UI::XamlHost {

// We can't use the cppwinrt generated code for activation because it relies on the activation going through
// LoadLibrary (inferring what library based on namespace). In the case of XamlApplication, it's a local type, so we short circuit it.

  inline XamlApplication::XamlApplication(param::vector<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> const& providers) {
    winrt::Windows::Foundation::IInspectable baseInterface, innerInterface;
    auto factory = winrt::make<winrt::Microsoft::Toolkit::Win32::UI::XamlHost::factory_implementation::XamlApplication>();
    auto self = winrt::get_self<winrt::Microsoft::Toolkit::Win32::UI::XamlHost::factory_implementation::XamlApplication>(factory);
    *this = self->CreateInstance(providers, baseInterface, innerInterface);

    // *this = impl::call_factory<XamlApplication, IXamlApplicationFactory>([&](IXamlApplicationFactory const& f) { return f.CreateInstance(providers, baseInterface, innerInterface); });
  }
  inline XamlApplication::XamlApplication() {
    winrt::Windows::Foundation::IInspectable baseInterface, innerInterface;
    auto factory = winrt::make<winrt::Microsoft::Toolkit::Win32::UI::XamlHost::factory_implementation::XamlApplication>();
    auto self = winrt::get_self<winrt::Microsoft::Toolkit::Win32::UI::XamlHost::factory_implementation::XamlApplication>(factory);
    *this = self->CreateInstance2(baseInterface, innerInterface);
    //    *this = impl::call_factory<XamlApplication, IXamlApplicationFactory>([&](IXamlApplicationFactory const& f) { return f.CreateInstance2(baseInterface, innerInterface); });
  }

}