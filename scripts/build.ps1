$ErrorActionPreference = "Stop"

Write-Host "Checking for build directory..."

if (Test-Path -Path "build" -PathType Container) {
    Write-Host "build directory already exists."
    Write-Host "Removing..."
    Remove-Item -Path "build" -Recurse -Force
}

Write-Host "Creating build directory"
New-Item -ItemType Directory -Path "build" | Out-Null

Write-Host "Moving into build directory"
Push-Location "build"

try {
    Write-Host "Generating build environment..."
    cmake ..

    Write-Host "Making..."
    cmake --build .
}
finally {
    Pop-Location
}
