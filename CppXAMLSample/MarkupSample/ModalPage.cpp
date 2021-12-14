#include "pch.h"
#include "ModalPage.h"
#if __has_include("ModalPage.g.cpp")
#include "ModalPage.g.cpp"
#endif

#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.System.h>

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::MarkupSample::implementation
{
    ModalPage::ModalPage()
    {
        InitializeComponent();
    }
}

void winrt::MarkupSample::implementation::ModalPage::Page_KeyUp(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
{
    if (e.Key() == Windows::System::VirtualKey::Escape) {
        OkClicked.invoke(*this, L"");
    }
}


void winrt::MarkupSample::implementation::ModalPage::Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
    OkClicked.invoke(*this, L"");
}


void winrt::MarkupSample::implementation::ModalPage::WebView_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
    //sender.as<Controls::WebView>().Navigate(Windows::Foundation::Uri{ L"https://bing.com" });
}
