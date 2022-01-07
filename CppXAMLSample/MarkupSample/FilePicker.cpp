#include "pch.h"
#include "FilePicker.h"
#if __has_include("FilePicker.g.cpp")
#include "FilePicker.g.cpp"
#endif

#if __has_include("StorageItem2.g.cpp")
#include "StorageItem2.g.cpp"
#endif

#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.Storage.Streams.h>

#include <shellapi.h>
#include <shobjidl_core.h>
#include <shlobj_core.h>
#include <chrono>

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
        root.Name(L"C:");
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

    auto GetIconBitmapBits(HICON icon) {
        std::array<uint8_t, 32 * 32 * 4> bytes;

        if (HDC hdc = GetDC(nullptr)) {
            if (HDC memdc = CreateCompatibleDC(hdc)) {

                if (auto bitmap = CreateCompatibleBitmap(hdc, 32, 32)) {

                    auto hOldBitmap = SelectObject(memdc, bitmap);

                    DrawIconEx(memdc, 0, 0, icon, 32, 32, 0, nullptr, DI_NORMAL);
                    SelectObject(memdc, hOldBitmap);

                    BITMAPINFO bmi{};
                    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
                    bmi.bmiHeader.biWidth = 32;
                    bmi.bmiHeader.biHeight = -32;
                    bmi.bmiHeader.biBitCount = 32;
                    
                    GetDIBits(memdc, bitmap, 0, 0, nullptr, &bmi, DIB_RGB_COLORS);

                    auto lines = GetDIBits(memdc, bitmap, 0, 32, &bytes, &bmi, DIB_RGB_COLORS);

                    DeleteObject(bitmap);
                }
                DeleteDC(memdc);
            }
            ReleaseDC(nullptr, hdc);
        }
        DestroyIcon(icon);
        return bytes;
    }

    auto WriteableBitmapFromBits(const std::array<uint8_t, 32*32*4>& bits) {
        Media::Imaging::WriteableBitmap wb(32, 32);
        auto data = wb.PixelBuffer().data();
        memcpy(data, &bits, bits.size());

        return wb;
    }

    void FilePicker::FetchFolder() {
        Items().Clear();

        TreeViewItems().Clear();
        auto path = CurrentFolder().Path() + LR"(\*)";
        WIN32_FIND_DATA fd{};
        auto file = FindFirstFile(path.c_str(), &fd);
        do {
            if (wcscmp(fd.cFileName, L".") == 0 ||
                wcscmp(fd.cFileName, L"..") == 0) continue;

            auto si = MarkupSample::StorageItem2{};
            si.Name(fd.cFileName);
            si.Parent(CurrentFolder());
            SHFILEINFO sfi{};
            SHGetFileInfoW(si.Path().c_str(), fd.dwFileAttributes, &sfi, sizeof(sfi), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
            
            si.DateModified(winrt::clock::from_FILETIME(fd.ftLastWriteTime));
            
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) continue;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) continue;
            si.Type(sfi.szTypeName);

            HICON icon{nullptr};

            winrt::com_ptr<IExtractIcon> extractIcon;
            winrt::check_hresult(SHCreateFileExtractIcon(si.Path().c_str(), fd.dwFileAttributes, IID_IExtractIconW, extractIcon.put_void()));

            int index{};
            uint32_t flags{};
            wchar_t location[MAX_PATH]{};
            extractIcon->GetIconLocation(GIL_FORSHELL, location, (uint32_t)std::size(location), &index, &flags);

            extractIcon->Extract(location, index, nullptr, &icon, 32);

            auto bitmap = GetIconBitmapBits(icon);
            auto wb = WriteableBitmapFromBits(bitmap);

            si.ImageSource(wb);
            Items().Append(si);

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                TreeViewItems().Append(si);
            }
        } while (FindNextFileW(file, &fd) != 0);
        FindClose(file);

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


void winrt::MarkupSample::implementation::FilePicker::up_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::TappedRoutedEventArgs const& e)
{
    assert(CurrentFolder().Parent() != nullptr);
    NavigateToFolder(CurrentFolder().Parent());
}
