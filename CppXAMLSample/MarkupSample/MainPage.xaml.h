﻿#pragma once
#include <datetimeapi.h>
#include <WinNls.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include "cppxaml/XAMLProperty.h"
#include "MainPage.g.h"


using namespace cppxaml;
namespace winrt::MarkupSample::implementation
{


    struct MainPage : MainPageT<MainPage>, SimpleNotifyPropertyChanged<MainPage>
    {
        MainPage();
        XamlProperty<winrt::hstring> InterfaceStr;
        XamlProperty<winrt::hstring> ImplString;

        XamlEvent<winrt::hstring> EventHappened;

        void Button_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::TappedRoutedEventArgs const& e);
        void Hyperlink_Click(winrt::Windows::UI::Xaml::Documents::Hyperlink const& sender, winrt::Windows::UI::Xaml::Documents::HyperlinkClickEventArgs const& args);


    private:
        void BuildUIProgrammatically();
        std::wstring m_EvaluationStrTemplate;
    };
}

namespace winrt::MarkupSample::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
