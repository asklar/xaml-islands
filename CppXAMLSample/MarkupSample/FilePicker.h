#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "FilePicker.g.h"
#include "StorageItem2.g.h"

#include <cppxaml/XamlProperty.h>
#include <winrt/Windows.Globalization.DateTimeFormatting.h>
namespace winrt {
    inline ::winrt::hstring to_hstring(::winrt::Windows::Foundation::DateTime dt) {
        auto f = winrt::Windows::Globalization::DateTimeFormatting::DateTimeFormatter(L"shortdate shorttime");
        return f.Format(dt);
    }
}

namespace winrt::MarkupSample::implementation
{
    using obsListOfItems_t = winrt::Windows::Foundation::Collections::IObservableVector<MarkupSample::StorageItem2>;
    using listOfItems_t = winrt::Windows::Foundation::Collections::IVector<MarkupSample::StorageItem2>;

    struct StorageItem2 : StorageItem2T<StorageItem2> {
        cppxaml::XamlProperty<MarkupSample::StorageItem2> Parent{nullptr};
        cppxaml::XamlProperty<winrt::hstring> Name;
        cppxaml::XamlProperty<winrt::Windows::Foundation::DateTime> DateModified;
        cppxaml::XamlProperty<winrt::hstring> Type;
        cppxaml::XamlProperty<obsListOfItems_t> Children = winrt::single_threaded_observable_vector<MarkupSample::StorageItem2>();
        cppxaml::XamlProperty<winrt::Windows::UI::Xaml::Controls::Symbol> Icon;
        MarkupSample::StorageItem2 AddChild(MarkupSample::StorageItem2 child) { 
            Children().Append(child);
            return *this; 
        }

        hstring Path() {
            if (Parent() != nullptr && Parent().Name() != L"") {
                return Parent().Path() + L"/" + Name();
            }
            else {
                return Name();
            }
        }

        StorageItem2() = default;
    };

    struct FilePicker : FilePickerT<FilePicker>, cppxaml::SimpleNotifyPropertyChanged<FilePicker>
    {
        FilePicker();

        cppxaml::XamlPropertyWithNPC<obsListOfItems_t> TreeViewItems;
        cppxaml::XamlPropertyWithNPC<obsListOfItems_t> Items;
        cppxaml::XamlProperty<listOfItems_t> NavigationStack = winrt::single_threaded_vector<MarkupSample::StorageItem2>();

        bool CanGoForward() {
            return currentIndex < NavigationStack().Size() - 1 && NavigationStack().Size() != 0;
        }

        bool CanGoBack() {
            auto r = currentIndex > 0 && NavigationStack().Size() != 0;
            OutputDebugString(L"CanGoBack: ");
            OutputDebugString(std::to_wstring(r).c_str());
            OutputDebugString(L"\n");
            return r;
        }

        bool CanGoUp() {
            return false;
        }
    private:
        void NavigateToFolder(MarkupSample::StorageItem2 item);
        void FetchFolder();
        uint32_t currentIndex = 0;
        MarkupSample::StorageItem2 CurrentFolder() {
            return NavigationStack().GetAt(currentIndex);
        }
    public:
        void itemsview_SelectionChanged(IInspectable, Windows::UI::Xaml::Controls::SelectionChangedEventArgs e) {
            if (e.AddedItems().Size() == 1) {
                auto item = e.AddedItems().GetAt(0).as<MarkupSample::StorageItem2>();
                FileName().Text(item.Path());
            }
        }

        void TreeViewItem_Tapped(IInspectable sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs) {
            auto item = sender.as<FrameworkElement>().DataContext().as<MarkupSample::StorageItem2>();
            NavigateToFolder(item);
        }
        void back_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::TappedRoutedEventArgs const& e);
        void forward_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::TappedRoutedEventArgs const& e);
    };
}

namespace winrt::MarkupSample::factory_implementation
{
    struct FilePicker : FilePickerT<FilePicker, implementation::FilePicker>
    {
    };

    struct StorageItem2 : StorageItem2T<StorageItem2, implementation::StorageItem2> {};
}
 