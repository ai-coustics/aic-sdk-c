# aic-sdk - C Interface for ai-coustics SDK

Core C interface for the ai-coustics SDK.

For comprehensive documentation, visit [docs.ai-coustics.com](https://docs.ai-coustics.com).

> [!NOTE]
> This SDK requires a license key. Generate your key at [developers.ai-coustics.com](https://developers.ai-coustics.com).

## Installation

Download the SDK binaries from the [releases page](https://github.com/ai-coustics/aic-sdk-c/releases). We provide:

- **Static libraries** (`.a`, `.lib`) for linking directly into your application
- **Dynamic libraries** (`.so`, `.dll`, `.dylib`) for runtime loading
- **Multiple platforms**: Linux, Windows, macOS
- **Multiple architectures**: x86_64, ARM64

## Quick Start

All audio APIs are mono. Every buffer you hand to the SDK holds a single channel, so downmix
multichannel audio before passing it in, or create one instance per channel.

Every model runs at every supported sample rate (8000 - 192000 Hz) and at any block size, so you can
initialize for the format your host delivers. `aic_model_get_optimal_sample_rate` and
`aic_model_get_optimal_block_size` report the model's native configuration, which avoids internal
resampling and extra buffering and therefore gives the lowest delay.

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
    aic_processor_create(&processor, model, license_key, NULL);

    // Get optimal configuration
    uint32_t sample_rate;
    size_t block_size;
    aic_model_get_optimal_sample_rate(model, &sample_rate);
    aic_model_get_optimal_block_size(model, sample_rate, &block_size);

    // Initialize processor with optimal settings
    aic_processor_initialize(processor, sample_rate, block_size, false);

    // Process audio (Mono only)
    float* audio = (float*) calloc(block_size, sizeof(float));
    aic_processor_process(processor, audio, block_size);

    // Cleanup
    free(audio);
    aic_processor_destroy(processor);
    aic_model_destroy(model);
    return 0;
}
```

## Usage

### Error Handling

Every function returns an `AicErrorCode`. The snippets below leave the checks out to stay readable,
production code should not.

Most codes report a programming mistake you fix once: `AIC_ERROR_CODE_NULL_POINTER`,
`AIC_ERROR_CODE_NOT_INITIALIZED`, or `AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH` when a block is longer
than the configured `block_size`. The codes worth real handling are the license and authorization
ones.

Licensing and model type are checked when you create an object:

```c
enum AicErrorCode result = aic_processor_create(&processor, model, license_key, NULL);

switch (result) {
    case AIC_ERROR_CODE_SUCCESS:
        break;
    case AIC_ERROR_CODE_LICENSE_FORMAT_INVALID:
        fprintf(stderr, "License key is malformed\n");
        break;
    case AIC_ERROR_CODE_LICENSE_EXPIRED:
        fprintf(stderr, "License key has expired\n");
        break;
    case AIC_ERROR_CODE_LICENSE_VERSION_UNSUPPORTED:
        fprintf(stderr, "License key is not compatible with this SDK version\n");
        break;
    case AIC_ERROR_CODE_MODEL_TYPE_UNSUPPORTED:
        fprintf(stderr, "Model is not an enhancement or bypass model\n");
        break;
    default:
        fprintf(stderr, "Error: %d\n", result);
        break;
}
```

`aic_processor_process`, `aic_vad_process`, and `aic_analyzer_analyze_buffered` return
`AIC_ERROR_CODE_PROCESSING_NOT_ALLOWED` when the SDK key was not authorized or usage reporting
failed, usually a missing internet connection. This can appear mid-stream and not only at startup,
so handle it where you process audio, not just during setup.

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

A single loaded model handle can be reused to create multiple processors, VADs,
or analyzers, according to the model type.

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

// Get optimal block size for a specific sample rate
size_t optimal_block_size;
aic_model_get_optimal_block_size(model, 48000, &optimal_block_size);
printf("Optimal block size at 48kHz: %zu\n", optimal_block_size);
```

### Configuring the Processor

Initialize the processor for the format you will feed it, either the model's optimal configuration
(see [Model Information](#model-information)) for the lowest delay, or whatever your host delivers:

```c
// Create processor with license key
struct AicProcessor* processor = NULL;
aic_processor_create(&processor, model, license_key, NULL);

// Parameters: sample_rate, block_size, variable_block_size
aic_processor_initialize(processor, sample_rate, block_size, false);

// Any supported configuration works, for example 480 samples at 48 kHz
aic_processor_initialize(processor, 48000, 480, false);
```

`variable_block_size` describes how your host delivers audio. Leave it `false` when every call
passes exactly `block_size` samples, which is the common case and the lowest-delay one. Pass `true`
when the block length varies between calls: shorter calls are then accepted, at the cost of extra
buffering and therefore more delay. Calls larger than `block_size` are always rejected with
`AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH`.

The same flag exists on `aic_vad_initialize` and `aic_collector_initialize` and means the same
thing there.

You can create multiple independent processors from the same enhancement model handle.
Each processor shares the underlying model data internally.

### Processing Audio

```c
// Mono audio block
size_t audio_len = block_size;
float* audio_ptr = (float*) calloc(audio_len, sizeof(float));

// Process audio in-place
aic_processor_process(processor, audio_ptr, audio_len);

free(audio_ptr);
```

### Processor Context

The processor context provides thread-safe access to processor control APIs:

```c
// Create processor context for thread-safe control
struct AicProcessorContext* context = NULL;
aic_processor_context_create(&context, processor);

// Get the delay applied to the audio in samples
size_t delay;
aic_processor_context_get_audio_delay(context, &delay);
printf("Audio delay: %zu samples\n", delay);

// Reset processor state (clears internal buffers)
aic_processor_context_reset(context);

// Set enhancement parameters
aic_processor_context_set_parameter(context, AIC_PROCESSOR_PARAMETER_ENHANCEMENT_LEVEL, 0.8f);
aic_processor_context_set_parameter(context, AIC_PROCESSOR_PARAMETER_BYPASS, 0.0f);

// Get parameter values
float level;
aic_processor_context_get_parameter(context, AIC_PROCESSOR_PARAMETER_ENHANCEMENT_LEVEL, &level);
printf("Enhancement level: %.2f\n", level);

// Cleanup
aic_processor_context_destroy(context);
```

### Voice Activity Detection (VAD)

Voice activity detection runs on its own object, `AicVad`, and needs a dedicated VAD model.
Enhancement models are rejected by `aic_vad_create` with
`AIC_ERROR_CODE_MODEL_TYPE_UNSUPPORTED`.

#### Creating and Initializing a VAD

```c
// Load a dedicated VAD model
struct AicModel* vad_model = NULL;
aic_model_create_from_file(&vad_model, "path/to/vad_model.aicmodel");

// Create the VAD with your license key
struct AicVad* vad = NULL;
aic_vad_create(&vad, vad_model, license_key, NULL);

// Get optimal configuration for this model
uint32_t sample_rate;
size_t block_size;
aic_model_get_optimal_sample_rate(vad_model, &sample_rate);
aic_model_get_optimal_block_size(vad_model, sample_rate, &block_size);

// Initialize before processing any audio
// Parameters: sample_rate, block_size, variable_block_size
aic_vad_initialize(vad, sample_rate, block_size, false);
```

You can create multiple independent VAD instances from the same VAD model handle.
Each VAD shares the underlying model data internally.

#### VAD Context

The VAD context provides thread-safe access to the VAD's control and query APIs. Create it once
and keep it for as long as you need to read predictions or change parameters:

```c
struct AicVadContext* vad_context = NULL;
aic_vad_context_create(&vad_context, vad);

// Configure VAD parameters (all can be changed while audio is running)
aic_vad_context_set_parameter(vad_context, AIC_VAD_PARAMETER_SENSITIVITY, 0.8f);
aic_vad_context_set_parameter(vad_context, AIC_VAD_PARAMETER_SPEECH_HOLD_DURATION, 0.05f);
aic_vad_context_set_parameter(vad_context, AIC_VAD_PARAMETER_MINIMUM_SPEECH_DURATION, 0.0f);

// Get parameter values
float sensitivity;
aic_vad_context_get_parameter(vad_context, AIC_VAD_PARAMETER_SENSITIVITY, &sensitivity);
printf("VAD sensitivity: %.2f\n", sensitivity);

// How far behind its input the prediction is, in samples.
// This delay is not applied to the audio, the VAD never modifies the buffer.
size_t prediction_delay;
aic_vad_context_get_prediction_delay(vad_context, &prediction_delay);
printf("VAD prediction delay: %zu samples\n", prediction_delay);
```

#### Running the VAD

Drive the VAD one mono block at a time, then read the prediction. Unlike
`aic_processor_process`, the input is read-only and is not modified:

```c
float* audio_ptr = (float*) calloc(block_size, sizeof(float));

// Per audio block: process, then query
aic_vad_process(vad, audio_ptr, block_size);

bool is_speech_detected;
aic_vad_context_is_speech_detected(vad_context, &is_speech_detected);
if (is_speech_detected) {
    printf("Speech detected!\n");
}

// The model's direct output, without the SDK's post-processing
// (speech hold duration, sensitivity thresholding, ...)
float raw_probability;
aic_vad_context_get_raw_vad_probability(vad_context, &raw_probability);
```

Reset clears the internal buffers and the published prediction. Call it when the audio stream is
interrupted or when seeking, to prevent mispredictions from the previous content. The VAD stays
initialized:

```c
aic_vad_context_reset(vad_context);
```

Clean up in any order. The context may outlive the VAD, it just stops receiving new data:

```c
free(audio_ptr);
aic_vad_context_destroy(vad_context);
aic_vad_destroy(vad);
aic_model_destroy(vad_model);
```

### Combining Enhancement and VAD

Enhancement and VAD are independent objects, each with its own model. Run them side by side and
**feed the VAD the original input audio, not the processor's output.**

Create an `AicVad` from a VAD model and an `AicProcessor` from an enhancement model as shown
above, each with its own context. Initialize both for the same sample rate and block size, the
format your host delivers. The two models may report different optimal configurations, which does
not prevent running them on a common one.

In your audio callback, pass the same block to both. `aic_vad_process` does not modify its input,
so calling it first is all it takes to keep the VAD on the unprocessed signal:

```c
aic_vad_process(vad, audio_ptr, audio_len);              // reads the original input
aic_processor_process(processor, audio_ptr, audio_len);  // enhances it in-place

bool is_speech_detected;
aic_vad_context_is_speech_detected(vad_context, &is_speech_detected);
```

Because both objects read the same input, their delays are independent:

```c
size_t prediction_delay, audio_delay;
aic_vad_context_get_prediction_delay(vad_context, &prediction_delay);
aic_processor_context_get_audio_delay(proc_context, &audio_delay);

// The enhanced audio lags the input by audio_delay.
// The VAD prediction lags the same input by prediction_delay.
```

Avoid chaining the two, meaning feeding the processor's output into the VAD. Enhancement is
designed to change the signal, so the VAD would be detecting speech in audio that no longer
matches what its model expects, and the prediction would then lag the original input by
`audio_delay + prediction_delay`.

### Working with the Analyzer

Analysis needs a dedicated analysis model. Other model types are rejected by
`aic_analyzer_pair_create` with `AIC_ERROR_CODE_MODEL_TYPE_UNSUPPORTED`.

Instantiate an analyzer pair:

```c
struct AicCollector* collector = NULL;
struct AicAnalyzer* analyzer = NULL;

// Create an analyzer pair with your license key
aic_analyzer_pair_create(&collector, &analyzer, analysis_model, license_key);

// Initialize the collector, similar to the processor initialization
aic_collector_initialize(collector, sample_rate, block_size, false);
```

You can create multiple independent analyzer pairs from the same analysis model handle.
Each analyzer shares the underlying model data internally.

Buffer the audio with `aic_collector_buffer`, from your audio callback. Unlike
`aic_processor_process`, the input is read-only and is not modified:

```c
// Mono audio block
size_t audio_len = block_size;
float* audio_ptr = (float*) calloc(audio_len, sizeof(float));

// Buffer audio for later analysis
aic_collector_buffer(collector, audio_ptr, audio_len);

free(audio_ptr);
```

The analysis itself does not happen on its own. Nothing runs until you call
`aic_analyzer_analyze_buffered`, and you have to call it from a thread of your own, never from the
audio thread: analysis models are far too expensive for a real-time callback. That is the whole
reason the collector and the analyzer are separate objects. `aic_collector_buffer` may keep running
on the paired collector while an analysis is in progress.

The analysis model consumes a fixed length of audio, so if the collector has buffered less than
that, the tail of the input is analyzed as silence:

```c
struct AicAnalysisResult result = {0};
aic_analyzer_analyze_buffered(analyzer, &result);
printf("Risk score: %f\n", result.risk_score);
```

Reset clears the buffered audio and the internal state of both the analyzer and its collector. Call
it when the audio stream is interrupted or when seeking. The collector stays initialized:

```c
aic_analyzer_reset(analyzer);
```

## Examples

See the [`examples/enhancement.c`](examples/enhancement.c), [`examples/vad.c`](examples/vad.c), and [`examples/analyzer.c`](examples/analyzer.c) files for complete working examples.

To run the enhancement example with the default model:

```bash
export AIC_SDK_LICENSE="your_license_key_here"
make run-enhancement
```

To run the VAD example with a dedicated VAD model:

```bash
make run-vad MODEL_PATH="path/to/your/vad-model.aicmodel"
```

To run the analyzer example with the default analyzer model:

```bash
make run-analyzer
```

You can specify a model to use with any of the examples:

```bash
make run-enhancement MODEL_PATH="path/to/your/processor-model.aicmodel"
make run-vad MODEL_PATH="path/to/your/vad-model.aicmodel"
make run-analyzer MODEL_PATH="path/to/your/analyzer-model.aicmodel"
```

**Note:** If you don't provide `MODEL_PATH`, the Makefile will automatically download the default model for the enhancement and analyzer examples. `run-vad` requires `MODEL_PATH` until a default VAD model URL is configured. Use `make run-examples` to run the examples with configured default models; `run-examples` does not accept `MODEL_PATH` because the examples require different model types.

## Documentation

- **Full Documentation**: [docs.ai-coustics.com](https://docs.ai-coustics.com)
- **C API Reference**: See the [header file](include/aic.h) for detailed API documentation
- **Available Models**: [artifacts.ai-coustics.io](https://artifacts.ai-coustics.io)

## License

This C interface is distributed under the Apache 2.0 license. The core SDK library is distributed under the proprietary AIC-SDK license.
