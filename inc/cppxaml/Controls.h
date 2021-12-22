#pragma once
#include <winrt/windows.ui.xaml.h>
#include <winrt/windows.ui.xaml.media.h>
#include <winrt/windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <type_traits>
#include <ShObjIdl_core.h>

#ifdef USE_WINUI3
#include <microsoft.ui.xaml.window.h>
#endif

namespace cppxaml {

#ifdef USE_WINUI3
    namespace xaml = winrt::Microsoft::UI::Xaml;
#else
    namespace xaml = winrt::Windows::UI::Xaml;
#endif

    namespace utils {
        auto tolower(std::wstring_view sv) {
            std::wstring copy(sv);
            std::transform(copy.begin(), copy.end(), copy.begin(), [](wchar_t x) { return (wchar_t)::tolower(x); });
            return copy;
        }
    }

    template<typename T = cppxaml::xaml::DependencyObject>
    T FindChildByName(cppxaml::xaml::DependencyObject d, std::wstring_view name) {
        if (auto fe = d.try_as<cppxaml::xaml::FrameworkElement>()) {
            if (fe.Name() == name) return d.as<T>();
        }
        for (int i = 0; i < cppxaml::xaml::Media::VisualTreeHelper::GetChildrenCount(d); i++) {
            auto r = FindChildByName<T>(cppxaml::xaml::Media::VisualTreeHelper::GetChild(d, i), name);
            if (r) return r.as<T>();
        }
        return nullptr;
    }

    namespace details {
        template<typename T>
        struct WrapperT {
            T m_value{ nullptr };

            WrapperT(const T& v = T()) : m_value(v) {}

            T operator*() const {
                return m_value;
            }
            const T* operator->() const { return &m_value; }
            auto Name() const { return m_value.Name(); }
            auto Name(std::wstring_view n) const { m_value.Name(n); return *this; }

            auto Margin() const { return m_value.Margin(); }
            auto Margin(cppxaml::xaml::Thickness t) const { m_value.Margin(t); return *this; }
            auto Margin(double m) const { m_value.Margin(cppxaml::xaml::ThicknessHelper::FromUniformLength(m)); return *this; }
            auto Margin(double l, double t, double r, double b) const {
                m_value.Margin(cppxaml::xaml::ThicknessHelper::FromLengths(l, t, r, b)); return *this;
            }

            auto Padding() const { return m_value.Padding(); }
            auto Padding(cppxaml::xaml::Thickness t) const { m_value.Padding(t); return *this; }
            auto Padding(double m) const { m_value.Padding(cppxaml::xaml::ThicknessHelper::FromUniformLength(m)); return *this; }
            auto Padding(double l, double t, double r, double b) const {
                m_value.Padding(cppxaml::xaml::ThicknessHelper::FromLengths(l, t, r, b)); return *this;
            }

            template<typename D, std::enable_if_t<std::is_assignable_v<D, T>, bool> = true>
            operator D() const {
                return m_value;
            }

        };

        template<typename T, typename TItems = void>
        struct Wrapper : WrapperT<T> {};

        template<>
        struct Wrapper<cppxaml::xaml::Controls::ContentDialog> : WrapperT<cppxaml::xaml::Controls::ContentDialog> {
            auto PrimaryButtonText(std::wstring_view t) {
                this->m_value.as<cppxaml::xaml::Controls::ContentDialog>().PrimaryButtonText(t);
                return *this;
            }
        };

        template<>
        struct Wrapper<cppxaml::xaml::Controls::StackPanel> : WrapperT<cppxaml::xaml::Controls::StackPanel> {
            auto Orientation(cppxaml::xaml::Controls::Orientation o) {
                this->m_value.as<cppxaml::xaml::Controls::StackPanel>().Orientation(o);
                return *this;
            }
        };


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
            auto EnableDefaultSearch(bool caseInsensitive = true) {
                SetEventHandlers(caseInsensitive);
                return *this;
            }
        };

        template<typename TItems>
        struct Wrapper<cppxaml::xaml::Controls::ItemsControl, TItems> : WrapperT<cppxaml::xaml::Controls::ItemsControl> {
            using items_t = TItems;

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

    template<typename T, typename TItems = void>
    std::enable_if_t<std::is_assignable_v<cppxaml::xaml::Controls::ContentControl, T>, cppxaml::details::Wrapper<T, TItems>>
        MakeContentControl(winrt::Windows::Foundation::IInspectable i) {
        cppxaml::details::Wrapper<T> t;
        t->Content(i);
        return t;
    }

    template<typename T>
    std::enable_if_t<std::is_assignable_v<cppxaml::xaml::Controls::ContentControl, T>, cppxaml::details::Wrapper<T>>
        MakeContentControl(winrt::hstring v) {
        cppxaml::details::Wrapper<T> t;
        t->Content(winrt::box_value(v));
        return t;
    }

    template<typename T>
    std::enable_if_t<std::is_assignable_v<cppxaml::xaml::Controls::Panel, T>, cppxaml::details::Wrapper<T>>
        MakePanel(const std::initializer_list<cppxaml::xaml::UIElement>& elems) {
        cppxaml::details::Wrapper<T> panel;
        for (auto e : elems) {
            panel->Children().Append(e);
        }
        return panel;
    }

    template<typename C>
    auto ContentDialog(C i) {
        return MakeContentControl<cppxaml::xaml::Controls::ContentDialog>(i);
    }


    auto StackPanel(const std::initializer_list<cppxaml::xaml::UIElement>& elems) {
        return MakePanel<cppxaml::xaml::Controls::StackPanel>(elems);
    }

    auto Grid(details::GridRows gr, cppxaml::details::GridColumns gc, const std::initializer_list<details::UIElementInGrid>& elems) {
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

    auto TextBox(std::wstring_view text = L"") {
        cppxaml::details::Wrapper<cppxaml::xaml::Controls::TextBox> tb;
        tb->Text(text);
        return tb;
    }


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

    template<typename TItems>
    auto AutoSuggestBox(const TItems& /*std::initializer_list<std::wstring_view>*/ svs) {
        return MakeItemsControl<cppxaml::xaml::Controls::AutoSuggestBox, TItems>(svs);
    }

    auto TextBlock(std::wstring_view text) {
        cppxaml::xaml::Controls::TextBlock tb;
        tb.Text(text);
        return tb;
    }

    template<typename C>
    auto Button(C c) {
        return MakeContentControl<cppxaml::xaml::Controls::Button>(c);
    }

    template<typename TWindow>
    std::enable_if_t<!std::is_assignable_v<TWindow, cppxaml::XamlWindow*>> InitializeWithWindow(winrt::Windows::Foundation::IInspectable obj, TWindow /*winrt::Microsoft::UI::Xaml::Window*/ w) {
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

    bool InitializeWithWindow(cppxaml::xaml::FrameworkElement obj, HWND hwnd) {
        if (auto iww = obj.try_as<IInitializeWithWindow>()) {
            iww->Initialize(hwnd);
            return true;
        }
        return false;
    }

    void InitializeWithWindow(cppxaml::xaml::FrameworkElement obj, const cppxaml::XamlWindow* xw) {
        if (!InitializeWithWindow(obj, xw->hwnd())) {
            obj.XamlRoot(xw->GetUIElement<cppxaml::xaml::FrameworkElement>().XamlRoot());
        }
    }

    template<typename TUIElement, typename TWindow>
    std::enable_if_t<std::is_assignable_v<cppxaml::xaml::UIElement, TUIElement> &&
        !std::is_same_v<TWindow, const cppxaml::XamlWindow*>, void>
        InitializeWithWindow(cppxaml::details::Wrapper<TUIElement> obj, TWindow /*winrt::Microsoft::UI::Xaml::Window*/ w) {
        return InitializeWithWindow(*obj, w);
    }
}