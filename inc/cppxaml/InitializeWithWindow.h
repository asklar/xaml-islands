#pragma once
#include <cppxaml/Controls.h>
#include <cppxaml/XamlWindow.h>
#include <type_traits>
#include <ShObjIdl_core.h>

/** @file
* @author Alexander Sklar
* @section LICENSE
 * Copyright (c) Alexander Sklar
 *
 * Licensed under the MIT license
*/

namespace cppxaml {
      /**
     * @brief Initializes an object with a window-like object
     * @tparam TWindow the window-like type. \n
     * In system XAML, it must have a XamlRoot. \n
     * In WinUI 3, it must implement `IWindowNative` and have a Content property that exposes a XamlRoot (this will usually be Microsoft::UI::Xaml::Window).
     * @param obj The object to attach to the window, e.g. a `ContentDialog`.
     * @param w The window-like object.
     * @return
    */
    template<typename TWindow>
#ifndef DOXY
    std::enable_if_t<
        !std::is_same_v<TWindow, cppxaml::XamlWindow*> &&
        !std::is_same_v<TWindow, HWND>>
#else
    void
#endif
        InitializeWithWindow(winrt::Windows::Foundation::IInspectable obj, TWindow /*winrt::Microsoft::UI::Xaml::Window*/ w) {
#ifdef USE_WINUI3
        if (auto iww = obj.try_as<IInitializeWithWindow>()) {
            HWND hwnd{};
            w.as<IWindowNative>()->get_WindowHandle(&hwnd);
            iww->Initialize(hwnd);
        }
        else if (auto window = w.as<cppxaml::xaml::Window>()) {
            obj.XamlRoot(window.Content().XamlRoot());
        }
        else
#else
        {
            obj.as<winrt::Windows::UI::Xaml::FrameworkElement>().XamlRoot(w.XamlRoot());
        }
#endif
    }

    /**
     * @brief Initializes a `FrameworkElement` with an HWND
     * @param obj
     * @param hwnd
     * @return
    */
    bool InitializeWithWindow(winrt::Windows::Foundation::IInspectable obj, HWND hwnd) {
        if (auto iww = obj.try_as<IInitializeWithWindow>()) {
            iww->Initialize(hwnd);
            return true;
        }
        return false;
    }

    /**
     * @brief Initializes a `FrameworkElement` with the window represented by the `XamlWindow` or the XAML content root that the `XamlWindow` object is hosting
     * @param obj
     * @param xw
    */
    void InitializeWithWindow(winrt::Windows::Foundation::IInspectable obj, const cppxaml::XamlWindow* xw) {
        if (!InitializeWithWindow(obj, xw->hwnd())) {
            if (auto fe = obj.try_as<cppxaml::xaml::FrameworkElement>()) {
                fe.XamlRoot(xw->GetUIElement<cppxaml::xaml::FrameworkElement>().XamlRoot());
            }
        }
    }

    /**
     * @brief
     * @tparam TUIElement
     * @tparam TWindow
     * @param obj
     * @param w
     * @return
    */
    template<typename TUIElement, typename TWindow>
 #ifndef DOXY
    std::enable_if_t<std::is_assignable_v<cppxaml::xaml::UIElement, TUIElement> &&
        !std::is_same_v<TWindow, const cppxaml::XamlWindow*>, void>
#else
    void
#endif
        InitializeWithWindow(cppxaml::details::Wrapper<TUIElement> obj, TWindow /*winrt::Microsoft::UI::Xaml::Window*/ w) {
        return InitializeWithWindow(*obj, w);
    }
}