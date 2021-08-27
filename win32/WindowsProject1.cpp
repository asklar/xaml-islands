// WindowsProject1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WindowsProject1.h"
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>


#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Microsoft.Toolkit.Win32.UI.XamlHost.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.DragDrop.h>

#include <winrt/Windows.UI.Xaml.Markup.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::UI;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::UI::Xaml::Media;
using namespace Imaging;
using namespace Microsoft::Toolkit::Win32::UI::XamlHost;


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
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    winrt::init_apartment(apartment_type::single_threaded);

    WindowsXamlManager winxamlmanager = WindowsXamlManager::InitializeForCurrentThread();

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
    wcex.lpszMenuName = nullptr;
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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_POPUP,
      0, 0,  1024, 768, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // The call to winrt::init_apartment initializes COM; by default, in a multithreaded apartment.

   return TRUE;
}


struct Item {
  std::wstring displayName;
  std::wstring iconPath;
};

auto MakeBitmapIcon(const std::wstring& filename) {
  BitmapImage bi;

  wchar_t cwd[MAX_PATH] = {};
  GetCurrentDirectoryW(std::size(cwd), cwd);
  auto absolutePath = std::wstring(cwd) + L"\\" + filename;

  bi.UriSource(Uri{ absolutePath });
  Image i;
  i.Source(bi);
  auto size = 64;
  i.Width(size);
  i.Height(size);
  
  return i;

/*
  BitmapImage bi;
  auto size = 128;
  bi.DecodePixelWidth(size);
  bi.DecodePixelHeight(size);
  bi.UriSource(Uri{ uri });
  Image img;
  img.Source(bi);
  img.Width(size);
  img.Height(size);
  return img;
  */
  
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

      BitmapImage bi;
      bi.UriSource(Uri{ L"https://w.wallhaven.cc/full/0q/wallhaven-0qmo8d.jpg" });
      Image i;
      i.Source(bi);
      
      Canvas c;
      Canvas::SetZIndex(i, -100);
      c.Children().Append(i);

      TextBlock tb;
      tb.Text(L"Hello world!");
      tb.Foreground(SolidColorBrush(Colors::White()));
      
      Canvas::SetLeft(tb, 100);
      Canvas::SetTop(tb, 200);

      c.Children().Append(tb);


      GridView gv;
      auto ipt= Markup::XamlReader::Load(LR"(
        <ItemsPanelTemplate xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
          <ItemsWrapGrid VerticalAlignment="Top" ItemWidth="64" Orientation="Horizontal" MaximumRowsOrColumns="5"/>
        </ItemsPanelTemplate>
      )").as<ItemsPanelTemplate>();
      gv.ItemsPanel(ipt);

      std::vector<Item> items = {
        {L"item 1", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 2", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 3", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 4", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 5", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 6", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 7", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 1", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 2", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 3", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 4", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 5", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 6", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 7", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 1", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 2", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 3", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 4", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 5", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 6", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
        {L"item 7", L"Assets\\Icon1.png" /*L"https://www.vectorico.com/download/office/folder-icon.jpg"*/},
      };
      for (const auto& e : items) {
        StackPanel sp;
        sp.Orientation(Orientation::Vertical);
        sp.Margin(ThicknessHelper::FromUniformLength(6));
        auto img = MakeBitmapIcon(e.iconPath);
        
        Border border;
        border.CornerRadius(CornerRadiusHelper::FromUniformRadius(8));
        border.Child(img);
        sp.Children().Append(border);
        
        TextBlock tb;
        tb.Text(e.displayName);
        tb.Foreground(SolidColorBrush{ Colors::Orange() });
        tb.Margin(ThicknessHelper::FromUniformLength(4));

        tb.Tapped([=](auto&, auto&) {
          TextBox box;
          box.Text(tb.Text());
          box.SelectAll();
          tb.Visibility(Visibility::Collapsed);
          sp.Children().Append(box);
          box.Focus(FocusState::Keyboard);
          box.KeyDown([=](auto&, Xaml::Input::KeyRoutedEventArgs a) {
            bool dismiss = false;
            if (a.Key() == Windows::System::VirtualKey::Enter) {
              tb.Text(box.Text());
              dismiss = true;
            }
            else if (a.Key() == Windows::System::VirtualKey::Escape) {
              dismiss = true;
            }

            if (dismiss) {
              sp.Children().RemoveAtEnd();
              tb.Visibility(Visibility::Visible);
            }
            });
          });
        sp.Children().Append(tb);

        gv.Items().Append(sp);

        sp.RightTapped([=](auto& sender, auto& args) {
          MenuFlyout mf;

          std::vector commands = { L"Command 1", L"Command 2" };
          for (const auto& c : commands) {
            MenuFlyoutItem mfi;
            mfi.Text(c);
            mfi.Icon(SymbolIcon{ Symbol::Comment });
            mf.Items().Append(mfi);
          }
          mf.ShowAt(sp);
          });

        sp.DoubleTapped([=](auto& sender, auto& args) {
          MessageBeep(MB_ICONASTERISK);
          });

      }
      
      c.Children().Append(gv);

      gv.CanReorderItems(true);
      gv.CanDragItems(true);
      gv.IsItemClickEnabled(true);
      gv.AllowDrop(true);
      gv.Drop([=](auto& s, const DragEventArgs& a) {
        Canvas::SetLeft(s.as<UIElement>(), a.GetPosition(c).X);
        Canvas::SetTop(s.as<UIElement>(), a.GetPosition(c).Y);
        });

      desktopXamlSource.Content(c);


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
