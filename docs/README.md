# CppXAML

[CppXAML](https://github.com/asklar/xaml-islands) aims to make usage of XAML and XAML islands in C++ more natural and idiomatic.

[C++/WinRT](https://docs.microsoft.com/windows/uwp/cpp-and-winrt-apis/) provides a projection of a Windws Runtime component's API, but one that isn’t always easy to use (esp. for XAML). It also is unopinionated w.r.t. how to implement properties. This added flexibility can be good, but is often unnecessary and results in overly-verbose code.

CppXAML provides the following higher-level helpers:

## XamlWindow

XAMLWindow implements an HWND based host for XAML Islands. You can create a `XamlWindow` from one of three overloads of Make:

1. Host a build-time XAML `UIElement` (usually defined in a runtime component project, often will be a `Page`)
    API:
    ```cpp
    template<typename TUIElement>
    static XamlWindow& Make(PCWSTR id, AppController* controller = nullptr);
    ```
    Usage:
    ```cpp
    auto& mainWindow = cppxaml::XamlWindow::Make<MarkupSample::MainPage>(L"MarkupSample", &controller);
    ```
2. Host UI created from markup at runtime:
   API:
   ```cpp
   static XamlWindow& Make(PCWSTR id, std::wstring_view markup, AppController* c = nullptr)
   ```
   Usage:
   ```cpp
   auto& xw = cppxaml::XamlWindow::Make(L"MyPage", LR"(
     <StackPanel>
       <TextBlock>Hello</TextBlock>
     </StackPanel>)", &controller);
   ```
3. Host UI created programmatically at runtime:
   API:
   ```cpp
   static XamlWindow& Make(PCWSTR id, winrt::Windows::UI::Xaml::UIElement(*getUI)(const XamlWindow&), AppController* c = nullptr);
   ```
   Usage:
   ```cpp
   auto& xw = cppxaml::XamlWindow::Make(L"Foo", [](auto&...) { 
     return winrt::Windows::UI::Xaml::Controls::Button(); 
   });
   ```

## AppController
`AppController` is responsible for coordinating XamlWindow instances, can extend their wndproc, and provides an opportunity to hook up event handlers once a XAML UI becomes live

## Property and event helpers

- `XamlProperty<T>`
- `SimpleNotifyPropertyChanged<F>`
- `XamlPropertyWithNPC<T>`
- `XamlEvent<T>`

These provide stock/simple property objects that remove the need for verbose hand-written options. 

#### Examples:
**MainPage.h**:
```cpp
cppxaml::XamlEvent<winrt::hstring> OkClicked;
cppxaml::XamlProperty<winrt::hstring> InterfaceStr;
cppxaml::XamlProperty<winrt::hstring> InterfaceStrWithNPC;
```

**MainPage.cpp**:
```cpp
MainPage::MainPage() : INIT_PROPERTY(MyInt, 42)
{
    InitializeComponent();
    // Properties can be assigned to and read from with the operator= too!
    ImplString = winrt::hstring{ L"This string comes from the implementation" };
    winrt::hstring v = ImplString;
}
```

