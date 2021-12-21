#pragma once
#include <datetimeapi.h>
#include <WinNls.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include "cppxaml/XAMLProperty.h"
#include "MainPage.g.h"


namespace winrt::MarkupSample::implementation
{


    struct MainPage : MainPageT<MainPage>, cppxaml::SimpleNotifyPropertyChanged<MainPage>
    {
        MainPage();
        cppxaml::XamlProperty<winrt::hstring> InterfaceStr;
        cppxaml::XamlProperty<winrt::hstring> ImplString;

        cppxaml::XamlPropertyWithNPC<int> MyInt;
        auto Link() const {
            return m_hl;
        }
    private:
        void BuildUIProgrammatically();
        std::wstring m_EvaluationStrTemplate;
        winrt::Windows::UI::Xaml::Documents::Hyperlink m_hl{nullptr};
    };
}

namespace winrt::MarkupSample::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
