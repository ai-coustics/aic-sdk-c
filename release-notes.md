## Platform Support

- Windows MSVC: the shipped `aic.dll` (x86_64 and arm64) now statically links the MSVC C runtime, so it no longer requires the Visual C++ Redistributable to be installed.
- Windows MSVC: both CRT variants of the static import library are now shipped, `lib/dynamic-crt/aic.lib` and `lib/static-crt/aic.lib`. This lets consumers match their own runtime setting and avoids LNK2038 mismatches for projects building with the default `/MD`, which a `/MT`-only library would break.
- Apple: the release artifacts now include an `aic-sdk-apple-xcframework` bundle covering macOS, Mac Catalyst, iOS, tvOS, and visionOS (device and simulator).
