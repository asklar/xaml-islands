#include "pch.h"
#include "MainPage.xaml.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif


#include <windows.h>
#include <stdio.h>
#include <strsafe.h>
#include <winevt.h>
#include <utility>
#include <algorithm>
#include <iostream>
#include <chrono>
#undef max

#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include "FilePicker.h"
namespace winrt {
    using namespace Microsoft::UI::Xaml;
    using namespace Windows::UI::Xaml::Controls;
}
namespace xaml = winrt::Windows::UI::Xaml;

using namespace std::chrono;
// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.


namespace winrt::MarkupSample::implementation
{
    MainPage::MainPage() : INIT_PROPERTY(MyInt, 42)
    {
        InitializeComponent();
        // Properties can be assigned to and read from with the operator= too!
        ImplString = winrt::hstring{ L"This string comes from the implementation" };
        winrt::hstring v = ImplString;
        BuildUIProgrammatically();
    }

    void MainPage::BuildUIProgrammatically() {
        std::wstring paraStr{ L"This string was created at runtime in code. Here is <A>a link</A> that opens a modal window." };
        size_t i = 0;
        while (i < paraStr.length()) {
            auto anchorStart = paraStr.find(L"<A>", 0);
            if (anchorStart > i) {
                xaml::Documents::Run run;
                run.Text(paraStr.substr(i, anchorStart - i));
                rtbParagraph().Inlines().Append(run);
                auto textStart = anchorStart + std::size(L"<A>") - 1;
                auto anchorEnd = paraStr.find(L"</A>", textStart);
                if (anchorEnd > anchorStart) {
                    xaml::Documents::Run linkText;
                    linkText.Text(paraStr.substr(textStart, anchorEnd - textStart));
                    assert(m_hl == nullptr);
                    m_hl = winrt::Windows::UI::Xaml::Documents::Hyperlink();
                    m_hl.Inlines().Append(linkText);
                    rtbParagraph().Inlines().Append(m_hl);
                    i = anchorEnd + std::size(L"</A>") - 1;
                }
            }
            else {
                xaml::Documents::Run run;
                run.Text(paraStr.substr(i));
                rtbParagraph().Inlines().Append(run);
                break;
            }
        }

    }
}



void winrt::MarkupSample::implementation::MainPage::PickFileButton_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::TappedRoutedEventArgs const& e)
{
    MarkupSample::FilePicker fp;
    popup().Child(fp);
    content().Opacity(0.01);
    popup().IsOpen(true);
    popup().Closed([this](auto&...) {
        content().Opacity(1);
        });
}
