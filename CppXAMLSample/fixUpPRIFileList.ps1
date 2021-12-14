[CmdletBinding()]
param (
    [ValidateSet('x64', 'x86')]
    [string]
    $Arch = [ArchTypeEnum]::x64,
    [ValidateSet('Debug', 'Release')]
    [string]
    $Config = [Config]::Release
)

enum ArchTypeEnum {
  x64
  x86
}

enum Config {
  Debug
  Release
}

$resources = @(
"..\$Arch\$Config\MarkupSample\MarkupSample.pri",
"..\packages\Microsoft.Toolkit.Win32.UI.XamlApplication.6.1.3\runtimes\win10-x64\native\Microsoft.Toolkit.Win32.UI.XamlHost.pri",
"..\packages\Microsoft.UI.Xaml.2.8.0-prerelease.210927001\runtimes\win10-x64\native\Microsoft.UI.Xaml.pri"
)
Write-Output $resources | Out-File $PWD\XamlIslandsSample\pri.resfiles
