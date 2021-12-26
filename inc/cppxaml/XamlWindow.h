#pragma once
#include <winrt/base.h>
#include <winrt/Microsoft.Toolkit.Win32.UI.XamlHost.h>
#include <winrt/Windows.UI.Xaml.h>
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>
#include <mutex>

/** @file
* @author Alexander Sklar
* @section LICENSE
 * Copyright (c) Alexander Sklar
 *
 * Licensed under the MIT license
*/

namespace cppxaml {
    struct XamlWindow;

    /**
     * @brief `AppController` is responsible for coordinating XamlWindow instances, can extend their wndproc, and provides an opportunity to hook up event handlers once a XAML UI becomes live.
    */
    struct AppController {
    public:
        /**
         * @brief App-provided callback to be called when any of this controller's windows create their UI (usually Pages).
        */
        void (*OnUICreated)(winrt::Windows::UI::Xaml::UIElement content, XamlWindow* xw) { nullptr };
        LRESULT(*WndProc)(HWND, INT, WPARAM, LPARAM, XamlWindow*) { nullptr };

        /**
         * @brief
         * @param hInst 
         * @param xapp 
        */
        AppController(HINSTANCE hInst, winrt::Windows::UI::Xaml::Application xapp) {
            m_hInstance = hInst;
            m_xapp = xapp;
            m_winxamlmanager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
        }

        HINSTANCE HInstance() const {
            return m_hInstance;
        }

    private:
        winrt::Windows::UI::Xaml::Application m_xapp{ nullptr };
        HINSTANCE m_hInstance{ nullptr };
        winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager m_winxamlmanager{ nullptr };
    };

    /**
     * @brief Implements an HWND based host for XAML Islands. You can create a `XamlWindow` from one of three overloads of XamlWindow::Make.
    */
    struct XamlWindow {
    private:
        XamlWindow(std::wstring_view id) : m_Id(id) {}
        winrt::Windows::UI::Xaml::UIElement(*m_getUI) (const XamlWindow& xw) { nullptr };

        // This DesktopWindowXamlSource is the object that enables a non-UWP desktop application 
        // to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
        winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource m_desktopXamlSource{ nullptr };
        HWND m_hWnd{ nullptr };
        HACCEL m_hAccelTable{ nullptr };

        AppController* m_controller{ nullptr };
        std::wstring m_Id;
        std::wstring m_markup;
        winrt::Windows::UI::Xaml::UIElement m_ui{ nullptr };

        static inline std::unordered_map<std::wstring, XamlWindow> s_windows{};
    public:
        XamlWindow(const XamlWindow&) = delete;
        XamlWindow(XamlWindow&& other) noexcept :
            m_Id(std::move(other.m_Id)),
            m_getUI(std::move(other.m_getUI)),
            m_markup(std::move(other.m_markup)),
            m_desktopXamlSource(std::move(other.m_desktopXamlSource)),
            m_hWnd(std::move(other.m_hWnd)),
            m_hAccelTable(std::move(m_hAccelTable)),
            m_controller(std::move(other.m_controller))
        {}

        /**
         * @brief returns the HWND backing the XamlWindow.
         * @return 
        */
        HWND hwnd() const {
            return m_hWnd;
        }

        /**
         * @brief returns the XAML UI that the XamlWindow was created with.
         * @tparam T XAML type to cast to; defaults to UIElement.
         * @return 
        */
        template<typename T = winrt::Windows::UI::Xaml::UIElement>
        T GetUIElement() const {
            return m_ui.as<T>();
        }


        /**
         * @brief Creates a window that hosts a XAML UI element 
         * @tparam TUIElement the XAML type to instantiate, usually a subclass of Page; this is usually defined by a separate WinRT Runtime component project, that the XAML Islands project references via C++/WinRT.
         * @param id 
         * @param controller 
         * @return 
         * @details Usage: \n
         * @code
         *  auto& mainWindow = cppxaml::XamlWindow::Make<MarkupSample::MainPage>(L"MarkupSample", &controller);
         * @endcode
         */
        template<typename TUIElement>
        static XamlWindow& Make(std::wstring_view id, AppController* controller = nullptr) {
            XamlWindow xw(id);
            xw.m_controller = controller;
            xw.m_getUI = [](const XamlWindow&) { return TUIElement().as<winrt::Windows::UI::Xaml::UIElement>(); };
            const auto& entry = s_windows.emplace(id, std::move(xw));
            return entry.first->second;
        }

        /**
         * @brief Creates a window that hosts XAML UI, based on some markup provided as an input parameter.
         * @param id 
         * @param markup XAML markup string to use to create the UI.
         * @param c 
         * @return 
         * @details 
         * Usage: \n
         * @code
         *  auto& xw = cppxaml::XamlWindow::Make(L"MyPage", LR"(
         *      <StackPanel>
         *        <TextBlock>Hello</TextBlock>
         *      </StackPanel>)", &controller);
         * @endcode
        */
        static XamlWindow& Make(std::wstring_view id, std::wstring_view markup, AppController* c = nullptr) {
            XamlWindow xw(id);
            xw.m_controller = c;
            xw.m_markup = markup;
            xw.m_getUI = [](const XamlWindow& xw) {
                return winrt::Windows::UI::Xaml::Markup::XamlReader::Load(xw.m_markup)
                    .as<winrt::Windows::UI::Xaml::UIElement>();
            };
            const auto& entry = s_windows.emplace(id, std::move(xw));
            return entry.first->second;
        }

        /**
         * @brief Creates a window that hosts XAML UI. The UI will be provided by the app.
         * @param id 
         * @param getUI App-provided function to create UI. This can e.g. instantiate XAML types programmatically.
         * @param c 
         * @return 
         * @details
         * Usage: \n
         * @code
         * auto& xw = cppxaml::XamlWindow::Make(L"Foo", [](auto&...) { return winrt::Windows::UI::Xaml::Controls::Button(); });
         * @endcode
        */
        static XamlWindow& Make(std::wstring_view id, winrt::Windows::UI::Xaml::UIElement(*getUI)(const XamlWindow&), AppController* c = nullptr) {
            XamlWindow xw(id);
            xw.m_controller = c;
            xw.m_getUI = getUI;
            const auto& entry = s_windows.emplace(id, std::move(xw));
            return entry.first->second;
        }

        /**
         * @brief returns the window's id
         * @return 
        */
        std::wstring_view Id() const {
            return m_Id;
        }
        
        /**
         * @brief returns the XamlWindow corresponding to the id.
         * @param id 
         * @return 
        */
        static XamlWindow& Get(std::wstring_view id) {
            return s_windows.at(id);
        }
        static constexpr std::wstring_view WindowClass() {
            return L"XamlWindow";
        }

        /**
         * @brief Call this function to create the actual window. Afterwards, you will call XamlWindow::RunLoop to process window messages.
         * @param szTitle The window title.
         * @param style The window style. See [Window styles](https://docs.microsoft.com/windows/win32/winmsg/window-styles) and [CreateWindow](https://docs.microsoft.com/windows/win32/api/winuser/nf-winuser-createwindoww).
         * @param parent optional parent window.
         * @param width optional window width.
         * @param height optional window height. 
         * @param nCmdShow Controls how the window is shown. See [ShowWindow](https://docs.microsoft.com/windows/win32/api/winuser/nf-winuser-showwindow).
         * @return 
        */
        HWND Create(std::wstring_view szTitle, DWORD style, HWND parent = nullptr, int width = CW_USEDEFAULT, int height = CW_USEDEFAULT, int nCmdShow = SW_NORMAL) {
            static std::once_flag once;

            std::call_once(once, [ctl = this->m_controller]() {
                WNDCLASSEXW wcex{};
                wcex.cbSize = sizeof(wcex);

                wcex.style = CS_HREDRAW | CS_VREDRAW;
                wcex.lpfnWndProc = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
                {
                    // Get handle to the core window.
                    auto xw = reinterpret_cast<XamlWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
                    if (xw == nullptr && message == WM_CREATE) {
                        auto createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);

                        xw = reinterpret_cast<XamlWindow*>(createStruct->lpCreateParams);
                        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(xw));
                    }


                    return xw ? xw->WndProc(hWnd, message, wParam, lParam) : DefWindowProc(hWnd, message, wParam, lParam);
                };

                wcex.cbWndExtra = sizeof(XamlWindow*);
                wcex.hInstance = ctl->HInstance();
                wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
                wcex.lpszClassName = WindowClass();
                winrt::check_bool(RegisterClassExW(&wcex) != 0);
            });

            m_desktopXamlSource = winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource();
            auto hWnd = CreateWindowW(WindowClass(), szTitle, style,
                CW_USEDEFAULT, CW_USEDEFAULT, width, height, parent, nullptr, m_controller->HInstance(), this);
            if (hWnd) {
                ShowWindow(hWnd, nCmdShow);
                UpdateWindow(hWnd);
            }
            return hWnd;
        }

        /**
         * @brief 
         * @param acc 
        */
        void SetAcceleratorTable(HACCEL acc) {
            m_hAccelTable = acc;
        }

        /**
         * @brief 
         * @return 
        */
        AppController* Controller() const {
            return m_controller;
        }

        /**
         * @brief 
         * @return 
        */
        int RunLoop() {
            MSG msg;

            // Main message loop:
            while (GetMessage(&msg, nullptr, 0, 0))
            {
                if (auto xamlSourceNative2 = m_desktopXamlSource.as<IDesktopWindowXamlSourceNative2>()) {
                    BOOL xamlSourceProcessedMessage = FALSE;
                    winrt::check_hresult(xamlSourceNative2->PreTranslateMessage(&msg, &xamlSourceProcessedMessage));
                    if (xamlSourceProcessedMessage) {
                        continue;
                    }
                }

                if (!TranslateAccelerator(msg.hwnd, m_hAccelTable, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            return (int)msg.wParam;
        }

        HWND GetBridgeWindow() const {
            static HWND hWndXamlIsland = nullptr;
            if (!hWndXamlIsland) {
                auto interop = m_desktopXamlSource.as<IDesktopWindowXamlSourceNative>();
                winrt::check_hresult(interop->get_WindowHandle(&hWndXamlIsland));
            }
            return hWndXamlIsland;
        }

        LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
            auto interop = m_desktopXamlSource.as<IDesktopWindowXamlSourceNative>();

            switch (message)
            {
            case WM_CREATE: {
                this->m_hWnd = hwnd;
                auto createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);

                // Parent the DesktopWindowXamlSource object to the current window.
                winrt::check_hresult(interop->AttachToWindow(m_hWnd));  // This fails due to access violation!

                this->m_ui = this->m_getUI(*this);
                if (m_controller && m_controller->OnUICreated) {
                    m_controller->OnUICreated(m_ui, this);
                }
                m_desktopXamlSource.Content(m_ui);
                break;
            }
            case WM_SIZE:
            {
                HWND hWndXamlIsland = nullptr;
                winrt::check_hresult(interop->get_WindowHandle(&hWndXamlIsland));

                SetWindowPos(hWndXamlIsland, nullptr, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_SHOWWINDOW);

                break;
            }
            case WM_NCDESTROY:
            {
                SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)nullptr);
                break;
            }
            }
            if (m_controller && m_controller->WndProc) {
                return m_controller->WndProc(hwnd, message, wParam, lParam, this);
            }
            else return DefWindowProc(hwnd, message, wParam, lParam);

        }
    };
}
