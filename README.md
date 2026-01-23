# aic-sdk - C Interface for ai-coustics SDK

Core C interface for the ai-coustics Speech Enhancement SDK. 

For comprehensive documentation, visit [docs.ai-coustics.com](https://docs.ai-coustics.com).

> [!NOTE]
> This SDK requires a license key. Generate your key at [developers.ai-coustics.io](https://developers.ai-coustics.io).

## Installation

Download the SDK binaries from the [releases page](https://github.com/ai-coustics/aic-sdk-c/releases). We provide:

- **Static libraries** (`.a`, `.lib`) for linking directly into your application
- **Dynamic libraries** (`.so`, `.dll`, `.dylib`) for runtime loading
- **Multiple platforms**: Linux, Windows, macOS
- **Multiple architectures**: x86_64, ARM64

## Quick Start

```c
#include <stdlib.h>
#include "aic.h"

int main() {
    // Get your license key from the environment variable
    const char* license_key = getenv("AIC_SDK_LICENSE");
    
    // Download and load a model (or download manually at https://artifacts.ai-coustics.io/)
    struct AicModel* model = NULL;
    aic_model_create_from_file(&model, "path/to/model.aicmodel");
    
    // Create processor with license key
    struct AicProcessor* processor = NULL;
    aic_processor_create(&processor, model, license_key);
    
    // Get optimal configuration
    uint32_t sample_rate;
    size_t num_frames;
    aic_model_get_optimal_sample_rate(model, &sample_rate);
    aic_model_get_optimal_num_frames(model, sample_rate, &num_frames);
    
    // Initialize processor with optimal settings
    aic_processor_initialize(processor, sample_rate, 2, num_frames, false);
    
    // Process audio (planar layout: separate buffers per channel)
    float* audio_left = (float*) calloc(num_frames, sizeof(float));
    float* audio_right = (float*) calloc(num_frames, sizeof(float));
    float* audio_planar[2] = {audio_left, audio_right};
    aic_processor_process_planar(processor, audio_planar, 2, num_frames);
    
    // Cleanup
    free(audio_left);
    free(audio_right);
    aic_processor_destroy(processor);
    aic_model_destroy(model);
    return 0;
}
```

## Usage

### SDK Information

```c
// Get SDK version
const char* version = aic_get_sdk_version();
printf("SDK version: %s\n", version);

// Get compatible model version
uint32_t model_version = aic_get_compatible_model_version();
printf("Compatible model version: %u\n", model_version);
```

### Loading Models

Download models and find available IDs at [artifacts.ai-coustics.io](https://artifacts.ai-coustics.io/).

#### From File
```c
struct AicModel* model = NULL;
enum AicErrorCode result = aic_model_create_from_file(&model, "path/to/model.aicmodel");
if (result != AIC_ERROR_CODE_SUCCESS) {
    // Handle error
}
```

#### From Memory Buffer
```c
// Buffer must be 64-byte aligned and remain valid for the model's lifetime
uint8_t* buffer = /* ... */;
size_t buffer_len = /* ... */;

struct AicModel* model = NULL;
enum AicErrorCode result = aic_model_create_from_buffer(&model, buffer, buffer_len);
if (result != AIC_ERROR_CODE_SUCCESS) {
    // Handle error
}
```

### Model Information

```c
// Get model ID
const char* model_id = aic_model_get_id(model);
printf("Model ID: %s\n", model_id);

// Get optimal sample rate for the model
uint32_t optimal_rate;
aic_model_get_optimal_sample_rate(model, &optimal_rate);
printf("Optimal sample rate: %u Hz\n", optimal_rate);

// Get optimal frame count for a specific sample rate
size_t optimal_frames;
aic_model_get_optimal_num_frames(model, 48000, &optimal_frames);
printf("Optimal frames at 48kHz: %zu\n", optimal_frames);
```

### Configuring the Processor

```c
// Create processor with license key
struct AicProcessor* processor = NULL;
aic_processor_create(&processor, model, license_key);

// Get optimal configuration
uint32_t sample_rate;
size_t num_frames;
aic_model_get_optimal_sample_rate(model, &sample_rate);
aic_model_get_optimal_num_frames(model, sample_rate, &num_frames);

// Initialize with optimal settings
// Parameters: sample_rate, num_channels, num_frames, allow_variable_frames
aic_processor_initialize(processor, sample_rate, 2, num_frames, false);

// Or initialize with custom settings
aic_processor_initialize(processor, 48000, 2, 480, false);
```

### Processing Audio

The C interface provides three audio buffer formats:

#### Planar Layout (Separate Buffers per Channel)
```c
// Each channel has its own buffer
float* audio_left = (float*) calloc(num_frames, sizeof(float));
float* audio_right = (float*) calloc(num_frames, sizeof(float));
float* audio_planar[2] = {audio_left, audio_right};

// Process audio in-place
aic_processor_process_planar(processor, audio_planar, 2, num_frames);

// Cleanup
free(audio_left);
free(audio_right);
```

#### Interleaved Layout (Channels Alternating)
```c
// Single buffer with channels interleaved: [L, R, L, R, ...]
float* audio = (float*) calloc(num_channels * num_frames, sizeof(float));

// Process audio in-place
aic_processor_process_interleaved(processor, audio, num_channels, num_frames);

free(audio);
```

#### Sequential Layout (All Channel Data Sequential)
```c
// Single buffer with all data for each channel in sequence: [L..L, R...R]
float* audio = (float*) calloc(num_channels * num_frames, sizeof(float));

// Process audio in-place
aic_processor_process_sequential(processor, audio, num_channels, num_frames);

free(audio);
```

### Processor Context

The processor context provides thread-safe access to processor control APIs:

```c
// Create processor context for thread-safe control
struct AicProcessorContext* context = NULL;
aic_processor_context_create(&context, processor);

// Get output delay in samples
size_t delay;
aic_processor_context_get_output_delay(context, &delay);
printf("Output delay: %zu samples\n", delay);

// Reset processor state (clears internal buffers)
aic_processor_context_reset(context);

// Set enhancement parameters
aic_processor_context_set_parameter(context, AIC_PROCESSOR_PARAMETER_ENHANCEMENT_LEVEL, 0.8f);
aic_processor_context_set_parameter(context, AIC_PROCESSOR_PARAMETER_VOICE_GAIN, 1.5f);
aic_processor_context_set_parameter(context, AIC_PROCESSOR_PARAMETER_BYPASS, 0.0f);

// Get parameter values
float level;
aic_processor_context_get_parameter(context, AIC_PROCESSOR_PARAMETER_ENHANCEMENT_LEVEL, &level);
printf("Enhancement level: %.2f\n", level);

// Cleanup
aic_processor_context_destroy(context);
```

### Voice Activity Detection (VAD)

```c
// Create VAD context from processor
struct AicVadContext* vad_context = NULL;
aic_vad_context_create(&vad_context, processor);

// Configure VAD parameters
aic_vad_context_set_parameter(vad_context, AIC_VAD_PARAMETER_SENSITIVITY, 6.0f);
aic_vad_context_set_parameter(vad_context, AIC_VAD_PARAMETER_SPEECH_HOLD_DURATION, 0.05f);
aic_vad_context_set_parameter(vad_context, AIC_VAD_PARAMETER_MINIMUM_SPEECH_DURATION, 0.0f);

// Get parameter values
float sensitivity;
aic_vad_context_get_parameter(vad_context, AIC_VAD_PARAMETER_SENSITIVITY, &sensitivity);
printf("VAD sensitivity: %.2f\n", sensitivity);

// Check for speech (after processing audio through the processor)
bool is_speech_detected;
aic_vad_context_is_speech_detected(vad_context, &is_speech_detected);
if (is_speech_detected) {
    printf("Speech detected!\n");
}

// Cleanup
aic_vad_context_destroy(vad_context);
```

### Error Handling

All functions return `AicErrorCode` for proper error handling:

```c
enum AicErrorCode result = aic_processor_initialize(processor, 48000, 2, 480, false);

switch (result) {
    case AIC_ERROR_CODE_SUCCESS:
        // Success
        break;
    case AIC_ERROR_CODE_NULL_POINTER:
        fprintf(stderr, "Error: NULL pointer provided\n");
        break;
    case AIC_ERROR_CODE_AUDIO_CONFIG_UNSUPPORTED:
        fprintf(stderr, "Error: Audio configuration not supported\n");
        break;
    // ... handle other error codes
    default:
        fprintf(stderr, "Error: Unknown error code %d\n", result);
        break;
}
```

## Examples

See the [`examples/basic.c`](examples/basic.c) file for a complete working example.

To run the example:

```bash
export AIC_SDK_LICENSE="your_license_key_here"
make run MODEL_PATH="path/to/your/model.aicmodel"
```

**Note:** If you don't provide `MODEL_PATH`, the Makefile will automatically download a small model for you.

## Documentation

- **Full Documentation**: [docs.ai-coustics.com](https://docs.ai-coustics.com)
- **C API Reference**: See the [header file](../include/aic.h) for detailed API documentation
- **Available Models**: [artifacts.ai-coustics.io](https://artifacts.ai-coustics.io)

## License

This C interface is distributed under the Apache 2.0 license. The core SDK library is distributed under the proprietary AIC-SDK license.
