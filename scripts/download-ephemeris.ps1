$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot  = Split-Path -Parent $ScriptDir
$DataDir   = Join-Path $RepoRoot "data\ephemeris"
$Url       = "https://ssd.jpl.nasa.gov/ftp/eph/planets/bsp/de440.bsp"
$Dest      = Join-Path $DataDir "de440.bsp"

New-Item -ItemType Directory -Force -Path $DataDir | Out-Null

if (Test-Path $Dest) {
    Write-Host "de440.bsp already present at $Dest"
    exit 0
}

Write-Host "Downloading JPL DE440 ephemeris (~83 MB)..."
Invoke-WebRequest -Uri $Url -OutFile $Dest -UseBasicParsing
Write-Host "Done: $Dest"
