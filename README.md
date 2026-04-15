# cloudfile

Cloud File CLI is a command-line utility for interacting with cloud-stored files. It allows users to materialize (download) or evict (remove the local copy while keeping the cloud entry) through platform-specific backends.

## Usage

```sh
cloudfile <command> <file-path>
```

### Commands
- `materialize` - Downloads the file from the cloud
- `evict` - Removes the local copy while retaining it in the cloud
- `status` - Prints `evicted` or `materialized`

## Building and Installing

Ensure you have CMake installed before proceeding.

1. Clone the repository:
   ```sh
   git clone <repo-url>
   cd <repo-name>
   ```

2. Run the build script for your platform.

   On macOS or Linux:
   ```sh
   chmod +x scripts/build.sh
   ./scripts/build.sh
   ```

   On Windows PowerShell:
   ```powershell
   ./scripts/build.ps1
   ```

## Requirements
- CMake
- macOS with Clang/Xcode command line tools for the macOS backend
- Windows with a C++ toolchain and Cloud Files API support for the Windows backend
