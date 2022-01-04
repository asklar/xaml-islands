#include "pch.h"
#include "FilePicker.h"
#if __has_include("FilePicker.g.cpp")
#include "FilePicker.g.cpp"
#endif

#if __has_include("StorageItem2.g.cpp")
#include "StorageItem2.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::MarkupSample::implementation
{
    FilePicker::FilePicker() :
        INIT_PROPERTY(TreeViewItems, winrt::single_threaded_observable_vector<MarkupSample::StorageItem2>()),
        INIT_PROPERTY(Items, winrt::single_threaded_observable_vector<MarkupSample::StorageItem2>())
    {
        InitializeComponent();

        MarkupSample::StorageItem2 root;
        root.Name(L"");
        root.Type(L"Directory");
        NavigateToFolder(root);
    }

    void FilePicker::NavigateToFolder(MarkupSample::StorageItem2 item) {
        if (NavigationStack().Size() != 0 && currentIndex < NavigationStack().Size() - 1)
        {
            auto nToRemove = NavigationStack().Size() - currentIndex - 1;
            for (auto i = 0u; i < nToRemove; i++) {
                NavigationStack().RemoveAt(currentIndex + 1);
            }
        }

        NavigationStack().Append(item);
        currentIndex = NavigationStack().Size() - 1;

        AddressBar().Text(item.Path());

        FetchFolder();
    }
    MarkupSample::StorageItem2 MakeDir(hstring name, MarkupSample::StorageItem2 parent = nullptr) {
        auto si = MarkupSample::StorageItem2();
        si.Name(name);
        si.Type(L"Directory");
        si.Parent(parent);
        return si;
    }


    void FilePicker::FetchFolder() {
        Items().Clear();
        Items().Append(MakeDir(L"folder 1", CurrentFolder())
            .AddChild(MakeDir(L"folder 1.1"))
            .AddChild(MakeDir(L"folder 1.2")));
        Items().Append(MakeDir(L"folder 2", CurrentFolder()));
        Items().Append(MakeDir(L"folder 3", CurrentFolder()));

        for (int i = 1; i < 100; i++)
        {
            auto item = MarkupSample::StorageItem2{};
            item.Name(L"test" + std::to_wstring(i) + L".txt");
            item.DateModified(Windows::Foundation::DateTime());
            item.Type(L"Text file");
            item.Parent(CurrentFolder());
            Items().Append(item);
        }

        TreeViewItems().Clear();
        for (const auto& f : Items()) {
            if (f.Type() == L"Directory") {
                TreeViewItems().Append(f);
            }
        }

        RaisePropertyChanged(L"TreeViewItems");
        RaisePropertyChanged(L"Items");

        RaisePropertyChanged(L"CanGoBack");
        RaisePropertyChanged(L"CanGoForward");
    }

}


void winrt::MarkupSample::implementation::FilePicker::back_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::TappedRoutedEventArgs const& e)
{
    assert(CanGoBack());

    currentIndex--;

    FetchFolder();

    AddressBar().Text(CurrentFolder().Path());
}


void winrt::MarkupSample::implementation::FilePicker::forward_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::TappedRoutedEventArgs const& e)
{
    assert(CanGoForward());
    
    currentIndex++;

    FetchFolder();

    AddressBar().Text(CurrentFolder().Path());
}
