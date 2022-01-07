  # CppXAML

[CppXAML](https://github.com/asklar/xaml-islands) aims to make usage of XAML and XAML islands in C++ more natural and idiomatic.

[C++/WinRT](https://docs.microsoft.com/windows/uwp/cpp-and-winrt-apis/) provides a projection of a Windows Runtime component's API, but one that isn’t always easy to use (esp. for XAML). It also is unopinionated about how to implement properties. This added flexibility can be useful, but is often unnecessary and results in overly-verbose code.

CppXAML provides several kinds of higher-level helpers. Some usage information can be found below; for more details, see the [API reference](https://asklar.github.io/xaml-islands).

GitHub repo: https://github.com/asklar/xaml-islands

# Table of Contents
* [Facilities for writing XAML controls](#facilities-for-writing-xaml-controls)
* [Facilities for using XAML controls](#facilities-for-using-xaml-controls)
* [Facilities for using XAML islands](#facilities-for-using-xaml-islands)

# Facilities for writing XAML controls      {#facilities-for-writing-xaml-controls}

## Property and event helpers

- [`XamlProperty<T>`](./structcppxaml_1_1_xaml_property.html)
- [`SimpleNotifyPropertyChanged<F>`](./structcppxaml_1_1_simple_notify_property_changed.html)
- [`XamlPropertyWithNPC<T>`](./structcppxaml_1_1_xaml_property_with_n_p_c.html)
- [`XamlEvent<T>`](./structcppxaml_1_1_xaml_event.html)
- [`TypedXamlEvent<TSender, TArgs>`](./structcppxaml_1_1_typed_xaml_event.html)

These provide stock/simple property objects that remove the need for verbose hand-written options. 

#### Example

Suppose you have the following Page defined in IDL:
```csharp
namespace Foo {
  runtimeclass MainPage : Windows.UI.Xaml.Controls.Page, Windows.UI.Xaml.Data.INotifyPropertyChanged {
    MainPage();
    event Windows.Foundation.EventHandler<String> OkClicked;
    String InterfaceStr;
    Int32 MyInt;
  }
}
```

**Before (plain C++/WinRT)**
```cpp
struct MainPage : MainPageT<MainPage> { 
  winrt::event<winrt::Windows::Foundation::EventHandler<winrt::hstring>> m_okClicked{};
  winrt::event_token OkClicked(winrt::Windows::Foundation::EventHandler<winrt::hstring> h) {
    return m_okClicked.add(h);
  }
  void OkClicked(winrt::event_token token) {
    m_okClicked.remove(token);
  }

  winrt::hstring m_InterfaceStr;
  winrt::hstring InterfaceStr() {
    return m_InterfaceStr;
  }
  void InterfaceStr(winrt::hstring v) {
    m_InterfaceStr = v;
  }

  winrt::event<winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

  winrt::event_token PropertyChanged(winrt::Windows::Xaml::Data::PropertyChangedEventHandler const& value) {
    return m_propertyChanged.add(value);
  }
  void PropertyChanged(winrt::event_token const& token) {
    m_propertyChanged.remove(token);
  }

  int32_t m_MyInt;
  int32_t MyInt() {
    return m_MyInt;
  }
  void MyInt(int32_t v) { 
    m_MyInt = v; 
    m_propertyChanged(*this, { L"MyInt" });
  }

};

```

**After (with CppXaml)**
```cpp

struct MainPage : MainPageT<MainPage>, cppxaml::SimpleNotifyPropertyChanged<MainPage> {
  cppxaml::XamlEvent<winrt::hstring> OkClicked;
  cppxaml::XamlProperty<winrt::hstring> InterfaceStr;
  cppxaml::XamlPropertyWithNPC<int32_t> MyInt;
  MainPage() : INIT_PROPERTY(MyInt, 42) {
    InitializeComponent();

    // Properties can be assigned to and read from with the operator= too!
    InterfaceStr = winrt::hstring{ L"This string comes from the implementation" };
    winrt::hstring v = InterfaceStr;
  }
};
```

# Facilities for using XAML controls        {#facilities-for-using-xaml-controls}

## Control helpers
CppXAML includes some primitives to make it more natural to write XAML UI in code.


### xaml Namespace alias
Since we want CppXAML to be future proof and work with WinUI 3, CppXAML creates a namespace alias `cppxaml::xaml` which points at either `Windows::UI::Xaml` or `Microsoft::UI::Xaml` for system XAML or WinUI 3, respectively.

### Builder-style programming
C++/WinRT enables setting properties on a type by calling a property setter method, e.g. `myTextBlock.Text(L"text");`. If you then want to set another property, then you have to make another call `myTextBlock.XYZ(...);`. This can get verbose when having to set multiple properties. CppXAML enables writing builder-style code instead of the former imperative-style:

**Before (plain C++/WinRT)**
```cpp
auto myTextBlock = winrt::Windows::UI::Xaml::Controls::TextBlock{};
myTextBlock.Text(L"Hello world");
myTextBlock.Margin(winrt::Windows::UI::Xaml::ThicknessHelper::FromUniformLength(4));
myTextBlock.Padding(winrt::Windows::UI::Xaml::ThicknessHelper::FromUniformLength(6));
myTextBlock.Name(L"myTB");
```

**After (with CppXAML)**
```cpp
auto myTextBlock = cppxaml::TextBlock(L"Hello world")
                    .Margin(4)
                    .Padding(6)
                    .Name(L"myTB");
```

### TextBlock
Most commonly, you will want to construct a `TextBlock` from a string of text. This can be a bit cumbersome if you are using C++/WinRT directly. 
With cppxaml, as you've seen above, you can just write:
```cpp
auto tb = cppxaml::TextBlock(L"Hello");
```

### StackPanel
Panels in XAML can have zero or more children. CppXAML makes it easy to naturally describe a panel's children with an initializer list. For example:
```cpp
auto sp = cppxaml::StackPanel({
  cppxaml::TextBlock(L"Hello"),
  cppxaml::TextBlock(L"world!")
  });
```


You can also set its `Orientation`:
```cpp
auto sp cppxaml::StackPanel({
    cppxaml::TextBlock(L"Hello"),
    cppxaml::TextBlock(L"world!").Name(L"worldTB")
  }).Orientation(cppxaml::xaml::Controls::Orientation::Horizontal);
```

### ContentControl (Button, etc.)
Sub-classes of XAML's `ContentControl` enable nesting one control into another via the `Content` property. CppXAML makes this easier:
```cpp
auto scrollViewer = cppxaml::MakeContentControl<cppxaml::xaml::Controls::ScrollViewer>({
  cppxaml::StackPanel({
    cppxaml::TextBlock(L"Hello"),
    cppxaml::TextBlock(L"world!").Name(L"worldTB")
  })
});
```

### Locating elements by name
Sometimes you will need to perform some operation on an element that is deeply nested in a builder-style declaration. You can use `FindChildByName` to find an element in a XAML tree with a given name:
```cpp
auto worldTB = cppxaml::FindChildByName<Controls::TextBlock>(*scrollViewer, L"worldTB");
```

### Grid
Declaring a `Grid` in XAML via code is cumbersome as one has to create and set its `RowDefinitions` and `ColumnDefinitions`. 
CppXAML makes this a lot more straightforward:
```cpp
auto grid = cppxaml::Grid({"40, *"}, {"Auto, Auto"}, {
                {0, 0, cppxaml::TextBlock(L"first") },
                {0, 1, cppxaml::TextBlock(L"second") },
                {1, 0, cppxaml::TextBlock(L"third") },
                {1, 1, cppxaml::TextBlock(L"fourth") },
                }),
```

Here we defined a 2x2 `Grid`. The rows have heights of 40 px and `*`, and the columns are `Auto`. Then each child of the Grid is added in the cell designated by each entry in the initializer list, read as "row, column, child".

In addition to the string syntax (which requires some parsing/tokenizing), this also works:
```cpp
auto grid = cppxaml::Grid({40, {"*"}}, {{"Auto"}, {"Auto"}}, {
                {0, 0, cppxaml::TextBlock(L"first") },
                {0, 1, cppxaml::TextBlock(L"second") },
                {1, 0, cppxaml::TextBlock(L"third") },
                {1, 1, cppxaml::TextBlock(L"fourth") },
                }),
```

### Attached properties
You can set arbitrary dependency properties, including attached properties, on a cppxaml wrapper:

```cpp
auto tb = cppxaml::TextBlock(L"something")
                    .Set(Grid::RowProperty(), 1)
                    .Set(Grid::ColumnSpanProperty(), 2);
```

### AutoSuggestBox
You can easily create an `AutoSuggestBox` from a `std::vector<std::wstring>`; make sure the vector's lifetime extends for at least as long as the XAML UI is up.

`EnableDefaultSearch()` will provide a reasonable default search experience (filters the list of items down to those that contain the search string). This behavior is case insensitive by default, but can be made case sensitive by passing `false`.

```cpp
auto asb = cppxaml::AutoSuggestBox(GetFontFamilies())
                    .EnableDefaultSearch()
                    .Margin(0, 16, 0, 4)
                    .Name(L"fontTB");
```

### Visual State change notifications
You can easily set up visual state change notifications:

```cpp
    auto button = cppxaml::Button(L"click me")
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
            { L"Pressed", [](auto& sender, auto&) {
                auto button = sender.as<Controls::Button>();
                button.Content(winrt::box_value(L"pressed"));
            } },
        });
```

### Initializing Panels from collections
As we saw earlier, you can initialize a panel from its children. 
You can also use transforms, to map elements from a data model into its corresponding view.
For more details see `cppxaml::transform` and `cppxaml::transform_with_index` in [utils.h](./namespacecppxaml_1_1utils.html).

### Menus and icons
You can easily compose MenuFlyoutItems into a MenuFlyout, and also have a centralized menu handler callback:
```cpp
    auto menuFlyout = cppxaml::MenuFlyout(
                        cppxaml::MenuFlyoutItem(L"Exit")
                            .IconElement(cppxaml::FontIcon(0xe8bb))
                            .Click([hwnd = xw->hwnd()](auto&...) {
                                DestroyWindow(hwnd);
                            }),
                        cppxaml::MenuFlyoutItem(L"Cancel")
                            .IconElement(cppxaml::FontIcon(L"\xE8A7"))
                        )
                    .CentralizedHandler([](Windows::Foundation::IInspectable sender, auto&) {
                        auto mfi = sender.as<Controls::MenuFlyoutItem>();
                        auto x = mfi.Text();
                        });
```


# Facilities for using XAML Islands         {#facilities-for-using-xaml-islands}

## XamlWindow

`XamlWindow` implements an HWND based host for XAML Islands. You can create a `XamlWindow` from one of three overloads of Make:

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

### Parenting flyouts
To parent a flyout or context menu, you may use one of the `InitializeWithWindow` methods:

- Initialize with a WinUI 3 `Window`-like object:
  ```cpp
  template<typename TWindow>
  std::enable_if_t<!std::is_assignable_v<TWindow, cppxaml::XamlWindow*>> InitializeWithWindow(winrt::Windows::Foundation::IInspectable obj, TWindow /*winrt::Microsoft::UI::Xaml::Window*/ w)
  ```
- Initialize with an `HWND`:
  ```cpp
  bool InitializeWithWindow(cppxaml::xaml::FrameworkElement obj, HWND hwnd)
  ```
- Initialize with a `XamlWindow`:
  ```cpp
  void InitializeWithWindow(cppxaml::xaml::FrameworkElement obj, const cppxaml::XamlWindow* xw)
  ```

## AppController
`AppController` is responsible for coordinating XamlWindow instances, can extend their wndproc, and provides an opportunity to hook up event handlers once a XAML UI becomes live


