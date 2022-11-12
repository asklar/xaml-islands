// WindowsProject1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WindowsProject1.h"
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>


#include <winrt/base.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Markup.h>

#include <XamlApplication.h>
#include <winrt/AppMarkup.h>

using namespace winrt;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Hosting;

using namespace CppXaml;

XamlApplication xapp{ nullptr };

// This DesktopWindowXamlSource is the object that enables a non-UWP desktop application 
// to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
DesktopWindowXamlSource desktopXamlSource{ nullptr };

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    winrt::init_apartment(apartment_type::single_threaded);

    cppxaml::InitializeWinUI();
    auto winuiIXMP = winrt::Microsoft::UI::Xaml::XamlTypeInfo::XamlControlsXamlMetaDataProvider();
    auto markupIXMP = winrt::AppMarkup::XamlMetaDataProvider();
    xapp = winrt::make_application(winuiIXMP, markupIXMP);
    
    WindowsXamlManager winxamlmanager = WindowsXamlManager::InitializeForCurrentThread();
    xapp.Resources().MergedDictionaries().Append(winrt::Microsoft::UI::Xaml::Controls::XamlControlsResources());

    desktopXamlSource = DesktopWindowXamlSource();

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

    MSG msg;


    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (auto xamlSourceNative2 = desktopXamlSource.as<IDesktopWindowXamlSourceNative2>()) {
          BOOL xamlSourceProcessedMessage = FALSE;
          winrt::check_hresult(xamlSourceNative2->PreTranslateMessage(&msg, &xamlSourceProcessedMessage));
          if (xamlSourceProcessedMessage) {
            continue;
          }
        }

        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  // Get handle to the core window.
  auto interop = desktopXamlSource.as<IDesktopWindowXamlSourceNative>();
  
  switch (message)
    {
    case WM_CREATE: {

      
      // Parent the DesktopWindowXamlSource object to the current window.
      check_hresult(interop->AttachToWindow(hWnd));  // This fails due to access violation!

      auto createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);

      // Get the new child window's hwnd
      HWND hWndXamlIsland = nullptr;
      check_hresult(interop->get_WindowHandle(&hWndXamlIsland));
      SetWindowPos(hWndXamlIsland, nullptr, 0, 0, createStruct->cx, createStruct->cy, SWP_SHOWWINDOW);

#ifdef CREATE_UI_IN_CODE
      // Option 1: create UI in code:
      Controls::TextBlock tb;
      tb.Text(L"Hello world!");
      desktopXamlSource.Content(tb);
#elif defined(CREATE_UI_FROM_STRING)
      // Option 2: create UI from a string
      auto tb = Markup::XamlReader::Load(LR"(
        <TextBlock Text="Hello world!"/>
)").as<TextBlock>();
      desktopXamlSource.Content(tb);
#else
      // Option 3: use a Windows Runtime component to define the UI in markup, and load it here
      Frame f;
      desktopXamlSource.Content(f);
      f.Navigate(winrt::xaml_typename<AppMarkup::BlankPage>());
#endif
      break;
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
        }
        break;
    case WM_SIZE:
    {
      HWND hWndXamlIsland = nullptr;
      check_hresult(interop->get_WindowHandle(&hWndXamlIsland));

      SetWindowPos(hWndXamlIsland, nullptr, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_SHOWWINDOW);

      break;
    }
    case WM_DESTROY:
        xapp.Close();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
