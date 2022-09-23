#pragma once
#include <winrt/windows.ui.xaml.h>
#include <winrt/windows.ui.xaml.media.h>
#include <winrt/windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <type_traits>

#include <cppxaml/utils.h>
#include <cppxaml/VisualState.h>
#ifdef USE_WINUI3
#include <microsoft.ui.xaml.window.h>
#endif

/** @file
* @author Alexander Sklar
* @section LICENSE
 * Copyright (c) Alexander Sklar
 *
 * Licensed under the MIT license
*/


/**
 * @namespace cppxaml
 * @brief The main CppXAML namespace
*/
namespace cppxaml {

    namespace details {
        /**
         * @brief Internal wrapper type that powers builder-style programming. \n
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
             * @param t `Thickness` struct
             * @return
            */
            auto Margin(cppxaml::xaml::Thickness t) const { m_value.Margin(t); return *this; }
            /**
             * @brief
             * @param m Uniform length
             * @return
            */
            auto Margin(double m) const { m_value.Margin(cppxaml::xaml::ThicknessHelper::FromUniformLength(m)); return *this; }
            /**
             * @brief
             * @param left
             * @param top
             * @param right
             * @param bottom
             * @return
            */
            auto Margin(double left, double top, double right, double bottom) const {
                m_value.Margin(cppxaml::xaml::ThicknessHelper::FromLengths(left, top, right, bottom)); return *this;
            }

            /**
             * @brief
             * @return
            */
            auto Padding() const { return m_value.Padding(); }
            /**
             * @brief
             * @param t `Thickness` struct
             * @return
            */
            auto Padding(cppxaml::xaml::Thickness t) const { m_value.Padding(t); return *this; }
            /**
             * @brief
             * @param m Uniform length
             * @return
            */
            auto Padding(double m) const { m_value.Padding(cppxaml::xaml::ThicknessHelper::FromUniformLength(m)); return *this; }
            /**
             * @brief
             * @param left
             * @param top
             * @param right
             * @param bottom
             * @return
            */
            auto Padding(double left, double top, double right, double bottom) const {
                m_value.Padding(cppxaml::xaml::ThicknessHelper::FromLengths(left, top, right, bottom)); return *this;
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
             * @details Example:\n
             * @code
             * auto buttonInGrid = cppxaml::Button(L"Click me)
             *                          .Set(Grid::RowProperty(), 3)
             *                          .Set(Grid::ColumnProperty(), 2);
             * @endcode
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

            /**
             * @brief Sets up a visual state change listeners
             * @param map A map of visual state names to handlers
             * @return 
             * @details Example:\n
             * @code
             * auto button = cppxaml::Button(L"click me")
                    .VisualStates( {
                        { L"PointerOver", [](auto&sender, cppxaml::xaml::VisualStateChangedEventArgs args) {
                            auto x = args.NewState().Name();
                            auto button = sender.as<Controls::Button>();
                            button.Content(winrt::box_value(x));
                        } },
                        { L"Normal", [](auto&sender, auto&) {
                            auto button = sender.as<Controls::Button>();
                            button.Content(winrt::box_value(L"click me"));
                        } },
                    });
             * @endcode
             * @image html VSM.gif
            */
            auto VisualStates(const std::unordered_map<std::wstring, cppxaml::xaml::VisualStateChangedEventHandler>& map) {
                return cppxaml::VSMListener(*this, map);
            }
        };

        using VisualStateMap = std::unordered_map<std::wstring, cppxaml::xaml::VisualStateChangedEventHandler>;

        template<typename T, typename TItems = void>
        struct Wrapper : WrapperT<T> {};

        /**
         * @brief builder-style wrapper for `ContentDialog`
        */
        template<>
        struct Wrapper<cppxaml::xaml::Controls::ContentDialog> : WrapperT<cppxaml::xaml::Controls::ContentDialog> {
            auto PrimaryButtonText(std::wstring_view t) {
                this->m_value.as<cppxaml::xaml::Controls::ContentDialog>().PrimaryButtonText(t);
                return *this;
            }
        };

        /**
         * @brief builder-style wrapper for `StackPanel`
        */
        template<>
        struct Wrapper<cppxaml::xaml::Controls::StackPanel> : WrapperT<cppxaml::xaml::Controls::StackPanel> {
            auto Orientation(cppxaml::xaml::Controls::Orientation o) {
                this->m_value.as<cppxaml::xaml::Controls::StackPanel>().Orientation(o);
                return *this;
            }
        };

        /**
         * @brief builder-style wrapper for `AutoSuggestBox`
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
                // work around MSVC bug: https://developercommunity.visualstudio.com/t/c3779-when-using-type-in-a-class-method-in-a-templ/1617634
                auto GetReason = [](cppxaml::xaml::Controls::AutoSuggestBoxTextChangedEventArgs const& args) -> cppxaml::xaml::Controls::AutoSuggestionBoxTextChangeReason { return args.Reason(); };
                m_textChangedToken = m_value.TextChanged([GetReason, caseInsensitive, items = this->m_items](cppxaml::xaml::Controls::AutoSuggestBox sender, cppxaml::xaml::Controls::AutoSuggestBoxTextChangedEventArgs args) {
                    if (GetReason(args) == cppxaml::xaml::Controls::AutoSuggestionBoxTextChangeReason::UserInput) {
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

        /**
         * @brief builder-style wrapper for `MenuFlyoutItem`
        */
        template<>
        struct Wrapper<cppxaml::xaml::Controls::MenuFlyoutItem> : WrapperT<cppxaml::xaml::Controls::MenuFlyoutItem> {
            /**
             * @brief 
             * @return 
            */
            cppxaml::xaml::Controls::IconElement IconElement() const { return m_value.Icon(); }
            /**
             * @brief 
             * @param icon 
             * @return 
            */
            auto IconElement(cppxaml::xaml::Controls::IconElement icon) const {
                m_value.Icon(icon); return *this;
            }
            winrt::event_token m_clickToken{};
            /**
             * @brief 
             * @return 
            */
            auto Click() const {
                return m_clickToken;
            }
            /**
             * @brief 
             * @param handler 
             * @return 
            */
            auto Click(cppxaml::xaml::RoutedEventHandler handler) { 
                m_clickToken = m_value.Click(handler);
                return *this; 
            }
        };

        /**
         * @brief builder-style wrapper for `MenuFlyout`
        */
        template<>
        struct Wrapper<cppxaml::xaml::Controls::MenuFlyout> : WrapperT<cppxaml::xaml::Controls::MenuFlyout> {
            /**
             * @brief Sets up a centralized Click handler for all the MenuFlyoutItems in the MenuFlyout
             * @tparam F 
             * @param f the lambda to call when a MenuFlyoutItem is clicked
             * @return 
            */
            template<typename F>
            auto CentralizedHandler(const F& f) {
                if (m_eventHandlers.size() != 0) {
                    throw std::exception("Centralized handler already set");
                }

                for (auto i : m_value.Items()) {
                    if (auto mfi = i.try_as<cppxaml::xaml::Controls::MenuFlyoutItem>()) {
                        m_eventHandlers.push_back(mfi.Click([f](winrt::Windows::Foundation::IInspectable sender, cppxaml::xaml::RoutedEventArgs args) {
                            f(sender, args);
                            }));
                    }
                }
                return *this;
            }
            std::vector<winrt::event_token> m_eventHandlers{};
            ~Wrapper() {
                // Wrapper goes out of scope when ShowAt is called, before the event handlers have had a chance to fire. Don't dismiss them for now.
                /*
                if (m_eventHandlers.size() == m_value.Items().Size()) {
                    for (auto i = 0u; i < m_eventHandlers.size(); i++) {
                        if (auto mfi = m_value.Items().GetAt(i).try_as<cppxaml::xaml::Controls::MenuFlyoutItem>()) {
                            mfi.Click(m_eventHandlers[i]);
                        }
                        else {
                            break; // bail, something is off
                        }
                    }
                }
                */
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
    IF_ASSIGNABLE_CONTROL(Panel)
        MakePanel(const std::initializer_list<cppxaml::xaml::UIElement>& elems) {
        cppxaml::details::Wrapper<T> panel;
        for (auto e : elems) {
            panel->Children().Append(e);
        }
        return panel;
    }

    /**
     * @fn template<typename C> cppxaml::details::Wrapper<cppxaml::xaml::Controls::ContentDialog> ContentDialog(C i)
     * @brief Creates a `ContentDialog`
     * @tparam C Type of list of child controls
     * @param i The list of child controls
     * @return cppxaml::details::Wrapper<cppxaml::xaml::Controls::ContentDialog>
    */
    template<typename C>
    auto ContentDialog(C i) {
        return MakeContentControl<cppxaml::xaml::Controls::ContentDialog>(i);
    }

    /**
     * @brief Creates a `StackPanel`.
     * @param elems The list of child controls.
     * @return cppxaml::details::Wrapper<cppxaml::xaml::Controls::StackPanel>
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
    DOXY_RT(cppxaml::details::Wrapper<cppxaml::xaml::Controls::Grid>) 
        Grid(details::GridRows gr, cppxaml::details::GridColumns gc, TElements /*std::initializer_list<details::UIElementInGrid>& */&& elems) {
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
            if constexpr (std::is_assignable_v<cppxaml::details::UIElementInGrid, decltype(e)>) {
                grid->Children().Append(e.m_element);
                cppxaml::xaml::Controls::Grid::SetRow(e.m_element.as<cppxaml::xaml::FrameworkElement>(), e.m_row);
                cppxaml::xaml::Controls::Grid::SetColumn(e.m_element.as<cppxaml::xaml::FrameworkElement>(), e.m_column);
            }
            else {
                grid->Children().Append(e);
            }
        }
        return grid;
    }

    /**
     * @brief Creates a `TextBox`
     * @param text The text to create the control from
     * @return cppxaml::details::Wrapper<cppxaml::xaml::Controls::TextBox>
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
    IF_ASSIGNABLE_CONTROL_TITEMS(ItemsControl, TItems)
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
     * @fn template<typename TItems> auto AutoSuggestBox(const TItems& svs)
     * @brief Creates an `AutoSuggestBox` from a list of items
     * @tparam TItems
     * @param svs a list of `wstring_view` to populate the control from.
     * @return cppxaml::details::Wrapper<cppxaml::xaml::Controls::AutoSuggestBox, TItems> 
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
    DOXY_RT(cppxaml::details::Wrapper<cppxaml::xaml::Controls::TextBlock>)
        TextBlock(std::wstring_view text) {
        cppxaml::details::Wrapper<cppxaml::xaml::Controls::TextBlock> tb;
        tb->Text(text);
        return tb;
    }

    /**
     * @fn template<typename C> cppxaml::details::Wrapper<cppxaml::xaml::Controls::Button> Button(C c)
     * @brief Creates a Button
     * @tparam C The type of the content to initialize the button with.
     * @param c The content for the button, e.g. a string, a `TextBlock`, or a panel with some children, etc.
     * @return
    */
    template<typename C>
    auto Button(C c) {
        return MakeContentControl<cppxaml::xaml::Controls::Button>(c);
    }

    /**
     * @brief Creates a FontIcon from a string
     * @param icon
     * @return cppxaml::details::Wrapper<cppxaml::xaml::Controls::FontIcon>
    */
    auto FontIcon(std::wstring_view icon) {
        cppxaml::details::Wrapper<cppxaml::xaml::Controls::FontIcon> fi;
        fi->Glyph(icon);
        return fi;
    }

    /**
     * @brief Creates a FontIcon from a glyph codepoint
     * @param glyph
     * @return cppxaml::details::Wrapper<cppxaml::xaml::Controls::FontIcon>
    */
    auto FontIcon(uint32_t glyph) {
        wchar_t icon[3]{};
        icon[0] = glyph & 0xffff;
        icon[1] = (glyph >> 16) & 0xffff;
        return FontIcon(icon);
    }


    /**
     * @brief Creates a MenuFlyoutItem with the specified text
     * @param text 
     * @return cppxaml::details::Wrapper<cppxaml::xaml::Controls::MenuFlyoutItem>
    */
    auto MenuFlyoutItem(std::wstring_view text) {
        cppxaml::details::Wrapper<cppxaml::xaml::Controls::MenuFlyoutItem> mfi;
        mfi->Text(text);
        return mfi;
    }

    template<typename T, typename TArg>
    void AddItems(T t, TArg&& first) {
        t.Items().Append(first);
    }

    template<typename T, typename TArg, typename... TArgs>
    void AddItems(T t, TArg&& first, TArgs&&... rest) {
        t.Items().Append(first);
        if constexpr (sizeof...(TArgs) > 0) {
            AddItems(t, rest...);
        }
    }

    /**
     * @brief Creates a `MenuFlyout` from a list of `MenuFlyoutItems`
     * @param items 
     * @return cppxaml::details::Wrapper<cppxaml::xaml::Controls::MenuFlyout>
    */
    template<typename... TMenuFlyoutItemBase>
    auto MenuFlyout(TMenuFlyoutItemBase&&... items) {
        cppxaml::details::Wrapper<cppxaml::xaml::Controls::MenuFlyout> mf;
        AddItems(*mf, items...);
        return mf;
    }

}