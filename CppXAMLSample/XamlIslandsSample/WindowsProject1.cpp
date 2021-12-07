// WindowsProject1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WindowsProject1.h"
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>

#include <winrt/MarkupSample.h>
#include <cppxaml/XamlWindow.h>

#include <dwmapi.h>

using namespace winrt;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Hosting;
using namespace cppxaml;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    auto winuiIXMP = winrt::Microsoft::UI::Xaml::XamlTypeInfo::XamlControlsXamlMetaDataProvider();
    auto markupIXMP = winrt::MarkupSample::XamlMetaDataProvider();

    auto xapp = winrt::Microsoft::Toolkit::Win32::UI::XamlHost::XamlApplication({ winuiIXMP, markupIXMP });

    AppController controller(hInstance, xapp);

    controller.OnUICreated = [](UIElement page, XamlWindow* xw) {
        if (auto mainPage = page.try_as<MarkupSample::MainPage>()) {
            assert(xw->Id() == L"MarkupSample");
            mainPage.InterfaceStr(L"This string comes from the win32 app");
            mainPage.EventHappened([xw](Windows::Foundation::IInspectable sender, winrt::hstring str) {
                if (str == L"LinkClicked") {
                    auto& modalWindow = XamlWindow::Get(L"Modal");
                    auto& mainWindow = XamlWindow::Get(L"MarkupSample");
                    modalWindow.Create(L"Modal", 0 /*WS_POPUPWINDOW*/, mainWindow.hwnd(), 600 * GetDpiForSystem() / 96, 600 * GetDpiForSystem() / 96);
                    EnableWindow(mainWindow.hwnd(), FALSE);
                }
                else if (str == L"OkClicked") {
                    DestroyWindow(xw->hwnd());
                }
                });
        }
        else if (auto modalPage = page.try_as<MarkupSample::ModalPage>()) {
            assert(xw->Id() == L"Modal");
            modalPage.OkClicked([xw](auto&...) {
                DestroyWindow(xw->hwnd());
                SetFocus(XamlWindow::Get(L"MarkupSample").hwnd());
                });
        }
    };

    controller.WndProc =
        [](HWND hWnd, INT message, WPARAM wParam, LPARAM lParam, XamlWindow* xw) -> LRESULT {
        switch (message) {
        case WM_CREATE: {
            auto hInstance = xw->Controller()->HInstance();
            auto smallIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SMALL));
            assert(smallIcon != nullptr);
            SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)smallIcon);
            auto bigIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
            assert(bigIcon != nullptr);
            SendMessageW(hWnd, WM_SETICON, ICON_BIG, (LPARAM)bigIcon);
            return 0;
            break;
        }
        case WM_KEYUP: {
            if (xw->Id() == L"Modal" && wParam == VK_ESCAPE) {
                DestroyWindow(xw->hwnd());
                SetFocus(XamlWindow::Get(L"MarkupSample").hwnd());
                return 0;
            }
            break;
        }
        case WM_DESTROY:
            if (xw->Id() == L"MarkupSample") {
                PostQuitMessage(0);
            }
            else if (xw->Id() == L"Modal") {
                EnableWindow(XamlWindow::Get(L"MarkupSample").hwnd(), TRUE);
            }
            break;
        }
        return DefWindowProcW(hWnd, message, wParam, lParam);
    };

    xapp.Resources().MergedDictionaries().Append(winrt::Microsoft::UI::Xaml::Controls::XamlControlsResources());
    auto& mainWindow = XamlWindow::Make<MarkupSample::MainPage>(L"MarkupSample", &controller);
    auto& modalWindow = XamlWindow::Make<MarkupSample::ModalPage>(L"Modal", &controller);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));
    mainWindow.SetAcceleratorTable(hAccelTable);

    wchar_t szTitle[100];                  // The title bar text
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, static_cast<int>(std::size(szTitle)));
    HWND hWnd = mainWindow.Create(szTitle, WS_OVERLAPPEDWINDOW, nullptr, 660, 880, nCmdShow);
    if (!hWnd)
    {
        return FALSE;
    }

    return mainWindow.RunLoop();
}
