param([string]$BuildDir = 'D:/Window-No-20-Tools/p1-host-tests')
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw '需要本机 Visual Studio C++ 编译工具。' }
$vcvars = Join-Path $vs 'VC/Auxiliary/Build/vcvars64.bat'
# 仅本次进程采用编译环境，不修改系统设置。
$compilerEnv = & $env:ComSpec /d /s /c "`"$vcvars`" >nul && set"
if ($LASTEXITCODE -ne 0) { throw '初始化 C++ 编译环境失败。' }
foreach ($line in $compilerEnv) {
  if ($line -match '^([^=]+)=(.*)$') { [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process') }
}
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
$exe = Join-Path $BuildDir 'p1-tests.exe'
Push-Location $BuildDir
try {
  & cl.exe /nologo /std:c++17 /EHsc /W4 /WX /utf-8 (Join-Path $repo 'firmware/p0_room/test/model_tests.cpp') (Join-Path $repo 'firmware/p0_room/src/state_tests.cpp') "/Fe:$exe"
  if ($LASTEXITCODE -ne 0) { throw '主机测试编译失败。' }
  & $exe
  if ($LASTEXITCODE -ne 0) { throw '主机测试失败。' }
} finally { Pop-Location }
