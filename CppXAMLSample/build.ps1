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

Write-Host Building $Arch $Config -ForegroundColor Green

try {
  $msbuild = Get-Command msbuild.exe
} catch {}

if ($null -eq $msbuild) {
  Write-Host Must run this in a VS dev cmd
  return
}

& nuget restore $PWD
& msbuild $PWD\MarkupSample.sln /p:Platform=$Arch /p:Configuration=$Config /p:RestorePackagesConfig=true /restore  /bl
$resources = @(
"..\$Arch\$Config\MarkupSample\MarkupSample.pri",
"..\packages\Microsoft.Toolkit.Win32.UI.XamlApplication.6.1.3\runtimes\win10-x64\native\Microsoft.Toolkit.Win32.UI.XamlHost.pri",
"..\packages\Microsoft.UI.Xaml.2.8.0-prerelease.210927001\runtimes\win10-x64\native\Microsoft.UI.Xaml.pri"
)
Write-Output $resources | Out-File $PWD\XamlIslandsSample\pri.resfiles

makepri new /pr $PWD\app /cf $PWD\XamlIslandsSample\priconfig.xml /of $PWD\$Arch\$Config\resources.pri /o

$output = "$PWD\$Arch\$Config\UnpackagedWin32.exe"
if (Test-Path $output) {
  Write-Host -ForegroundColor Green Output at $output
} else {
  Write-Error "Errors found, $output not found"
}

return
