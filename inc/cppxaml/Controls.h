#pragma once
#include <winrt/windows.ui.xaml.h>
#include <winrt/windows.ui.xaml.media.h>
#include <winrt/windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <type_traits>
#include <ShObjIdl_core.h>

#include <cppxaml/utils.h>
#ifdef USE_WINUI3
#include <microsoft.ui.xaml.window.h>
#endif

/** @file
* @author Alexander Sklar
* @section LICENSE
* MIT
*/

#ifndef DOXY
#define IF_ASSIGNABLE_CONTROL(XAMLTYPE)     std::enable_if_t<std::is_assignable_v<cppxaml::xaml::Controls::XAMLTYPE, T>, cppxaml::details::Wrapper<T>>
#else
#define IF_ASSIGNABLE_CONTROL(XAMLTYPE)     cppxaml::details::Wrapper<T>
#endif

/**
 * @namespace cppxaml
 * @brief The main CppXAML namespace
*/
namespace cppxaml {

    namespace details {
        /**
         * @brief Internal wrapper type that powers fluent-style programming. \n
         * This type is usually constructed and destructed in a single declaration line, only serving to set properties on the underlying XAML element, or to add the element to a parent.
         * @tparam T the XAML element type to hold.
        */
        template<typename T>
        struct WrapperT {
            T m_value{ nullptr };

            WrapperT(const T& v = T()) : m_value(v) {}

            T operator*() const {
                return m_value;
            }
            const T* operator->() const { return &m_value; }

            /**
             * @brief Returns the element's Name
             * @return
            */
            auto Name() const { return m_value.Name(); }
            /**
             * @brief Set the element's Name
             * @param n
             * @return
            */
            auto Name(std::wstring_view n) const { m_value.Name(n); return *this; }

            /**
             * @brief
             * @return
            */
            auto Margin() const { return m_value.Margin(); }
            /**
             * @brief
             * @param t
             * @return
            */
            auto Margin(cppxaml::xaml::Thickness t) const { m_value.Margin(t); return *this; }
            /**
             * @brief
             * @param m
             * @return
            */
            auto Margin(double m) const { m_value.Margin(cppxaml::xaml::ThicknessHelper::FromUniformLength(m)); return *this; }
            /**
             * @brief
             * @param l
             * @param t
             * @param r
             * @param b
             * @return
            */
            auto Margin(double l, double t, double r, double b) const {
                m_value.Margin(cppxaml::xaml::ThicknessHelper::FromLengths(l, t, r, b)); return *this;
            }

            /**
             * @brief
             * @return
            */
            auto Padding() const { return m_value.Padding(); }
            /**
             * @brief
             * @param t
             * @return
            */
            auto Padding(cppxaml::xaml::Thickness t) const { m_value.Padding(t); return *this; }
            /**
             * @brief
             * @param m
             * @return
            */
            auto Padding(double m) const { m_value.Padding(cppxaml::xaml::ThicknessHelper::FromUniformLength(m)); return *this; }
            /**
             * @brief
             * @param l
             * @param t
             * @param r
             * @param b
             * @return
            */
            auto Padding(double l, double t, double r, double b) const {
                m_value.Padding(cppxaml::xaml::ThicknessHelper::FromLengths(l, t, r, b)); return *this;
            }

            template<typename D, std::enable_if_t<std::is_assignable_v<D, T>, bool> = true>
            operator D() const {
                return m_value;
            }

            /**
             * @brief Sets a property
             * @tparam TValue
             * @param dp The DependencyProperty to set
             * @param value The value
             * @return
            */
            template<typename TValue>
            auto Set(cppxaml::xaml::DependencyProperty dp, TValue&& value) {
                if constexpr (std::is_assignable_v<winrt::Windows::Foundation::IInspectable, TValue>) {
                    m_value.SetValue(dp, std::move(value));
                }
                else {
                    m_value.SetValue(dp, winrt::box_value(std::move(value)));
                }
                return *this;
            }
        };

        template<typename T, typename TItems = void>
        struct Wrapper : WrapperT<T> {};

        /**
         * @brief Fluent-style wrapper for `ContentDialog`
        */
        template<>
        struct Wrapper<cppxaml::xaml::Controls::ContentDialog> : WrapperT<cppxaml::xaml::Controls::ContentDialog> {
            auto PrimaryButtonText(std::wstring_view t) {
                this->m_value.as<cppxaml::xaml::Controls::ContentDialog>().PrimaryButtonText(t);
                return *this;
            }
        };

        /**
         * @brief Fluent-style wrapper for `StackPanel`
        */
        template<>
        struct Wrapper<cppxaml::xaml::Controls::StackPanel> : WrapperT<cppxaml::xaml::Controls::StackPanel> {
            auto Orientation(cppxaml::xaml::Controls::Orientation o) {
                this->m_value.as<cppxaml::xaml::Controls::StackPanel>().Orientation(o);
                return *this;
            }
        };

        /**
         * @brief Fluent-style wrapper for `AutoSuggestBox`
         * @tparam TItems the type of collection from which to initialize the `AutoSuggestBox`'s `Items`.
        */
        template<typename TItems>
        struct Wrapper<cppxaml::xaml::Controls::AutoSuggestBox, TItems> : WrapperT<cppxaml::xaml::Controls::AutoSuggestBox>, std::enable_shared_from_this<Wrapper<cppxaml::xaml::Controls::AutoSuggestBox, TItems>> {
            TItems m_items;

            bool m_caseInsensitive{ false };
            Wrapper() = delete;
            Wrapper(const TItems& items) : m_items(items) {}
            Wrapper(const Wrapper<cppxaml::xaml::Controls::AutoSuggestBox, TItems>& other) : m_items(other.m_items) {
                if (other.m_textChangedToken) {
                    SetEventHandlers(other.m_caseInsensitive);
                }
            }
            Wrapper(Wrapper&& other) : m_items(other.m_items),
                m_textChangedToken(std::move(other.m_textChangedToken)),
                m_suggestionChosenToken(std::move(other.m_suggestionChosenToken))
            {
                other.m_textChangedToken = {};
                other.m_suggestionChosenToken = {};
            }

            ~Wrapper() {
                // Wrapper objects get destroyed as their only purpose is to facilitate creating the wrapped XAML objects
                // However, wrapper objects set up event handlers (like these AutoSuggest ones). Therefore these event handlers must:
                //      a) not use any state from Wrapper
                //      b) not be de-registered upon Wrapper's destructor
                //      c) be idempotent (as more than one event handler will likely run, as Wrappers can get copy-constructed/move constructed around).

                //if (m_textChangedToken) {
                //    m_value.TextChanged(m_textChangedToken);
                //}
                //if (m_suggestionChosenToken) {
                //    m_value.SuggestionChosen(m_suggestionChosenToken);
                //}
            }
        private:
            winrt::event_token m_textChangedToken{};
            winrt::event_token m_suggestionChosenToken{};
            void SetEventHandlers(bool caseInsensitive) {
                m_caseInsensitive = caseInsensitive;
                m_textChangedToken = m_value.TextChanged([caseInsensitive, items = this->m_items](cppxaml::xaml::Controls::AutoSuggestBox sender, cppxaml::xaml::Controls::AutoSuggestBoxTextChangedEventArgs args) {
                    if (args.Reason() == cppxaml::xaml::Controls::AutoSuggestionBoxTextChangeReason::UserInput) {
                        std::wstring search = caseInsensitive ? utils::tolower(sender.Text()) : sender.Text().c_str();
                        auto vec = TItems();

                        for (const auto& entry : items) {
                            auto str = caseInsensitive ? utils::tolower(entry) : entry.c_str();
                            if (search == L"" || str.find(search) != -1) {
                                vec.push_back(entry);
                            }
                        }
                        auto suitableItems = winrt::single_threaded_vector<winrt::Windows::Foundation::IInspectable>();
                        winrt::Windows::Foundation::IInspectable selected{ nullptr };
                        for (const auto& e : vec) {
                            auto boxed = winrt::box_value(e);
                            if (utils::tolower(e) == search && search != L"") {
                                selected = boxed;
                            }
                            suitableItems.Append(boxed);
                        }
                        sender.ItemsSource(suitableItems);
                        if (selected) {
                            sender.Text(winrt::unbox_value<winrt::hstring>(selected));
                        }
                    }
                });
                m_suggestionChosenToken = m_value.SuggestionChosen([](cppxaml::xaml::Controls::AutoSuggestBox sender, cppxaml::xaml::Controls::AutoSuggestBoxSuggestionChosenEventArgs args) {
                    sender.Text(winrt::unbox_value<winrt::hstring>(args.SelectedItem()));
                    });

            }
        public:
            /**
             * @brief Enables a default search experience for the AutoSuggestBox. The list of items shown gets filtered as the user searches, leaving only items that include the search string.
             * @param caseInsensitive Whether the search should be case insensitive or not. Default is true.
             * @return
            */
            auto EnableDefaultSearch(bool caseInsensitive = true) {
                SetEventHandlers(caseInsensitive);
                return *this;
            }
        };

        template<typename TItems>
        struct Wrapper<cppxaml::xaml::Controls::ItemsControl, TItems> : WrapperT<cppxaml::xaml::Controls::ItemsControl> {
            using items_t = TItems;

        };

        template<>
        struct Wrapper<cppxaml::xaml::Controls::MenuFlyoutItem> : WrapperT<cppxaml::xaml::Controls::MenuFlyoutItem> {
            cppxaml::xaml::Controls::IconElement IconElement() const { return m_value.Icon(); }
            auto IconElement(cppxaml::xaml::Controls::IconElement icon) const {
                m_value.Icon(icon); return *this;
            }
            winrt::event_token m_tappedToken{};
            auto Tapped() const {
                return m_tappedToken;
            }
            auto Tapped(cppxaml::xaml::Input::TappedEventHandler handler) { m_tappedToken = m_value.Tapped(handler); return *this; }
        };

        template<>
        struct Wrapper<cppxaml::xaml::Controls::MenuFlyout> : WrapperT<cppxaml::xaml::Controls::MenuFlyout> {
            template<typename F>
            auto CentralizedHandler(const F& f) {
                if (m_eventHandlers.size() != 0) {
                    throw std::exception("Centralized handler already set");
                }

                for (auto i : m_value.Items()) {
                    m_eventHandlers.push_back(i.Tapped([f](winrt::Windows::Foundation::IInspectable sender, cppxaml::xaml::Input::TappedRoutedEventArgs args) {
                        f(sender, args);
                        }));
                }
                return *this;
            }
            std::vector<winrt::event_token> m_eventHandlers{};
            ~Wrapper() {
                for (auto i = 0u; i < m_value.Items().Size(); i++) {
                    m_value.Items().GetAt(i).Tapped(m_eventHandlers[i]);
                }
            }
        };

        struct GridLength2 {
            operator cppxaml::xaml::GridLength() const { return m_length; }
            GridLength2(double len) : m_length(cppxaml::xaml::GridLengthHelper::FromPixels(len)) {}
            GridLength2(std::wstring_view v) {
                m_length = winrt::unbox_value<cppxaml::xaml::GridLength>(cppxaml::xaml::Markup::XamlBindingHelper::ConvertValue(winrt::xaml_typename<cppxaml::xaml::GridLength>(), winrt::box_value(v)));
            }
            GridLength2(std::string_view v) : GridLength2(winrt::to_hstring(v)) {}
        private:
            cppxaml::xaml::GridLength m_length;
        };
        struct GridLengths {
            std::vector<cppxaml::xaml::GridLength> m_lengths;
            GridLengths(std::initializer_list<GridLength2>&& ls) {
                for (auto& l : ls) {
                    m_lengths.push_back(l);
                }
            }
            GridLengths(std::string_view sv) {
                std::string_view last;
                for (std::string_view::size_type pos = 0; pos < sv.length(); ) {
                    auto nextDelimiter = sv.find(",", pos);
                    last = sv.substr(pos, nextDelimiter - pos);
                    auto len = winrt::unbox_value<cppxaml::xaml::GridLength>(cppxaml::xaml::Markup::XamlBindingHelper::ConvertValue(winrt::xaml_typename<cppxaml::xaml::GridLength>(), winrt::box_value(winrt::to_hstring(last))));
                    m_lengths.push_back(len);

                    if (nextDelimiter != -1) {
                        pos = nextDelimiter + 1;
                    }
                    else {
                        break;
                    }
                }
            }
        };

        using GridRows = GridLengths;
        using GridColumns = GridLengths;

        struct UIElementInGrid {
            int m_row{};
            int m_column{};
            cppxaml::xaml::UIElement m_element{ nullptr };
        };

    } // namespace details

    /**
     * @brief Create a XAML element of a class that derives from `ContentControl`
     * @tparam T The XAML type, e.g. `Button`.
     * Defaults to `void`.\n
     * @param i The object that will be set as the `Content` property.
     * @return
    */
    template<typename T>
    IF_ASSIGNABLE_CONTROL(ContentControl)
        MakeContentControl(winrt::Windows::Foundation::IInspectable i) {
        cppxaml::details::Wrapper<T> t;
        t->Content(i);
        return t;
    }

    /**
     * @brief Create a XAML element of a class that derives from `ContentControl` by wrapping a string as its content.
     * @tparam T The XAML type, e.g. `Button`.
     * @param v The string that will be set as the `Content` property.
     * @return
    */
    template<typename T>
    IF_ASSIGNABLE_CONTROL(ContentControl)
        MakeContentControl(winrt::hstring v) {
        cppxaml::details::Wrapper<T> t;
        t->Content(winrt::box_value(v));
        return t;
    }

    /**
     * @brief Creates a XAML element of a type that is a subclass of `Panel`.
     * @tparam T The XAML type.
     * @param elems The list of `UIElements`
     * @return
    */
    template<typename T>
    std::enable_if_t<std::is_assignable_v<cppxaml::xaml::Controls::Panel, T>, cppxaml::details::Wrapper<T>>
        MakePanel(const std::initializer_list<cppxaml::xaml::UIElement>& elems) {
        cppxaml::details::Wrapper<T> panel;
        for (auto e : elems) {
            panel->Children().Append(e);
        }
        return panel;
    }

    /**
     * @brief Creates a `ContentDialog`
     * @tparam C Type of list of child controls
     * @param i The list of child controls
     * @return
    */
    template<typename C>
    auto ContentDialog(C i) {
        return MakeContentControl<cppxaml::xaml::Controls::ContentDialog>(i);
    }

    /**
     * @brief Creates a `StackPanel`.
     * @param elems The list of child controls.
     * @return
    */
    auto StackPanel(const std::initializer_list<cppxaml::xaml::UIElement>& elems) {
        return MakePanel<cppxaml::xaml::Controls::StackPanel>(elems);
    }

    /**
     * @brief Creates a `Grid` with the specified rows, columns, and children.
     * @details
     * The row and column definitions are specified with the same syntax.\n
     * This will be either an initializer list where each element is a height (a `double`, or the strings `"Auto"`, or `"N*"` where `N` is an integer), or a string with the corresponding format. \n\n
     * Example usage:\n
     * @code
     *      auto grid = cppxaml::Grid({"40, *"}, {"Auto, Auto"}, {
     *           {0, 0, cppxaml::TextBlock(L"first") },
     *           {0, 1, cppxaml::TextBlock(L"second") },
     *           {1, 0, cppxaml::TextBlock(L"third") },
     *           {1, 1, cppxaml::TextBlock(L"fourth") },
     *           }),
     * @endcode
     * @param gr The set of grid row definitions.
     * @param gc The set of grid column definitions.
     * @param elems A list of cppxaml::details::UIElementInGrid entries where each entry is a `{rowNumber, columnNumber, myUIElement}`
     * @return
    */
    template<typename TElements>
    auto Grid(details::GridRows gr, cppxaml::details::GridColumns gc, TElements /*std::initializer_list<details::UIElementInGrid>& */&& elems) {
        auto grid = cppxaml::details::Wrapper<cppxaml::xaml::Controls::Grid>();
        for (auto& r : gr.m_lengths) {
            auto rd = cppxaml::xaml::Controls::RowDefinition();
            rd.Height(r);
            grid->RowDefinitions().Append(rd);
        }
        for (auto& c : gc.m_lengths) {
            auto cd = cppxaml::xaml::Controls::ColumnDefinition();
            cd.Width(c);
            grid->ColumnDefinitions().Append(cd);
        }

        for (auto& e : elems) {
            grid->Children().Append(e.m_element);
            cppxaml::xaml::Controls::Grid::SetRow(e.m_element.as<cppxaml::xaml::FrameworkElement>(), e.m_row);
            cppxaml::xaml::Controls::Grid::SetColumn(e.m_element.as<cppxaml::xaml::FrameworkElement>(), e.m_column);
        }
        return grid;
    }

    /**
     * @brief Creates a `TextBox`
     * @param text The text to create the control from
     * @return
    */
    auto TextBox(std::wstring_view text = L"") {
        cppxaml::details::Wrapper<cppxaml::xaml::Controls::TextBox> tb;
        tb->Text(text);
        return tb;
    }

    /**
     * @brief Creates a XAML element of a type that derives from `ItemsControl`
     * @tparam T The XAML type, e.g. `AutoSuggestBox`.
     * @tparam TItems The type of the collection of items that might be used as seed to construct the control.\n
     * For example, this could be a `std::vector<std::wstring>` that is used to construct the `Items` collection for an `AutoSuggestBox`.\n
     * Defaults to `void`.\n
     * @param items The list of items to initialize the control with.
     * @return
    */
    template<typename T, typename TItems>
    std::enable_if_t<std::is_assignable_v<cppxaml::xaml::Controls::ItemsControl, T>, cppxaml::details::Wrapper<T, TItems>>
        MakeItemsControl(const TItems& /*std::initializer_list<Windows::Foundation::IInspectable>*/ items) {
        cppxaml::details::Wrapper<T, TItems> t(items);
        for (auto& i : items) {
            if constexpr (std::is_assignable_v<winrt::Windows::Foundation::IInspectable, decltype(i)>) {
                t->Items().Append(i);
            }
            else {
                t->Items().Append(winrt::box_value(i));
            }
        }
        return t;
    }

    /**
     * @brief Creates an `AutoSuggestBox` from a list of items
     * @tparam TItems
     * @param svs a list of `wstring_view` to populate the control from.
     * @return
    */
    template<typename TItems>
    auto AutoSuggestBox(const TItems& /*std::initializer_list<std::wstring_view>*/ svs) {
        return MakeItemsControl<cppxaml::xaml::Controls::AutoSuggestBox, TItems>(svs);
    }

    /**
     * @brief Creates a `TextBlock` from its text content.
     * @param text The text for the control.
     * @return
    */
    auto TextBlock(std::wstring_view text) {
        cppxaml::xaml::Controls::TextBlock tb;
        tb.Text(text);
        return tb;
    }

    /**
     * @brief Creates a Button
     * @tparam C The type of the content to initialize the button with.
     * @param c The content for the button, e.g. a string, a `TextBlock`, or a panel with some children, etc.
     * @return
    */
    template<typename C>
    auto Button(C c) {
        return MakeContentControl<cppxaml::xaml::Controls::Button>(c);
    }

    auto FontIcon(std::wstring_view icon) {
        cppxaml::details::Wrapper<cppxaml::xaml::Controls::FontIcon> fi;
        fi->Glyph(icon);
        return fi;
    }

    /**
     * @brief Creates a MenuFlyoutItem with the specified text
     * @param text 
     * @return 
    */
    auto MenuFlyoutItem(std::wstring_view text) {
        cppxaml::details::Wrapper<cppxaml::xaml::Controls::MenuFlyoutItem> mfi;
        mfi->Text(text);
        return mfi;
    }

    /**
     * @brief Creates a `MenuFlyout` from a list of `MenuFlyoutItems`
     * @param items 
     * @return 
    */
    auto MenuFlyout(std::initializer_list<cppxaml::xaml::Controls::MenuFlyoutItemBase>&& items) {
        cppxaml::details::Wrapper<cppxaml::xaml::Controls::MenuFlyout> mf;
        for (auto& mfi : items) {
            mf->Items().Append(mfi);
        }
        return mf;
    }

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
    std::enable_if_t<
        !std::is_same_v<TWindow, cppxaml::XamlWindow*> &&
        !std::is_same_v<TWindow, HWND>>
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
    std::enable_if_t<std::is_assignable_v<cppxaml::xaml::UIElement, TUIElement> &&
        !std::is_same_v<TWindow, const cppxaml::XamlWindow*>, void>
        InitializeWithWindow(cppxaml::details::Wrapper<TUIElement> obj, TWindow /*winrt::Microsoft::UI::Xaml::Window*/ w) {
        return InitializeWithWindow(*obj, w);
    }
}