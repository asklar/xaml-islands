// WindowsProject1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WindowsProject1.h"
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>

#include <winrt/MarkupSample.h>
#include <cppxaml/XamlWindow.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <dwmapi.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Documents.h>
#include <winrt/windows.ui.xaml.controls.h>
#include <cppxaml/Controls.h>
#include <functional>

using namespace winrt;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Hosting;


std::vector<std::wstring> fontNames{};

auto GetFontFamilies() {
    if (fontNames.size() == 0) {
        auto hDC = GetDC(nullptr);
        LOGFONT lf{};
        lf.lfCharSet = DEFAULT_CHARSET;
        int nRet = EnumFontFamiliesEx(hDC, &lf, [](const LOGFONT* lf, const TEXTMETRIC* tm, DWORD fontType, LPARAM lParam) {
            auto& _names = *(std::vector<std::wstring>*)lParam;
            if (std::find(_names.begin(), _names.end(), lf->lfFaceName) == _names.end()) {
                _names.push_back(lf->lfFaceName);
            }
            return 1;
            }, (LPARAM)&fontNames, 0);

        ReleaseDC(nullptr, hDC);
    }

    return fontNames;
}

winrt::fire_and_forget CreateCppXamlUI(const cppxaml::XamlWindow* xw) {
    auto v = std::vector<int>{ 1,2,3 };
    auto w = cppxaml::utils::transform(v, [](const int& t) { return t; });
    auto z = cppxaml::utils::transform_with_index(v, [](const int& t, std::vector<int>::const_iterator::difference_type dt) { return t + dt; });


    auto gl = cppxaml::details::GridLength2(10);
    auto gl2 = cppxaml::details::GridLength2("*");
    auto gl3 = cppxaml::details::GridLength2("Auto");
    auto gl4 = cppxaml::details::GridLength2("2*");

    auto gc = cppxaml::details::GridColumns({ 10, 20 });
    auto gc2 = cppxaml::details::GridColumns({ 10,  {"*"}, {"Auto"} });

    auto gr = cppxaml::details::GridRows("10, 20, *");
    auto gr2 = cppxaml::details::GridRows{ "10, 20, *, Auto" };
    auto gr3 = cppxaml::details::GridRows{ 10, 20, {"*"}, {"Auto"} };

    auto strs = std::vector<std::wstring>{ L"first", L"second", L"third", L"fourth" };

    auto lambda = [](const std::wstring& t, std::vector<std::wstring>::const_iterator::difference_type index) {
        return cppxaml::details::UIElementInGrid{
            (int)index / 2,
            (int)index % 2,
            cppxaml::Button(winrt::hstring(t))
        };
    };

    auto button_1 = cppxaml::Button(L"my button")
        .Set(cppxaml::xaml::Controls::Grid::RowProperty(), 1)
        .Set(cppxaml::xaml::Controls::Grid::ColumnProperty(), 1);

//    auto grid = Markup::XamlReader::Load(LR"(
//<Grid
//    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
//    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
//    Background="Transparent"
//>
//    <VisualStateManager.VisualStateGroups>
//        <VisualStateGroup x:Name="VSG1">
//            <VisualState x:Name="VS1">
//                <Storyboard x:Name='SB1'>
//                    <ObjectAnimationUsingKeyFrames Storyboard.TargetName='Rect1' Storyboard.TargetProperty='Fill'>
//                        <DiscreteObjectKeyFrame KeyTime='0:0:0' Value='Green' />
//                    </ObjectAnimationUsingKeyFrames>
//                </Storyboard>
//            </VisualState>
//            <VisualState x:Name="state2"/>
//        </VisualStateGroup>
//    </VisualStateManager.VisualStateGroups>
//    <Rectangle x:Name='Rect1' Fill='Red' Width='100' />
//</Grid>
//)").as<Controls::Grid>();

    auto sv = cppxaml::MakeContentControl<cppxaml::xaml::Controls::ScrollViewer>({
            cppxaml::StackPanel({
                cppxaml::Grid(
                    {"40, *"}, {"Auto, Auto"},
                    cppxaml::utils::transform_with_index(strs,
                        [](const std::wstring& t, auto index) {
                            return cppxaml::details::UIElementInGrid{
                                (int)index / 2,
                                (int)index % 2,
                                cppxaml::Button(winrt::hstring(t))
                            };
                      })
                    ),
                //cppxaml::details::Wrapper<Controls::ColorPicker>().Name(L"cp"),
                cppxaml::AutoSuggestBox(GetFontFamilies())
                    .EnableDefaultSearch()
                    .Margin(0, 16, 0, 4)
                    .Name(L"fontTB"),
            }).Orientation(Controls::Orientation::Vertical).Name(L"stackpanel")
        });

    auto cd = cppxaml::ContentDialog(sv).PrimaryButtonText(L"Ok");

    cd->Loaded([sv](auto&...) {
        auto stackpanel = cppxaml::FindChildByName< Controls::StackPanel>(sv, L"stackpanel");
        stackpanel = sv->FindName(L"stackpanel").as<Controls::StackPanel>();

//        auto button = Markup::XamlReader::Load(LR"(
//<Button
//    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
//    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
//    Content="Hello world"
//>
//    <Control.Template>
//        <ControlTemplate>
//            <VisualStateGroup x:Name="VSG1">
//                <VisualState x:Name="VS1">
//                </VisualState>
//                <VisualState x:Name="state2"/>
//            </VisualStateGroup>
//        </ControlTemplate>
//    </Control.Template>
//</Button>
//)").as<Controls::Button>();
//
//        stackpanel.Children().Append(button);
//
//        try {
//            auto vsm = MarkupSample::CustomVSM();
//            winrt::Windows::UI::Xaml::VisualStateManager::SetCustomVisualStateManager(button, vsm);
//            auto vsmGroup = button.FindName(L"VSG1").as<VisualStateGroup>();
//            auto vsmState = button.FindName(L"VS1").as<VisualState>();
//            //auto gsc = &winrt::impl::consume_MarkupSample_ICustomVSMOverrides<winrt::MarkupSample::CustomVSM>::GoToStateCore;
//            //(vsm.*gsc)(Controls::Button(), grid, L"VS1", vsmGroup, vsmState, true);
//            // 
//
//
//            //        vsm.GoToStateCore(Controls::Button(), grid, L"VS1", vsmGroup, vsmState, true);
//
//            VisualStateManager::GoToState(button, L"VS1", true);
//            //(& vsm)->*(gsc)(Controls::Button(), grid, L"VS1", vsmGroup, vsmState, true);
//            //VisualStateManager::GoToState(grid, L"state1", true);
//
//        }
//        catch (const winrt::hresult_error& hr) {
//            auto x = hr.message();
//        }
//
//
        });
    auto fontTB = cppxaml::FindChildByName<Controls::AutoSuggestBox>(*cd, L"fontTB");

    cppxaml::InitializeWithWindow(cd, xw);
    auto res = co_await cd->ShowAsync();
    if (res == cppxaml::xaml::Controls::ContentDialogResult::Primary) {
        fontTB = cppxaml::FindChildByName<Controls::AutoSuggestBox>(*cd, L"fontTB");
        auto fontName = fontTB.Text();
    }

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    auto winuiIXMP = winrt::Microsoft::UI::Xaml::XamlTypeInfo::XamlControlsXamlMetaDataProvider();
    auto markupIXMP = winrt::MarkupSample::XamlMetaDataProvider();

    auto xapp = winrt::Microsoft::Toolkit::Win32::UI::XamlHost::XamlApplication({ winuiIXMP, markupIXMP });

    cppxaml::AppController controller(hInstance, xapp);

    controller.OnUICreated = [](UIElement page, cppxaml::XamlWindow* xw) {
        if (auto mainPage = page.try_as<MarkupSample::MainPage>()) {
            assert(xw->Id() == L"MarkupSample");
            mainPage.InterfaceStr(L"This string comes from the win32 app");

            mainPage.Link().Click([](auto&...) {

                auto& modalWindow = cppxaml::XamlWindow::Get(L"Modal");
                auto& mainWindow = cppxaml::XamlWindow::Get(L"MarkupSample");

                CreateCppXamlUI(&mainWindow);


                modalWindow.Create(L"Modal", 0 /*WS_POPUPWINDOW*/, mainWindow.hwnd(), 600 * GetDpiForSystem() / 96, 600 * GetDpiForSystem() / 96);
                EnableWindow(mainWindow.hwnd(), FALSE);
                });

            auto menuFlyout = cppxaml::MenuFlyout({
                                cppxaml::MenuFlyoutItem(L"menu option1"),
                                cppxaml::MenuFlyoutItem(L"menu option2")
                                    .IconElement(cppxaml::FontIcon(L"\xe701")),
                }).CentralizedHandler(
                    [](Windows::Foundation::IInspectable sender, Input::TappedRoutedEventArgs args) {
                        auto x = sender.as<cppxaml::xaml::Controls::MenuFlyoutItem>().Text();
                    });

            mainPage.OkButton().Tapped([xw](Windows::Foundation::IInspectable, auto&) {

                /* 
                // The following code breaks the build inside a lambda, but works fine outside it 
                auto menuFlyout = cppxaml::MenuFlyout({
                    cppxaml::MenuFlyoutItem(L"menu option1"),
                    cppxaml::MenuFlyoutItem(L"menu option2")
                        .IconElement(cppxaml::FontIcon(L"\xe701")),
                    }).CentralizedHandler([](Windows::Foundation::IInspectable sender, Input::TappedRoutedEventArgs args) {
                        auto x = sender.as<cppxaml::xaml::Controls::MenuFlyoutItem>().Text();
                        }); 
                    cppxaml::InitializeWithWindow(menuFlyout, xw);
                    menuFlyout->ShowAt(mainPage.OkButton());
                            */

                DestroyWindow(xw->hwnd());
                });
        }
        else if (auto modalPage = page.try_as<MarkupSample::ModalPage>()) {
            assert(xw->Id() == L"Modal");
            modalPage.OkClicked([xw](auto&...) {
                DestroyWindow(xw->hwnd());
                SetFocus(cppxaml::XamlWindow::Get(L"MarkupSample").hwnd());
                });
        }
    };

    controller.WndProc =
        [](HWND hWnd, INT message, WPARAM wParam, LPARAM lParam, cppxaml::XamlWindow* xw) -> LRESULT {
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
                SetFocus(cppxaml::XamlWindow::Get(L"MarkupSample").hwnd());
                return 0;
            }
            break;
        }
        case WM_DESTROY:
            if (xw->Id() == L"MarkupSample") {
                PostQuitMessage(0);
            }
            else if (xw->Id() == L"Modal") {
                EnableWindow(cppxaml::XamlWindow::Get(L"MarkupSample").hwnd(), TRUE);
            }
            break;
        }
        return DefWindowProcW(hWnd, message, wParam, lParam);
    };

    xapp.Resources().MergedDictionaries().Append(winrt::Microsoft::UI::Xaml::Controls::XamlControlsResources());
    auto& mainWindow = cppxaml::XamlWindow::Make<MarkupSample::MainPage>(L"MarkupSample", &controller);
    auto& modalWindow = cppxaml::XamlWindow::Make<MarkupSample::ModalPage>(L"Modal", &controller);

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
