// WindowsProject1.cpp : Defines the entry point for the application.
//
#include "pch.h"
#include "framework.h"
#include "WindowsProject1.h"


using namespace winrt;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Hosting;

using namespace Microsoft::Toolkit::Win32::UI::XamlHost;

using namespace Microsoft::UI::Xaml::Controls;

// This DesktopWindowXamlSource is the object that enables a non-UWP desktop application 
// to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
DesktopWindowXamlSource desktopXamlSource{ nullptr };
XamlApplication xapp{ nullptr };
ContentPresenter presenter{ nullptr };

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
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    winrt::init_apartment(apartment_type::single_threaded);

    auto winuiIXMP = winrt::Microsoft::UI::Xaml::XamlTypeInfo::XamlControlsXamlMetaDataProvider();

    xapp = XamlApplication({ winuiIXMP });
    WindowsXamlManager winxamlmanager = WindowsXamlManager::InitializeForCurrentThread();
    xapp.Resources().MergedDictionaries().Append(winrt::Microsoft::UI::Xaml::Controls::XamlControlsResources());
    
    desktopXamlSource = DesktopWindowXamlSource();
    SetThreadDescription(GetCurrentThread(), L"XAML thread");
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



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
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

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
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

   // The call to winrt::init_apartment initializes COM; by default, in a multithreaded apartment.

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
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

      StackPanel main;
      
      TextBlock tb;
      tb.Text(L"XAML islands Playground");
      tb.HorizontalAlignment(HorizontalAlignment::Center);
      tb.FontWeight(Windows::UI::Text::FontWeights::Black());
      tb.FontSize(32);
      main.Children().Append(tb);

      auto infobar = Microsoft::UI::Xaml::Controls::InfoBar();
      main.Children().Append(infobar);

      StackPanel sp;
      sp.Orientation(Orientation::Horizontal);

      Grid editPanel;
      TextBox edit;
      edit.Height(600);
      edit.MinWidth(600);
      edit.HorizontalAlignment(HorizontalAlignment::Stretch);
      edit.VerticalAlignment(VerticalAlignment::Top);
      edit.AcceptsReturn(true);
      edit.IsSpellCheckEnabled(false);
      Media::FontFamily consolas(L"Consolas");
      edit.FontFamily(consolas);
      
      edit.Text(LR"(
<StackPanel 
  xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" 
  xmlns:winui="using:Microsoft.UI.Xaml.Controls"
  Background="Black">
<!-- Your content goes here -->
</StackPanel>
)");
      editPanel.Children().Append(edit);

      Button run;
      run.Content(winrt::box_value(L"Run"));
      run.Tapped([=](auto&, auto&) {
        try {
          auto parsedContent = Markup::XamlReader::Load(edit.Text());
          presenter.Content(parsedContent);
        }
        catch (const winrt::hresult_error& e) {
          infobar.Title(L"Error");
          infobar.Message(e.message());
          infobar.Severity(InfoBarSeverity::Error);
          infobar.IsOpen(true);
        }
        });

      run.Margin(ThicknessHelper::FromUniformLength(4));
      run.HorizontalAlignment(HorizontalAlignment::Right);
      run.VerticalAlignment(VerticalAlignment::Top);
      editPanel.Margin(ThicknessHelper::FromUniformLength(12));
      editPanel.Children().Append(run);

      sp.Children().Append(editPanel);
      presenter = ContentPresenter();
      presenter.HorizontalAlignment(HorizontalAlignment::Stretch);
      presenter.Background(Media::SolidColorBrush{ Windows::UI::Colors::Aqua() });
      presenter.Margin(ThicknessHelper::FromUniformLength(4));

      presenter.MinWidth(600);
      presenter.MinHeight(800);
      sp.Children().Append(presenter);

      main.Children().Append(sp);

      desktopXamlSource.Content(main);

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
//            PAINTSTRUCT ps;
//            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
//            EndPaint(hWnd, &ps);
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
