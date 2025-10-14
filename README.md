# ai-coustics Speech Enhancement SDK

Welcome to the ai-coustics SDK! This repository provides the core C interface to our powerful real-time speech enhancement library, serving as the foundation for all language-specific wrappers and integrations.

## 🎯 What is this SDK?

Our Speech Enhancement SDK delivers state-of-the-art audio processing capabilities, enabling you to:
- Remove background noise from audio recordings
- Enhance speech clarity and intelligibility
- Process audio in real-time
- Integrate seamlessly into your applications

## 🚀 Quick Start

### Acquire an SDK License Key

To use the SDK, you'll need a license key. Contact our team to receive your time-limited demo key:

- **Email**: [info@ai-coustics.com](mailto:info@ai-coustics.com)
- **Website**: [ai-coustics.com](https://ai-coustics.com)

Once you have your license key, set it as an environment variable or pass it directly to the SDK initialization functions.

### Download the SDK

Get started by downloading the SDK binaries from the [releases page](https://github.com/ai-coustics/aic-sdk-c/releases). We provide:

- **Static libraries** (`.a`, `.lib`) for linking directly into your application
- **Dynamic libraries** (`.so`, `.dll`, `.dylib`) for runtime loading
- **Multiple platforms**: Linux, Windows, macOS
- **Multiple architectures**: x86_64, ARM64, and more

### Language Bindings

While this repository contains the core C interface, we offer convenient wrappers for popular programming languages:

| Language | Repository | Description |
|----------|------------|-------------|
| **C++** | [`aic-sdk-cpp`](https://github.com/ai-coustics/aic-sdk-cpp) | Modern C++ wrapper with RAII and type safety |
| **Python** | [`aic-sdk-py`](https://github.com/ai-coustics/aic-sdk-py) | Pythonic interface |
| **Node.js** | [`aic-sdk-node`](https://github.com/ai-coustics/aic-sdk-node) | JavaScript/TypeScript bindings for Node.js |
| **WebAssembly** | [`aic-sdk-wasm`](https://github.com/ai-coustics/aic-sdk-wasm) | Browser-compatible WebAssembly build |

## 📚 Documentation

- **[SDK Reference](docs/sdk-reference.md)** - Complete API documentation and function reference
- **[Basic Example](examples/basic.c)** - Sample code and integration patterns

## 💡 Example Usage

Here's a simple example showing the core SDK workflow. Error handling is omitted for clarity - see the [complete example](examples/basic.c) for production-ready code:

```c
#include "aic.h"

int main() {
    // Load your license key
    const char* license = getenv("AIC_SDK_LICENSE");

    // Create a new model
    struct AicModel* model = NULL;
    aic_model_create(&model, AIC_MODEL_TYPE_QUAIL_S, license);

    // Initialize with your audio settings
    aic_model_initialize(model, 48000, 2, 480);

    // Configure enhancement parameters
    aic_model_set_parameter(model, AIC_PARAMETER_ENHANCEMENT_LEVEL, 0.7f);

    // Process audio (interleaved version available as well)
    float* audio_buffer_left = (float*) calloc(480, sizeof(float));
    float* audio_buffer_right = (float*) calloc(480, sizeof(float));
    float* audio_buffer_planar[2] = {audio_buffer_left, audio_buffer_right};
    aic_model_process_planar(model, audio_buffer_planar, 2, 480);

    // Cleanup resources
    free(audio_buffer);
    aic_model_destroy(model);
    return 0;
}
```

## 🏃 Running the Example

To build and run the provided example code, use the following command:

```bash
make LIB_PATH=/path/to/your/libaic.a INCLUDE_PATH=/path/to/your/headers
```

Replace `/path/to/your/libaic.a` with the actual path to your downloaded SDK library file, and `/path/to/your/headers` with the path to the directory containing the SDK header files.

Make sure you have set your license key as an environment variable:

```bash
export AIC_SDK_LICENSE="your_license_key_here"
./aic-sdk-example
```

## 🆘 Support & Resources

Need help? We're here to assist:

- **Documentation**: [Complete SDK Reference](docs/sdk-reference.md)
- **Examples**: [Sample Code](examples/basic.c)
- **Issues**: [GitHub Issues](https://github.com/ai-coustics/aic-sdk-c/issues)
- **Technical Support**: [systems@ai-coustics.com](mailto:systems@ai-coustics.com)

---

Made with ❤️ by the ai-coustics team
