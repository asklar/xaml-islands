# CppXAML

[CppXAML](https://github.com/asklar/xaml-islands) aims to make usage of XAML and XAML islands in C++ more natural and idiomatic.

[C++/WinRT](https://docs.microsoft.com/windows/uwp/cpp-and-winrt-apis/) provides a projection of a Windws Runtime component's API, but one that isn’t always easy to use (esp. for XAML). It also is unopinionated about how to implement properties. This added flexibility can be good, but is often unnecessary and results in overly-verbose code.

CppXAML provides several kinds of higher-level helpers. Usage information can be found below. See also [API reference](https://asklar.github.io/xaml-islands).

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

## Property and event helpers

- `XamlProperty<T>`
- `SimpleNotifyPropertyChanged<F>`
- `XamlPropertyWithNPC<T>`
- `XamlEvent<T>`
- `XamlTypedEvent<TSender, TArgs>`

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

## Control helpers
CppXAML includes some primitives to make it more natural to write XAML UI in code.


### TextBlock
Most commonly, you will want to construct a `TextBlock` from a string of text. This can be a bit cumbersome if you are using C++/WinRT directly. With cppxaml, you can just write:
```cpp
auto tb = cppxaml::TextBlock(L"Hello");
```

### `xaml` Namespace alias
Since we want CppXAML to be future proof and work with WinUI 3, CppXAML creates a namespace alias `cppxaml::xaml` which points at either `Windows::UI::Xaml` or `Microsoft::UI::Xaml` for system XAML or WinUI 3, respectively.

### Fluent-style programming style
C++/WinRT enables setting properties on a type by calling a property setter method, e.g. `myTextBlock.Text(L"text");`. If you then want to set another property, then you have to make another call `myTextBlock.XYZ(...);`. This can get verbose when having to set multiple properties. CppXAML enables writing fluent-style code instead of the former imperative-style:
```cpp
auto myTextBlock = cppxaml::TextBlock(L"Hello world")
                    .Margin(4)
                    .Padding(6)
                    .Name(L"myTB");
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
Sometimes you will need to perform some operation on an element that is deeply nested in a fluent-style declaration. You can use `FindChildByName` to find an element in a XAML tree with a given name:
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

### AutoSuggestBox
You can easily create an `AutoSuggestBox` from a `std::vector<std::wstring>`; make sure the vector's lifetime extends for at least as long as the XAML UI is up.

`EnableDefaultSearch()` will provide a reasonable default search experience (filters the list of items down to those that contain the search string). This behavior is case insensitive by default, but can be made case sensitive by passing `false`.

```cpp
auto asb = cppxaml::AutoSuggestBox(GetFontFamilies())
                    .EnableDefaultSearch()
                    .Margin(0, 16, 0, 4)
                    .Name(L"fontTB");
```