param (
    [ValidateSet("Debug", "Development", "Release", "All")]
    [string]$BuildConfig = "Debug",
    [string]$SteamworksPath = $null,
    [string]$Arch = "x64"
)

#Import-Module (Join-Path $PSScriptRoot "Common.psm1")

# Ensure the script stops if any errors occur
$ErrorActionPreference = "Stop"

# Set GGPO's source directory
$ProjectRoot = $PSScriptRoot
$GGPOSourceDir = "$ProjectRoot/External/ggpo"

if (-not (Test-Path $GGPOSourceDir)) {
    New-Item -ItemType Directory -Path $GGPOSourceDir | Out-Null
}

$LogFile = "$GGPOSourceDir/BuildGGPO_log.txt"


function Resolve-PathSafe {
    param (
        [Parameter(Mandatory=$true)]
        [string] $FileName
    )

    $FileName = Resolve-Path $FileName -ErrorAction SilentlyContinue -ErrorVariable _frperror
    if (-not($FileName)) {
        # This ends up being the result of Resolve-Path if it doesn't exist on disk
        $FileName = $_frperror[0].TargetObject
    }

    return $FileName
}

function Add-Log {
    param (
        [string]$Message,
        [string]$Type = "Info"
    )

    $DateString = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host "$($DateString): [$Type] $Message"
    Add-Content -Path $LogFile -Value "[$Type] $Message"
}

function Update-GitSubmodules {
    param (
        [switch] $ForceSubmoduleUpdate = $false
    )

    Write-Host "Project root: $ProjectRoot"
    $ModuleContent = Get-Content (Resolve-PathSafe (Join-Path $ProjectRoot ".gitmodules"))
    $Modules = $ModuleContent | Select-String -Pattern "path = " | ForEach-Object { $_.Line -replace "path = ", "" }

    foreach ($Module in $Modules) {
        $ModuleDir = Resolve-PathSafe (Join-Path $ProjectRoot $Module.Trim().Replace("\t", ""))

        # Check if submodule exists, if not or if ForceSubmoduleUpdate is true, run submodule update
        if (-not (Test-Path $ModuleDir) -or $ForceSubmoduleUpdate) {
            git submodule update --init --recursive
        }
    }
}

Update-GitSubmodules

# ----------------------------------------------------
# Run the configure_windows.cmd script
# ----------------------------------------------------
# Navigate to the GGPO source directory
Push-Location $GGPOSourceDir

# Run the configuration script, if the build folder doesn't exist
if (-not (Test-Path "$GGPOSourceDir/build/CMakeCache.txt")) {
    if ($SteamworksPath -eq $null) {
        Add-Log "Steamworks path not specified, please specify the path to the Steamworks SDK using the -SteamworksPath parameter." Error
        return
    }

    Add-Log "Steamworks path: $SteamworksPath"
    .\configure_windows.cmd --no-prompt "$SteamworksPath"
}

# ----------------------------------------------------
# Build GGPO with CMake and MSBuild
# ----------------------------------------------------
# Define build configurations
$BuildConfigs = @("Debug", "Release") # Standard configurations for Windows

# Only build the specified configuration if not "All"
if ($BuildConfig -ne "All") {
    if ($BuildConfig -eq "Development") {
        $BuildConfigs = @("Release") # Map 'Development' to 'Release' for Windows
    } else {
        $BuildConfigs = @($BuildConfig)
    }
}

# Build the project
foreach ($Config in $BuildConfigs) {
    cmake --build .\build --config $Config -- /p:Platform=$Arch
}

# Return to the original directory
Pop-Location

Add-Log "GGPO build completed!"
# ----------------------------------------------------

# Export-ModuleMember -Variable '*'
# Export-ModuleMember -Function '*'