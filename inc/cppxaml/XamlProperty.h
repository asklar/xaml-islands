#pragma once
#include <type_traits>

namespace winrt {
    template<typename T, typename I>
    T try_as(I* i) {
        T t;
        if (i->QueryInterface(winrt::guid_of<T>(), winrt::put_abi(t)) == S_OK) {
            return t;
        }
        return nullptr;
    }
}

namespace cppxaml {

    template<typename T>
    struct XamlEvent_t {
        winrt::event_token operator()(T const& handler) {
            return m_handler.add(handler);
        }
        auto operator()(const winrt::event_token& token) noexcept { return m_handler.remove(token); }

        template<typename... TArgs>
        auto invoke(TArgs&&... args) {
            return m_handler(args...);
        }
    private:
        winrt::event<T> m_handler;
    };

    template<typename T>
    using XamlEvent = XamlEvent_t<winrt::Windows::Foundation::EventHandler<T>>;

    template<typename TSender, typename TArgs>
    using TypedXamlEvent = XamlEvent_t<winrt::Windows::Foundation::TypedEventHandler<TSender, TArgs>>;

    template<typename T>
    struct SimpleNotifyPropertyChanged {
    public:
        using Type = T;
        auto PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value) {
            return m_propertyChanged.add(value);
        }
        void PropertyChanged(winrt::event_token const& token) {
            m_propertyChanged.remove(token);
        }
    protected:
        winrt::event<winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

        template<typename TProperty>
        friend struct XamlProperty;
    };


    template<typename T>
    struct XamlProperty {
        using Type = T;
        T operator()() const {
            return m_value;
        }
        void operator()(const T& value) {
            m_value = value;
        }

        operator T() {
            return m_value;
        }

        XamlProperty<T>& operator=(const T& t) {
            operator()(t);
            return *this;
        }

        XamlProperty(const XamlProperty&) = delete;
        XamlProperty(XamlProperty&&) = delete;

        template<typename... TArgs>
        XamlProperty(TArgs&&... args) : m_value(std::forward<TArgs>(args)...) {}

    private:
        Type m_value{};
    };

    template<typename T>
    struct XamlPropertyWithNPC : XamlProperty<T> {
        using Type = T;

        T operator()() const {
            return XamlProperty<T>::operator()();
        }

        void operator()(const T& value) {
            if (value != operator()()) {
                XamlProperty<T>::operator()(value);
                if (m_npc) {
                    (*m_npc)(m_sender, winrt::Windows::UI::Xaml::Data::PropertyChangedEventArgs{ m_name });
                }
            }
        }
        template<typename... TArgs>
        XamlPropertyWithNPC(
            winrt::event<winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler>* npc,
            winrt::Windows::Foundation::IInspectable sender,
            std::wstring_view name,
            TArgs&&... args) :
            XamlProperty(std::forward<TArgs...>(args)...) {
            m_name = name;
            m_npc = npc;
            m_sender = sender;
        }

        XamlPropertyWithNPC(const XamlPropertyWithNPC&) = default;
        XamlPropertyWithNPC(XamlPropertyWithNPC&&) = default;
    private:
        std::wstring m_name;
        winrt::event<winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler>* m_npc;
        winrt::Windows::Foundation::IInspectable m_sender;
    };
}

#define INIT_PROPERTY(NAME, VALUE)  \
    NAME(&m_propertyChanged, winrt::try_as<winrt::Windows::Foundation::IInspectable>(this), std::wstring_view{ L#NAME }, VALUE)

