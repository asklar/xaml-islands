#pragma once

/** @file
* @author Alexander Sklar
* @section LICENSE
 * Copyright (c) Alexander Sklar
 *
 * Licensed under the MIT license
*/

/**
 * @brief 
 * @namespace cppxaml
*/
namespace cppxaml {

#ifdef USE_WINUI3
    namespace xaml = winrt::Microsoft::UI::Xaml;
#else
    namespace xaml = winrt::Windows::UI::Xaml;
#endif


    /**
     * @namespace cppxaml::utils
     * @brief Various utilities
    */
    namespace utils {
        /**
         * @brief
         * @param sv
         * @return
        */
        inline auto tolower(std::wstring_view sv) {
            std::wstring copy(sv);
            std::transform(copy.begin(), copy.end(), copy.begin(), [](wchar_t x) { return (wchar_t)::tolower(x); });
            return copy;
        }

        /**
         * @brief Maps each element in a container via a unary operation on each element
         * @tparam TOutContainer The output container type. Defaults to std::vector.
         * @tparam TInContainer The type of the container
         * @tparam UnaryOp Lambda type
         * @param iterable The container
         * @param unary_op The lambda for the unary operation; takes the element type of the input container.
         * @return A container comprised of the mapped elements
         * @details
         * Example:\n
         *
         *
        */
        template<template<typename...> typename TOutContainer = std::vector, typename TInContainer, typename UnaryOp>
        auto transform(TInContainer const& iterable, UnaryOp&& unary_op) {
            using result_t = std::invoke_result_t<UnaryOp, typename TInContainer::value_type>;
            TOutContainer<result_t> out{};
            auto o = std::inserter(out, out.end());
            for (auto i = iterable.cbegin(); i != iterable.cend(); i++) {
                o = unary_op(*i);
            }
            return out;
        }

        /**
         * @brief Maps each element in a container via a unary operation on each element
         * @tparam TOutContainer The output container type. Defaults to std::vector.
         * @tparam TInContainer The type of the container
         * @tparam UnaryOp Lambda type
         * @param iterable The container
         * @param unary_op The lambda for the unary operation; takes the element type of the input container, and the index within the container.
         * @return A container comprised of the mapped elements
         * @details Example:\n
         * Here we can define a Grid with Buttons, each of whose content is sourced from a string vector:
         * @code
         * auto strs = std::vector<std::wstring>{ L"first", L"second", L"third", L"fourth" };
         * auto grid = cppxaml::Grid({"40, *"}, {"Auto, Auto"},
         *      cppxaml::utils::transform_with_index(strs, [](const std::wstring& t, auto index) {
         *                 return cppxaml::details::UIElementInGrid{
         *                     (int)index / 2,
         *                     (int)index % 2,
         *                     cppxaml::Button(winrt::hstring(t))
         *                 };
         *             })
         *           );
         * @endcode
         * Example:\n
         * @code
         * auto strs = std::vector<std::wstring>{ L"first", L"second", L"third", L"fourth" };
         * auto grid = cppxaml::Grid({"40, *"}, {"Auto, Auto"},
         *      cppxaml::utils::transform_with_index(strs, [](const std::wstring& t, auto index) {
         *                 return cppxaml::Button(winrt::hstring(t))
         *                          .Set(Grid::RowProperty(), (int)index / 2)
         *                          .Set(Grid::ColumnProperty(), (int)index % 2);
         *                 };
         *             })
         *           );
         * @endcode
        */
        template<template<typename...> typename TOutContainer = std::vector, typename TInContainer, typename UnaryOp>
        auto transform_with_index(TInContainer const& iterable, UnaryOp&& unary_op) {
            using result_t = std::invoke_result_t<UnaryOp, typename TInContainer::value_type, typename TInContainer::const_iterator::difference_type>;
            TOutContainer<result_t> out{};
            auto o = std::inserter(out, out.end());
            for (auto i = iterable.cbegin(); i != iterable.cend(); i++) {
                o = unary_op(*i, i - iterable.cbegin());
            }
            return out;
        }

    }

    /**
     * @brief Finds a XAML element by name.
     * @tparam T Expected type of the element - defaults to DependencyObject.
     * @param d XAML element object.
     * @param name The name to search for.
     * @return The XAML element whose name matches the one specified as input.
    */
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


}

#ifndef DOXY
#define IF_ASSIGNABLE_CONTROL(XAMLTYPE)     std::enable_if_t<std::is_assignable_v<cppxaml::xaml::Controls::XAMLTYPE, T>, cppxaml::details::Wrapper<T>>
#define IF_ASSIGNABLE_CONTROL_TITEMS(XAMLTYPE, TITEMS)     std::enable_if_t<std::is_assignable_v<cppxaml::xaml::Controls::XAMLTYPE, T>, cppxaml::details::Wrapper<T, TITEMS>>
#define DOXY_RT(...) auto
#else
#define IF_ASSIGNABLE_CONTROL(XAMLTYPE)     cppxaml::details::Wrapper<T>
#define IF_ASSIGNABLE_CONTROL_TITEMS(XAMLTYPE, TITEMS)     cppxaml::details::Wrapper<T, TITEMS>
#define DOXY_RT(...) __VA_ARGS__
#endif
