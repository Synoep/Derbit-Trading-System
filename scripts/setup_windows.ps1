Write-Output "Setting up Deribit Trading System on Windows..."

# Install Chocolatey if not installed
if (-Not (Get-Command choco -ErrorAction SilentlyContinue)) {
    Write-Output "Chocolatey not found, installing..."
    Set-ExecutionPolicy Bypass -Scope Process -Force
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
    Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
}

# Install dependencies
Write-Output "Installing dependencies..."
choco install cmake git visualstudio2022buildtools -y
choco install boost-msvc-14.3 openssl.light -y

# Build the project
Write-Output "Building the project..."
mkdir build
Set-Location -Path .\build
cmake .. -Wno-dev
cmake --build .

Write-Output "Setup complete. You can now run './deribit_trader' to start the application."

./deribit_trader