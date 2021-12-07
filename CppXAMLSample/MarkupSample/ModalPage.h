#pragma once

#include "ModalPage.g.h"
#include "cppxaml/XAMLProperty.h"

namespace winrt::MarkupSample::implementation
{
    struct ModalPage : ModalPageT<ModalPage>
    {
        ModalPage();

        cppxaml::XamlEvent<winrt::hstring> OkClicked;

        void Page_KeyUp(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        void Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void WebView_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::MarkupSample::factory_implementation
{
    struct ModalPage : ModalPageT<ModalPage, implementation::ModalPage>
    {
    };
}
