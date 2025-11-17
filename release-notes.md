## Features

- **Internal library patching**: Static libraries are now patched internally to simplify usage from Rust, reducing integration complexity
- **Windows ARM64 support**: Added Windows ARM64 as a supported target platform

## Breaking Changes

- **Additional system library dependencies**: On macOS and Windows, the following system libraries must now be linked:
  - **Windows**: `Synchronization` and `bcryptprimitives`
  - **macOS**: `CoreFoundation`
