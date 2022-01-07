#pragma once
#include <winrt/Windows.Foundation.h>
#include <cppxaml/utils.h>

/** @file
* @author Alexander Sklar
* @section LICENSE
 * Copyright (c) Alexander Sklar
 *
 * Licensed under the MIT license
*/

namespace cppxaml {
    namespace details {

        cppxaml::xaml::FrameworkElement FindChildWithVSG(cppxaml::xaml::FrameworkElement fe) {
            cppxaml::xaml::FrameworkElement root{ nullptr };
            for (auto i = 0; i < cppxaml::xaml::Media::VisualTreeHelper::GetChildrenCount(fe); i++) {
                auto child = cppxaml::xaml::Media::VisualTreeHelper::GetChild(fe, i);
                if (auto childFE = child.try_as<cppxaml::xaml::FrameworkElement>()) {
                    winrt::Windows::Foundation::Collections::IVector<cppxaml::xaml::VisualStateGroup> vsgs = cppxaml::xaml::VisualStateManager::GetVisualStateGroups(childFE);
                    if (vsgs.Size() != 0) {
                        root = childFE;
                        break;
                    }
                }
            }
            return root;
        }

        // TODO: make the callback take a template parameter so we don't have to cast from IInspectable
        struct VSMListener : winrt::implements<VSMListener, winrt::Windows::Foundation::IInspectable> {

            VSMListener(cppxaml::xaml::FrameworkElement fe, const std::unordered_map<std::wstring, cppxaml::xaml::VisualStateChangedEventHandler>& map) : m_map(map) {
                if (auto root = FindChildWithVSG(fe)) {
                    for (const cppxaml::xaml::VisualStateGroup& vsg : cppxaml::xaml::VisualStateManager::GetVisualStateGroups(root)) {
                        vsg.CurrentStateChanged([_this = this->get_strong(), fe](winrt::Windows::Foundation::IInspectable sender, cppxaml::xaml::VisualStateChangedEventArgs args) {

                            auto newState = args.NewState();
                            auto newName = newState.Name();
                            const auto newNameStr = newName.c_str();
                            if (_this->m_map.contains(newNameStr)) {
                                _this->m_map[newNameStr](fe, args);
                            }
                        });
                    }
                }

            }

        private:
            std::unordered_map<std::wstring, cppxaml::xaml::VisualStateChangedEventHandler> m_map{};
        };
    }

    template<typename TFrameworkElement>
    auto VSMListener(TFrameworkElement obj, const std::unordered_map<std::wstring, cppxaml::xaml::VisualStateChangedEventHandler>& map) {
        cppxaml::xaml::FrameworkElement fe(obj);
        if (!fe.Parent()) {
            fe.Loaded([map](winrt::Windows::Foundation::IInspectable sender, auto&) {
                auto listener = winrt::make_self<cppxaml::details::VSMListener>(sender.as<cppxaml::xaml::FrameworkElement>(), map);
                });
        }
        else {
            auto listener = winrt::make_self<cppxaml::details::VSMListener>(fe, map);
        }
        return obj;
    }

}
