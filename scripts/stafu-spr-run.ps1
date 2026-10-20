param(
    [string]$Installed = ".\games\stafu",
    [string]$Cd = ".\games\stafu\cd",
    [string]$BuildDir = ".\out\build\x64-Debug",
    [string]$OutputDir = ".\out\stafu-spr-report",
    [switch]$SkipBuild,
    [switch]$SkipTests
)
$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
Set-Location $root
if (-not (Test-Path $Installed -PathType Container)) { throw "Installed game directory missing: $Installed" }
if (-not (Test-Path $Cd -PathType Container)) { throw "CD directory missing: $Cd" }
if (-not $SkipBuild) {
    cmake --build $BuildDir --config Debug --target reluva-tool
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $LASTEXITCODE" }
}
if (-not $SkipTests) {
    ctest --test-dir $BuildDir -C Debug --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "CTest failed: $LASTEXITCODE" }
}
$tool = Join-Path $BuildDir "Debug\reluva-tool.exe"
if (-not (Test-Path $tool)) { throw "Tool not found: $tool" }
New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
$names = @("BRDGPICA.SPR", "BRDGRIKR.SPR", "BRDGTROI.SPR", "BRDGWORF.SPR", "BRDGDATA.SPR", "brdgtitl.spr", "brdgdoor.spr", "viewscr.spr")
$summary = @()
foreach ($name in $names) {
    $log = Join-Path $OutputDir ($name + ".txt")
    & $tool -stafu spr-structure $Installed $Cd $name | Out-File -FilePath $log -Encoding utf8
    if ($LASTEXITCODE -ne 0) { throw "SPR inspection failed: $name" }
    $lines = @(Get-Content $log)
    $records = @($lines | Where-Object { $_ -match '^  record\[' }).Count
    $large = @($lines | Where-Object { $_ -match '^  record=\d+ offset=0x.* dataBytes=' }).Count
    $summary += [pscustomobject]@{ Resource=$name; Records=$records; LargePayloads=$large; Log=([IO.Path]::GetFileName($log)) }
    Write-Host "$name : $records records, $large large payloads"
}
$summary | ConvertTo-Json -Depth 3 | Set-Content (Join-Path $OutputDir "summary.json") -Encoding utf8
$summary | Format-Table -AutoSize
Write-Host "Reports: $OutputDir"
