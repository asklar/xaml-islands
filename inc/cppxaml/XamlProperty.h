#pragma once
#include <type_traits>
#include <cppxaml/utils.h>

/** @file
* @author Alexander Sklar
* @section LICENSE
 * Copyright (c) Alexander Sklar
 *
 * Licensed under the MIT license
*/

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

/**
 * @namespace
*/
namespace cppxaml {

    namespace details {
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
    }
    /**
     * @brief A default event handler that maps to [Windows.Foundation.EventHandler](https://docs.microsoft.com/uwp/api/windows.foundation.eventhandler-1).
     * @tparam T The event data type.
    */
    template<typename T>
    struct XamlEvent : cppxaml::details::XamlEvent_t<winrt::Windows::Foundation::EventHandler<T>> {};

    /**
     * @brief A default event handler that maps to [Windows.Foundation.TypedEventHandler](https://docs.microsoft.com/uwp/api/windows.foundation.typedeventhandler-2).
     * @tparam T The event data type.
     * @details Usage example:
     * @code
     *         // In IDL, this corresponds to:
     *         //   event Windows.Foundation.TypedEventHandler<ModalPage, String> OkClicked;
     *         cppxaml::TypedXamlEvent<MarkupSample::ModalPage, winrt::hstring> OkClicked;
     * @endcode
    */
    template<typename TSender, typename TArgs>
    struct TypedXamlEvent : cppxaml::details::XamlEvent_t<winrt::Windows::Foundation::TypedEventHandler<TSender, TArgs>> {};

    /**
     * @brief Helper base class to inherit from to have a simple implementation of [INotifyPropertyChanged](https://docs.microsoft.com/uwp/api/windows.ui.xaml.data.inotifypropertychanged).
     * @tparam T CRTP type
     * @details When you declare your class, make this class a base class and pass your class as a template parameter:
     * @code
     * struct MyPage : MyPageT<MyPage>, SimpleNotifyPropertyChanged<MyPage> { 
     *      cppxaml::XamlPropertyWithNPC<int> MyInt;
     * 
     *      MyPage() : INIT_PROPERTY(MyInt, 42) { }
     * };
     * @endcode
    */
    template<typename T>
    struct SimpleNotifyPropertyChanged {
    public:
        using Type = T;
        auto PropertyChanged(cppxaml::xaml::Data::PropertyChangedEventHandler const& value) {
            return m_propertyChanged.add(value);
        }
        void PropertyChanged(winrt::event_token const& token) {
            m_propertyChanged.remove(token);
        }
        Type& self() {
            return *static_cast<Type*>(this);
        }

        /**
         * @brief Raises a property change notification event
         * @param name The name of the property
         * @return 
         * @details Usage example\n
         * XAML file
         * @code{.xml}
         * <!-- XAML file... -->
         * <Image Source="ms-appx:///assets/skulogo2_client.png" Height="{x:Bind MyInt, Mode=OneWay}" />
         * @endcode
         * C++
         * @code
         * void MyPage::DoSomething() {
         *  // modify MyInt
         *  // MyInt = ...
         * 
         *  // now send a notification to update the bound UI elements
         *  RaisePropertyChanged(L"MyInt");
         * }
         * @endcode
        */
        auto RaisePropertyChanged(std::wstring_view name) {
            return m_propertyChanged(self(), cppxaml::xaml::Data::PropertyChangedEventHandler{name});
        }
    protected:
        winrt::event<cppxaml::xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

        template<typename TProperty>
        friend struct XamlProperty;
    };

    /**
     * @brief Implements a simple property (without change notifications).
     * @tparam T the property type.
    */
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

    /**
     * @brief Implements a property type with notifications
     * @tparam T the property type
     * @details Use the #INIT_PROPERTY macro to initialize this property in your class constructor. This will set up the right property name, and bind it to the `SimpleNotifyPropertyChanged` implementation.
    */
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
                    (*m_npc)(m_sender, cppxaml::xaml::Data::PropertyChangedEventArgs{ m_name });
                }
            }
        }
        template<typename... TArgs>
        XamlPropertyWithNPC(
            winrt::event<cppxaml::xaml::Data::PropertyChangedEventHandler>* npc,
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
        winrt::event<cppxaml::xaml::Data::PropertyChangedEventHandler>* m_npc;
        winrt::Windows::Foundation::IInspectable m_sender;
    };
}

/**
* @def INIT_PROPERTY
* @brief use this to initialize a cppxaml::XamlPropertyWithNPC in your class constructor.
*/
#define INIT_PROPERTY(NAME, VALUE)  \
    NAME(&m_propertyChanged, winrt::try_as<winrt::Windows::Foundation::IInspectable>(this), std::wstring_view{ L#NAME }, VALUE)

