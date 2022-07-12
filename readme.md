There are a few ways to use UWP XAML in a Win32 app via XAML islands. These options are sometimes independent so there is a matrix of possible combinations:

# Basic scaffolding

**Setup:**

0. You have a win32 desktop app
1. Add [CppWinRT](https://www.nuget.org/packages/Microsoft.Windows.CppWinRT/) NuGet package.
2. Add [VCRT forwarders](https://www.nuget.org/packages/Microsoft.VCRTForwarders.140/) NuGet package. -- or use the [Hybrid CRT](https://github.com/microsoft/WindowsAppSDK/blob/main/docs/Coding-Guidelines/HybridCRT.md)
3. [optional] Add the [Microsoft.Toolkit.Win32.UI.XamlApplication](https://www.nuget.org/packages/Microsoft.Toolkit.Win32.UI.XamlApplication/) NuGet package -- or create your own application type; the following assumes you used this package

```cpp

#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>


#include <winrt/base.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Microsoft.Toolkit.Win32.UI.XamlHost.h>

// Needed if you have a Runtime Component to host markup
#include <winrt/AppMarkup.h>

using namespace winrt;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Hosting;

using namespace Microsoft::Toolkit::Win32::UI::XamlHost;

XamlApplication xapp{ nullptr };

// This DesktopWindowXamlSource is the object that enables a non-UWP desktop application 
// to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
DesktopWindowXamlSource desktopXamlSource{ nullptr };


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    winrt::init_apartment(apartment_type::single_threaded);

    // only needed if referencing WinUI
    auto winuiIXMP = winrt::Microsoft::UI::Xaml::XamlTypeInfo::XamlControlsXamlMetaDataProvider();
    // only needed if you have a Runtime Component project for compiling markup
    auto markupIXMP = winrt::AppMarkup::XamlMetaDataProvider();

    // remove the IXMPs that you don't need
    xapp = XamlApplication({winuiIXMP, markupIXMP});

    WindowsXamlManager winxamlmanager = WindowsXamlManager::InitializeForCurrentThread();
    // needed if using WinUI
    xapp.Resources().MergedDictionaries().Append(winrt::Microsoft::UI::Xaml::Controls::XamlControlsResources());

    desktopXamlSource = DesktopWindowXamlSource();

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

    MSG msg;


    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (auto xamlSourceNative2 = desktopXamlSource.as<IDesktopWindowXamlSourceNative2>()) {
          BOOL xamlSourceProcessedMessage = FALSE;
          winrt::check_hresult(xamlSourceNative2->PreTranslateMessage(&msg, &xamlSourceProcessedMessage));
          if (xamlSourceProcessedMessage) {
            continue;
          }
        }

        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  // Get handle to the core window.
  auto interop = desktopXamlSource.as<IDesktopWindowXamlSourceNative>();
  
  switch (message)
    {
    case WM_CREATE: {

      
      // Parent the DesktopWindowXamlSource object to the current window.
      check_hresult(interop->AttachToWindow(hWnd));

      auto createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);

      // Get the new child window's hwnd
      HWND hWndXamlIsland = nullptr;
      check_hresult(interop->get_WindowHandle(&hWndXamlIsland));
      SetWindowPos(hWndXamlIsland, nullptr, 0, 0, createStruct->cx, createStruct->cy, SWP_SHOWWINDOW);

#ifdef CREATE_UI_IN_CODE
      // Option 1: create UI in code:
      Controls::TextBlock tb;
      tb.Text(L"Hello world!");
      desktopXamlSource.Content(tb);
#elif defined(CREATE_UI_FROM_STRING)
      auto tb = Markup::XamlReader::Load(LR"(
        <TextBlock Text="Hello world!"/>
)").as<TextBlock>();
      desktopXamlSource.Content(tb);
#else
      // Option 3: use a Windows Runtime component to define the UI in markup, and load it here
      Frame f;
      desktopXamlSource.Content(f);
      f.Navigate(winrt::xaml_typename<AppMarkup::BlankPage>());
#endif

      break;
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
        }
        break;
    case WM_SIZE:
    {
      HWND hWndXamlIsland = nullptr;
      check_hresult(interop->get_WindowHandle(&hWndXamlIsland));

      SetWindowPos(hWndXamlIsland, nullptr, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_SHOWWINDOW);

      break;
    }
    case WM_DESTROY:
        xapp.Close();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


```

# Packaging

## As a packaged app 
The easiest way to get started is to have your win32 project reference a Windows Runtime Component project (where you've added your XAML pages).
Then have a Windows Application Packaging project referencing your win32 app. 

## As an unpackaged app

### Scenarios
1. App just needs to use system XAML and create UI programmatically. This is the easiest case.
Your app can create all its UI in code. Just set up the basic scaffolding, create the XAML objects in code and then set the `DesktopWindowXamlSource`'s content to the top level object.

2. App uses WinUI 2.x (or other component libraries)

Things you'll need to worry about:

- Your app must reference the VCRT Forwarders NuGet package
- If you are using WinUI 2 in an unpackaged app, make sure you are using the *prerelease* package, or see Using framework packages
- If you are using a recent version of WinUI 2.x (which includes WebView2), there is a bug for non-UWP apps that prevents it from working out of the box. The bug will be fixed in upcoming versions (fix is checked in but not released yet). For the time being, you need to:
  - Reference the [Microsoft.Web.WebView2](https://www.nuget.org/packages/Microsoft.Web.WebView2/) NuGet package
  - Set the property in your vcxproj so that the WebView2 package knows to use WinRT APIs, not the Win32 ones:
    ```xml
    <WebView2UseWinRT>true</WebView2UseWinRT>
    ```
    If you don't do this, you will get an error like:
    ```
    1>cppwinrt : error Type 'Microsoft.Web.WebView2.Core.CoreWebView2' could not be found
    1> method: CoreWebView2
    1> type: Microsoft.UI.Xaml.Controls.IWebView2
    ```

- You need an application manifest to mark your app as working on 19h1 since that was the first XAML islands release:
```xml
<?xml version="1.0" encoding="utf-8"?>

<assembly manifestVersion="1.0" xmlns="urn:schemas-microsoft-com:asm.v1" xmlns:asmv3="urn:schemas-microsoft-com:asm.v3">
  <compatibility xmlns="urn:schemas-microsoft-com:compatibility.v1"> 
    <application> 
      <!--This Id value indicates the application supports Windows 10 functionality -->
      <supportedOS Id="{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}" />
      <maxversiontested Id="10.0.18362.0"/>
    </application>
  </compatibility>

  <assemblyIdentity
    name="WindowsProject1"
    type="winb32"
    version="1.0.0.0" />

  <dependency>
    <dependentAssembly>
      <assemblyIdentity
          type="win32"
          name="Microsoft.Windows.Common-Controls"
          version="6.0.0.0"
          processorArchitecture="*"
          publicKeyToken="6595b64144ccf1df"
          language="*"
        />
    </dependentAssembly>
  </dependency>
  
  <asmv3:application>
    <asmv3:windowsSettings xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings">
      <dpiAwareness>PerMonitorV2</dpiAwareness>
    </asmv3:windowsSettings>
  </asmv3:application>
</assembly>
```

- Your app needs to know how to activate the different WinRT types (including WinUI controls) that it references. You either need custom build logic (seen below) or you can use the [Unpackaged](https://www.nuget.org/packages/Unpackaged/) NuGet package to do this for you.

```xml
  <Target Name="_UnpackagedWin32MapWinmdsToManifestFiles" DependsOnTargets="ResolveAssemblyReferences">
    <ItemGroup>
      <!-- For each non-system .winmd file in References, generate a .manifest in IntDir for it. -->
      <_UnpackagedWin32WinmdManifest Include="@(ReferencePath->'$(IntDir)\%(FileName).manifest')" Condition="'%(ReferencePath.IsSystemReference)' != 'true' and '%(ReferencePath.WinMDFile)' == 'true' and '%(ReferencePath.ReferenceSourceTarget)' == 'ResolveAssemblyReference' and '%(ReferencePath.Implementation)' != ''">
        <WinMDPath>%(ReferencePath.FullPath)</WinMDPath>
        <Implementation>%(ReferencePath.Implementation)</Implementation>
      </_UnpackagedWin32WinmdManifest>
      <!-- For each referenced project that _produces_ a winmd, generate a temporary item that maps to
           the winmd, and use that temporary item to generate a .manifest in IntDir for it.
           We don't set Implementation here because it's inherited from the _ResolvedNativeProjectReferencePaths. -->
      <_UnpackagedWin32WinmdProjectReference Condition="'%(_ResolvedNativeProjectReferencePaths.ProjectType)' != 'StaticLibrary'" Include="@(_ResolvedNativeProjectReferencePaths-&gt;WithMetadataValue('FileType','winmd')-&gt;'%(RootDir)%(Directory)%(TargetPath)')" />
      <_UnpackagedWin32WinmdManifest Include="@(_UnpackagedWin32WinmdProjectReference->'$(IntDir)\%(FileName).manifest')">
        <WinMDPath>%(Identity)</WinMDPath>
      </_UnpackagedWin32WinmdManifest>
    </ItemGroup>
  </Target>
  <Target Name="_UnpackagedWin32GenerateAdditionalWinmdManifests" Inputs="@(_UnpackagedWin32WinmdManifest.WinMDPath)" Outputs="@(_UnpackagedWin32WinmdManifest)" DependsOnTargets="_UnpackagedWin32MapWinmdsToManifestFiles">
    <Message Text="Generating manifest for %(_UnpackagedWin32WinmdManifest.WinMDPath)" Importance="High" />
    <!-- This target is batched and a new Exec is spawned for each entry in _UnpackagedWin32WinmdManifest. -->
    <Exec Command="mt.exe -winmd:%(_UnpackagedWin32WinmdManifest.WinMDPath) -dll:%(_UnpackagedWin32WinmdManifest.Implementation) -out:%(_UnpackagedWin32WinmdManifest.Identity)" />
    <ItemGroup>
      <!-- Emit the generated manifest into the Link inputs. -->
      <Manifest Include="@(_UnpackagedWin32WinmdManifest)" />
    </ItemGroup>
  </Target>
```

- Your app will need to include WinUI's `resources.pri`. 
If your app doesn't have its own set of resources (i.e. you don't have any .xaml markup files, nor any other resources), then your app can just rename Microsoft.UI.Xaml.pri to resources.pri and put this file next to your exe.
If you are using the Unpackaged NuGet package (see above), you can set the MSBuild property `<HasOwnPriFiles>false</HasOwnPriFiles>` to automate this copy.

If your app does include its own resources (e.g. you have a Runtime Component project that includes your markup), then either:
- the Windows Application Packaging project will take care of merging your app's PRI and WinUI's PRI, or
- you need to merge the PRIs

### Merging PRIs
To merge the PRIs:
1) Create a file `pri.resfiles` in your project, and list the set of PRI files your project depends on:
```
C:\Users\asklar\source\repos\xaml-islands\AppMarkup\debug\AppMarkup\AppMarkup.pri
C:\Users\asklar\source\repos\xaml-islands\win32\packages\Microsoft.Toolkit.Win32.UI.XamlApplication.6.1.3\runtimes\win10-x86\native\Microsoft.Toolkit.Win32.UI.XamlHost.pri
C:\Users\asklar\source\repos\xaml-islands\win32\packages\Microsoft.UI.Xaml.2.8.0-prerelease.210927001\runtimes\win10-x86\native\Microsoft.UI.Xaml.pri
```

2) Create a file `priconfig.xml` in your project that references `pri.resfiles`:
```xml
<?xml version="1.0" encoding="utf-8"?>
<resources targetOsVersion="10.0.0" majorVersion="1">
  <index root="\" startIndexAt="pri.resfiles">
    <default>
      <qualifier name="Language" value="en-US" />
      <qualifier name="Contrast" value="standard" />
      <qualifier name="Scale" value="200" />
      <qualifier name="HomeRegion" value="001" />
      <qualifier name="TargetSize" value="256" />
      <qualifier name="LayoutDirection" value="LTR" />
      <qualifier name="DXFeatureLevel" value="DX9" />
      <qualifier name="Configuration" value="" />
      <qualifier name="AlternateForm" value="" />
      <qualifier name="Platform" value="UAP" />
    </default>
    <indexer-config type="PRI" />
    <indexer-config type="RESFILES" qualifierDelimiter="." />
  </index>
</resources>
```

3. You can then create the merged `resources.pri` by running:
```
makepri new /pr . /cf .\priconfig.xml /of .\debug\resources.pri /o
```

### Using Framework packages

Apps that use WinUI stable releases don't actually ship the WinUI 2 bits in their package, instead they declare a dependency and Windows will download the right framework package - which is shared with other apps installed on the system.


However unpackaged apps need a way to discover these framework packages. This is possible via using the Dynamic Dependencies API to add the WinUI package to your package graph (new in Windows 11, and also available separate from the Windows SDK as part of the Windows App SDK). Your app installer may also need to do more work to register the dependency.

Alternatively, you can reference *prerelease* NuGet packages of WinUI, which means you'll carry the WinUI 2 bits in your app, but also means you don't have to worry about framework packages.

# Accessing Win32 APIs from the Runtime Component
Since the runtime component will be desktop-only, it is okay for it to call non-UWP APIs.

Add these props to the WRC project (thanks to @sylveon for this tip):
```xml
  <PropertyGroup>
    <_NoWinAPIFamilyApp>true</_NoWinAPIFamilyApp>
    <_VC_Target_Library_Platform>Desktop</_VC_Target_Library_Platform>
    <DesktopCompatible>true</DesktopCompatible>
  </PropertyGroup>
```

You will also need to make sure you `#include` the right headers, and equally importantly, you'll need to add the right .lib. You can do this in the VS UI, or in your WRC vcxproj:
```xml
  <ItemDefinitionGroup>
    <Link>
      <AdditionalDependencies>Advapi32.lib;shell32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
```





