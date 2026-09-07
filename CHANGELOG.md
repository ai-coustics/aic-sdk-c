# Changelog

## 0.24.0

### New Features

#### SDK-internal error reporting

The SDK reports its own backend failures to ai-coustics error tracking. Covered are failed session
activations, failed usage reports, and bearer token refreshes rejected by
`aic_processor_context_update_bearer_token` or its VAD and analyzer counterparts.

A report contains the error class and message, the SDK version and wrapper, the model ID, the
operating system, the CPU architecture, and the account the license was issued to. It contains no
audio, no license key and no bearer token.

Disable reporting with `DO_NOT_TRACK=1`. The variable is read once per process. Licenses with an
offline entitlement and `wasm32` builds never report.

## 0.23.1 - 2026-09-07

No changes to the C SDK. The version moved with a Rust bindings release.

## 0.23.0 - 2026-08-10

### Breaking Changes

#### New model file version

This release requires model file version 7. Re-download your models so the SDK does not reject them with `AIC_ERROR_CODE_MODEL_VERSION_UNSUPPORTED`. See the [compatibility matrix](https://docs.ai-coustics.com/reference/sdk/compatibility-matrix).

### New Features

#### Tyto 1.0 has been replaced by Tyto 1.1

The analysis model is now Tyto 1.1, `tyto-1.1-l-16khz`. Tyto 1.0 (`tyto-l-16khz`) is not
loadable by this SDK version any more.

#### Analysis result fields changed

`AicAnalysisResult` gained a field and lost one:

- Added: `codec_degradation`, a measure of artifacts introduced by lossy speech codecs.
- Removed: `media_speech`.

### Bug Fixes

- `aic_analyzer_analyze_buffered` no longer crashes when OpenTelemetry reporting is enabled.

## 0.22.0 - 2026-08-05

This release contains two breaking changes that affect every integration:

- **All audio APIs are mono only.** The multi-channel process/buffer functions are gone.
- **The VAD is its own object.** VAD runs on dedicated VAD models through `AicVad`, and can no
   longer be derived from a processor.

Both migrations are covered step by step below.

### Breaking Changes

#### Multi-channel support removed

The processor and the analyzer's collector now operate on mono audio only.

All models process mono inputs. Previously the processor mixed all input channels down to mono
internally, which could lead to surprising results. To prevent misunderstandings, all APIs now take
exclusively mono inputs.

To process multi-channel audio, downmix to mono before calling `aic_processor_process`, or create a
separate processor instance per channel.

##### What changed

| Before (0.21.4) | Now |
| --- | --- |
| `aic_processor_initialize(processor, sample_rate, num_channels, num_frames, allow_variable_frames)` | `aic_processor_initialize(processor, sample_rate, block_size, variable_block_size)` |
| `aic_processor_process_planar` / `aic_processor_process_interleaved` / `aic_processor_process_sequential` | `aic_processor_process(processor, audio_ptr, audio_len)` |
| `aic_collector_initialize(collector, sample_rate, num_channels, num_frames, allow_variable_frames)` | `aic_collector_initialize(collector, sample_rate, block_size, variable_block_size)` |
| `aic_collector_buffer_planar` / `aic_collector_buffer_interleaved` / `aic_collector_buffer_sequential` | `aic_collector_buffer(collector, audio_ptr, audio_len)` |
| `aic_model_get_optimal_num_frames` | `aic_model_get_optimal_block_size` |

Naming also became consistent: the configured block length is `block_size` on
`aic_processor_initialize` and `aic_collector_initialize`, the per-call buffer length is `audio_len`
on `aic_processor_process` and `aic_collector_buffer`, and the `allow_variable_frames` flag is now
`variable_block_size`.

##### Before

```c
uint32_t sample_rate;
size_t   num_frames;
aic_model_get_optimal_sample_rate(model, &sample_rate);
aic_model_get_optimal_num_frames(model, sample_rate, &num_frames);

// Stereo in, stereo out. The SDK mixed both channels down to mono internally.
aic_processor_initialize(processor, sample_rate, 2, num_frames, false);

float *planar[2] = {left, right};
aic_processor_process_planar(processor, planar, 2, num_frames);
```

##### After

```c
uint32_t sample_rate;
size_t   block_size;
aic_model_get_optimal_sample_rate(model, &sample_rate);
aic_model_get_optimal_block_size(model, sample_rate, &block_size);

aic_processor_initialize(processor, sample_rate, block_size, false);

// Downmix to mono yourself, then process a single buffer in-place.
for (size_t i = 0; i < block_size; i++) {
  mono[i] = 0.5f * (left[i] + right[i]);
}

aic_processor_process(processor, mono, block_size);
```

If you need per-channel output instead of a downmix, create one processor per channel and call
`aic_processor_process` once per channel with that channel's buffer.

The same applies to the analyzer: downmix multichannel audio before calling
`aic_collector_buffer`, or create a separate collector/analyzer pair per channel.

The OpenTelemetry `audio.channels` metric has been kept for backwards compatibility, but it now
always reports exactly one channel.

#### VAD moved into its own object, energy-based VAD removed

Voice activity detection is no longer a side effect of enhancement. It is now a first-class object,
`AicVad`, that runs a dedicated VAD model.

Energy-based VADs, which inferred speech activity from the output level of an enhancement model,
have been removed. They were an approximation and their accuracy depended on the enhancement model
in use. A dedicated VAD model is trained for the task and is considerably more accurate.

##### What changed

| Before (0.21.4) | Now |
| --- | --- |
| VAD came from a processor: `aic_vad_context_create(&context, processor)` | VAD is standalone: `aic_vad_create` → `aic_vad_initialize` → `aic_vad_process`, then `aic_vad_context_create(&context, vad)` |
| Any enhancement model provided a VAD (energy-based), VAD models were also loaded into a processor | Only dedicated VAD models are accepted by `aic_vad_create`; `aic_processor_create` accepts only enhancement models |
| VAD advanced whenever the processor processed audio | VAD advances on `aic_vad_process`, independently of any processor |
| `AIC_VAD_PARAMETER_SENSITIVITY` ranged 0.0 - 1.0 on VAD models and 1.0 - 15.0 on energy-based VADs | `AIC_VAD_PARAMETER_SENSITIVITY` is always a probability threshold, 0.0 - 1.0 |

##### Before

```c
// One model, one processor: enhancement and VAD were coupled.
struct AicProcessor *processor = NULL;
aic_processor_create(&processor, enhancement_model, license, NULL);
aic_processor_initialize(processor, sample_rate, 1, num_frames, false);

struct AicVadContext *vad_context = NULL;
aic_vad_context_create(&vad_context, processor);
aic_vad_context_set_parameter(vad_context, AIC_VAD_PARAMETER_SENSITIVITY, 5.0f); // energy threshold

// The VAD updated as a side effect of enhancement.
aic_processor_process_interleaved(processor, audio, 1, num_frames);

bool is_speech_detected = false;
aic_vad_context_is_speech_detected(vad_context, &is_speech_detected);

aic_vad_context_destroy(vad_context);
aic_processor_destroy(processor);
```

##### After

```c
// Load a dedicated VAD model and create an AicVad from it.
struct AicModel *vad_model = NULL;
aic_model_create_from_file(&vad_model, "path/to/vad-model.aicmodel");

struct AicVad *vad = NULL;
// Returns AIC_ERROR_CODE_MODEL_TYPE_UNSUPPORTED if the model is not a VAD model.
aic_vad_create(&vad, vad_model, license, NULL);

uint32_t sample_rate;
size_t   block_size;
aic_model_get_optimal_sample_rate(vad_model, &sample_rate);
aic_model_get_optimal_block_size(vad_model, sample_rate, &block_size);
aic_vad_initialize(vad, sample_rate, block_size, false);

struct AicVadContext *vad_context = NULL;
aic_vad_context_create(&vad_context, vad);
aic_vad_context_set_parameter(vad_context, AIC_VAD_PARAMETER_SENSITIVITY, 0.8f); // probability

// The VAD is driven explicitly and does not modify the audio.
aic_vad_process(vad, audio, block_size);

bool is_speech_detected = false;
aic_vad_context_is_speech_detected(vad_context, &is_speech_detected);

aic_vad_context_destroy(vad_context);
aic_vad_destroy(vad);
aic_model_destroy(vad_model);
```

##### Run the VAD on the original audio

If you use enhancement and VAD together, **feed the VAD the original input audio, not the
processor's enhanced output.** Run the two objects side by side on the same mono block rather than chaining
them:

```c
// Recommended: both objects see the same original input block.
aic_vad_process(vad, input, block_size);             // reads the block, does not modify it
aic_processor_process(processor, input, block_size); // enhances the block in-place
```

`aic_vad_process` takes a `const float *` and leaves the buffer untouched, so calling it on the same
buffer before `aic_processor_process` is all it takes to keep the VAD on the unprocessed signal.

Enhancement is designed to change the signal, so running the VAD on its output means detecting
speech in audio that no longer matches what the VAD model expects. It also stacks the processor's
delay on top of the VAD's own prediction delay, which makes speech decisions harder to align.

##### Delay queries renamed

There is no single "output delay" any more. The processor delays audio, the VAD does not, so the two
queries are now named after what they actually report:

| Before (0.21.4) | Now | What it means |
| --- | --- | --- |
| `aic_processor_context_get_output_delay` | `aic_processor_context_get_audio_delay` | An **audio** delay. The enhanced samples leave `aic_processor_process` that many samples behind their input. |
| No VAD-specific query. The VAD was driven by a processor, so its prediction delay was the processor's `aic_processor_context_get_output_delay`. | `aic_vad_context_get_prediction_delay` | A **prediction** delay. It is *not* applied to the audio, `aic_vad_process` leaves the buffer untouched. It tells you how far behind its own input the published prediction is, so you can line speech decisions up with the audio timeline. |

With both objects fed from the same input block, the two delays are independent of each other:

```c
size_t audio_delay, prediction_delay;
aic_processor_context_get_audio_delay(proc_context, &audio_delay);
aic_vad_context_get_prediction_delay(vad_context, &prediction_delay);

// The enhanced audio lags the input by audio_delay.
// The VAD prediction lags the same input by prediction_delay.
```

`AicVad` mirrors the processor's lifecycle and control surface:

- `aic_vad_create`, `aic_vad_initialize`, `aic_vad_process`, `aic_vad_destroy`
- `aic_vad_context_create` / `aic_vad_context_destroy` for thread-safe control handles
- `aic_vad_context_reset`, `aic_vad_context_get_prediction_delay`,
  `aic_vad_context_update_bearer_token`
- `aic_vad_context_is_speech_detected`, `aic_vad_context_get_raw_vad_probability`,
  `aic_vad_context_set_parameter`, `aic_vad_context_get_parameter`

See `examples/vad.c` for a complete, error-checked example.

The OpenTelemetry `experimental.vad.speech_duration` metric is now reported for dedicated VAD models
only. Enhancement and bypass sessions no longer derive VAD state from the model output, so they
always report zero. `processor.vad_created` reports whether the session runs a VAD model.

#### Renamed error codes

Some error codes are no longer processor-specific, since they are now also returned by the VAD:

| Before (0.21.4) | Now |
| --- | --- |
| `AIC_ERROR_CODE_PROCESSOR_NOT_INITIALIZED` | `AIC_ERROR_CODE_NOT_INITIALIZED` |
| `AIC_ERROR_CODE_ENHANCEMENT_NOT_ALLOWED` | `AIC_ERROR_CODE_PROCESSING_NOT_ALLOWED` |
| `AIC_ERROR_CODE_MODEL_FILE_PATH_INVALID` | `AIC_ERROR_CODE_FILE_PATH_INVALID` |

The numeric values are unchanged, so only source-level references need updating.

### New Features

There are new explicit session termination APIs. Use these APIs to close telemetry sessions
during lifecycle events without waiting for the corresponding SDK object to be destroyed,
which is useful in integrations where object deallocation may be delayed.

- `aic_processor_terminate_session`
- `aic_vad_terminate_session`
- `aic_analyzer_terminate_session`

### Bug Fixes

- Resetting VAD state now immediately clears the published speech detection and
  raw VAD probability values, so query APIs no longer return stale values from
  the previous stream after reset.

## 0.21.4 - 2026-07-08

### Platform Support

- The Rust SDK can now be built with any Rust version, including the same version this library was built with.
- Enabled static linking for Android in the Rust SDK.

## 0.21.3 - 2026-07-03

### Platform Support

- Windows MSVC: the shipped `aic.dll` (x86_64 and arm64) now statically links the MSVC C runtime, so it no longer requires the Visual C++ Redistributable to be installed.
- Windows MSVC: both CRT variants of the static import library are now shipped, `lib/dynamic-crt/aic.lib` and `lib/static-crt/aic.lib`. This lets consumers match their own runtime setting and avoids LNK2038 mismatches for projects building with the default `/MD`, which a `/MT`-only library would break.
- Apple: the release artifacts now include an `aic-sdk-apple-xcframework` bundle covering macOS, Mac Catalyst, iOS, tvOS, and visionOS (device and simulator).

## 0.21.2 - 2026-06-30

### Platform Support

- Added Windows GNU/LLVM release targets `x86_64-pc-windows-gnullvm` and `aarch64-pc-windows-gnullvm`

## 0.21.1 - 2026-06-26

### New Features

Support for offline entitlements in JWT licenses.

## 0.21.0 - 2026-06-22

### New Features

This release includes a new `aic_vad_context_get_raw_vad_probability()` API to read the raw output of a VAD model.

### Changes

Reduced the necessary output delay of the Processor when using `allow_variable_frames = true`.

## 0.20.0 - 2026-06-10

### New Features

This release includes several new APIs for running our newest audio intelligence model, *Tyto*.

The new APIs introduce two new concepts: The `Collector` and the `Analyzer`.
 - The `Collector` is designed to be placed in the audio thread, buffering audio chunks for later analysis.
 - The `Analyzer` is designed to be run separately. Analysis models are computationally expensive and cannot run in the audio thread. The analyzer has access to the audio buffered by the collector, and it can access it safely across threads.

Initialize the `Collector` with the same configuration as your existing `Processor` and you can
call the `aic_collector_buffer_*` APIs in the same manner as the `aic_processor_process_*` APIs.

Call `aic_analyzer_analyze_buffered` in a separate thread to obtain an analysis of the latest
audio buffered by the `Collector`.

## 0.19.3 - 2026-06-09

### Platform Support

- Fixed a crash on Android during startup

## 0.19.2 - 2026-06-05

### Platform Support

- Added Android targets arm-v7a and x86_64

## 0.19.1 - 2026-06-02

### New Features

- Windows release bundles include the DLL import library `aic.dll.lib`
- Added iOS simulator arm64 release target `aarch64-apple-ios-sim`
- Enabled Rust wrapper support across all Apple targets

## 0.19.0 - 2026-05-27

### Features

#### JWT token support

The SDK now accepts JWT-form license keys in addition to the existing license
formats. Pass the JWT string as `license_key` to `aic_processor_create` as you
would with any other license key:

```c
aic_processor_create(&processor, model, jwt_string, NULL);
```

JWT licenses are short-lived. Use `aic_processor_context_update_bearer_token`
to swap in a renewed token while audio processing continues uninterrupted, the
context handle stays valid, and the new token is used for all subsequent
authentication against the ai-coustics backend:

```c
enum AicErrorCode aic_processor_context_update_bearer_token(
    const struct AicProcessorContext *context,
    const char *token);
```

In-place updates are only supported when both the originally configured key and
the new token are JWTs. If either side is not, the call returns the new error
code `AIC_ERROR_CODE_TOKEN_UPDATE_UNSUPPORTED` and the existing token stays in
use.

#### Voice activity detection models

VAD is no longer limited to the energy-based detector. The SDK now also
supports dedicated VAD models, which output a per-buffer speech probability.
`AIC_VAD_PARAMETER_SENSITIVITY` is interpreted differently depending on the
active model:

- VAD models: range `0.0` to `1.0`; the probability threshold above which
  speech is reported.
- Energy-based VADs: range `1.0` to `15.0`; energy threshold =
  `10 ^ (-sensitivity)`.

The default sensitivity is now model-specific.

#### Configurable OpenTelemetry export interval

`AicOtelConfig` has a new field `export_interval_ms` controlling how often
OpenTelemetry metrics are exported. Set it to `0` to keep the default of
60 000 ms:

```c
AicOtelConfig otel = {
  .enable = true,
  .session_id = NULL,
  .export_interval_ms = 5000, // export every 5 seconds
};
aic_processor_create(&processor, model, license_key, &otel);
```

### Breaking Changes

- Compatible model file version was bumped to 4. Models built for earlier versions are no longer supported.

## 0.18.0 - 2026-05-19

### Features

The SDK now supports observability with [OpenTelemetry](https://opentelemetry.io/).
See the [documentation](https://docs.ai-coustics.com/guides/observability) for the complete setup guide.
`aic_processor_create` takes a new `otel_config` parameter with two usage options:

**Environment variable**: pass `NULL` and set `AIC_SDK_OTEL_ENABLE=1` to enable telemetry globally:

```c
aic_processor_create(&processor, model, license_key, NULL);
```

**Per-session**: pass an `AicOtelConfig` to enable telemetry and associate it with a specific session ID:

```c
typedef struct AicOtelConfig {
  bool enable;
  const char *session_id; // NULL = auto-generate
} AicOtelConfig;

AicOtelConfig otel = { .enable = true, .session_id = "my-session-id" };
aic_processor_create(&processor, model, license_key, &otel);
```

### Breaking Changes

`aic_processor_create` has a new `otel_config` parameter. Existing call sites must be updated to pass `NULL` as the last argument.

## 0.17.1 - 2026-05-06

### Improvements

- Increased maximum VAD speech hold duration from 100x to 300x the model's window size.

### Bug Fixes

- Removed zero-padding when the host frame size does not match the model frame size, which caused unexpected behavior for some models.

## 0.17.0 - 2026-04-23

### New Features

- Added support for Quail Voice Focus 2.1 models.

### Breaking Changes

- Quail Voice Focus 2.0 is no longer supported.
- Compatible model file version was bumped to 3.

## 0.16.0 - 2026-04-16

### New Features

This release adds an **experimental** feature to export real-time audio processing metrics via OpenTelemetry (OTel).
The new feature is currently disabled by default and available for testing on early access only.

## 0.15.1 - 2026-03-17

### Improvements

- Improved performance of telemetry when using multiple processors.

### Fixes

- The scaling factor of the STFT now changes depending on the sample rate.

## 0.15.0 - 2026-02-27

### New features

- Support for V2 model files, which includes support for the new Quail Voice Focus 2.0 model.

### Improvements

- The parameters of Quail models are no longer fixed. The enhancement level of every model can now be adjusted between 0.0 and 1.0.

### Breaking Changes

- V1 model files are no longer supported.
- The error `AIC_ERROR_CODE_PARAMETER_FIXED` was removed.
- The parameter `AIC_PROCESSOR_PARAMETER_VOICE_GAIN` was removed.
- The parameter `AIC_VAD_PARAMETER_SPEECH_HOLD_DURATION` previously held detected speech for half of the specified duration. It has now been changed to better represent the intention of the developer.
- The default value for `AIC_VAD_PARAMETER_SPEECH_HOLD_DURATION` was changed from 50 ms to 30 ms to match the existing behavior.

### Fixes

- `aic_vad_context_set_parameter` no longer returns an error when trying to set a valid speech hold duration value before calling `aic_processor_initialize`.

## 0.14.0 - 2026-01-23

### Improvements

- Increased the maximum speech hold duration of the VAD from 20 to 100x the model's window size.

### Fixes

- Fixed an issue causing the VAD's state to be reset on every `aic_processor_process_*` call.

### Breaking changes

- `AIC_ERROR_CODE_MODEL_NOT_INITIALIZED` has been renamed to `AIC_ERROR_CODE_PROCESSOR_NOT_INITIALIZED`.

## 0.13.1 - 2026-01-15

### Fixes

- Fixed an issue allowing users to change processor parameters with certain models

## 0.13.0 - 2026-01-15

This release comes with a number of new features and several breaking changes. Most notably, the C library does no longer include any models, which significantly reduces the library's binary size. The models are now available separately for download at https://artifacts.ai-coustics.io.

**New license keys required**: License keys previously generated in the [developer portal](https://developers.ai-coustics.io) will no longer work. New license keys must be generated.

**Model naming changes**: Quail-STT models are now called "Quail" - These models are optimized for human-to-machine enhancement (e.g., Speech-to-Text (STT) applications). Quail models are now called "Sparrow" - These models are optimized for human-to-human enhancement (e.g., voice calls, conferencing). This naming change clarifies the distinction between STT-focused models and human-to-human communication model

**Major architectural changes**: The API has been restructured to separate model data from processing instances. What was previously called `AicModel` (which handled both model data and processing) has been split into:
- `AicModel`: Now represents only the ML model data loaded from files or memory
- `AicProcessor`: New type that performs the actual audio processing using a model
- Multiple processors can share the same model, allowing efficient resource usage across streams
- Model instances are reference-counted internally; `aic_model_destroy` can be called immediately after creating processors, and the model will be freed automatically when the last processor using it is destroyed
- To change parameters, reset the processor and get the output delay, a processor context must now be created via `aic_processor_context_create`. This context can be freely moved between threads

### New features

- Models now load from files via `aic_model_create_from_file`.
- Models can also be created from in-memory buffers with `aic_model_create_from_buffer`.
- Added new `aic_model_get_id` API to query the id of a model.
- A single model handle can be shared across multiple processors.
- Added processor handles with `aic_processor_create` so each stream can be initialized independently from a shared model while sharing weights.
- Added `aic_get_compatible_model_version` to query the required model version for this SDK.
- Added context-based APIs for thread-safe control operations:
    - `aic_processor_context_create` and `aic_processor_context_destroy` for processor context management
    - `aic_vad_context_create` and `aic_vad_context_destroy` for VAD context management
- Model query APIs moved to model handles:
    - `aic_model_get_optimal_sample_rate` - gets optimal sample rate for a model
    - `aic_model_get_optimal_num_frames` - gets optimal frame count for a model at given sample rate
- Added new error codes for model loading:
    - `AIC_ERROR_CODE_MODEL_INVALID`
    - `AIC_ERROR_CODE_MODEL_VERSION_UNSUPPORTED`
    - `AIC_ERROR_CODE_MODEL_FILE_PATH_INVALID`
    - `AIC_ERROR_CODE_FILE_SYSTEM_ERROR`
    - `AIC_ERROR_CODE_MODEL_DATA_UNALIGNED`

### Breaking changes

- License keys previously generated in the [development portal](developers.ai-coustics.io) will no longer work. New license keys have to be generated.
- Existing `aic_model_*` processing and configuration APIs have been renamed to `aic_processor_*`.
- Removed `AicModelType` enum; callers must supply a model file or aligned buffer instead of selecting a built-in model.
- License keys are now provided to `aic_processor_create` rather than model creation.
- Renamed `AicEnhancementParameter` to `AicProcessorParameter` (`AIC_PROCESSOR_PARAMETER_*`).
- VAD APIs now use `AicVadContext` handles and bind to processor handles instead of model handles:
    - `aic_vad_create` → `aic_vad_context_create` (takes `const AicProcessor*` instead of `AicModel*`)
    - `aic_vad_destroy` → `aic_vad_context_destroy`
    - `aic_vad_is_speech_detected` → `aic_vad_context_is_speech_detected` (takes `const AicVadContext*`)
    - `aic_vad_get_parameter` → `aic_vad_context_get_parameter` (takes `const AicVadContext*`)
    - `aic_vad_set_parameter` → `aic_vad_context_set_parameter` (takes `const AicVadContext*`)
- Processor control APIs now take `AicProcessorContext` handles created via `aic_processor_context_create`:
    - `aic_model_reset` → `aic_processor_context_reset` (takes `const AicProcessorContext*`)
    - `aic_model_get_parameter` → `aic_processor_context_get_parameter` (takes `const AicProcessorContext*`)
    - `aic_model_set_parameter` → `aic_processor_context_set_parameter` (takes `const AicProcessorContext*`)
    - `aic_get_output_delay` → `aic_processor_context_get_output_delay` (takes `const AicProcessorContext*`)
- Model query APIs moved to model methods:
    - `aic_get_optimal_sample_rate` → `aic_model_get_optimal_sample_rate`
    - `aic_get_optimal_num_frames` → `aic_model_get_optimal_num_frames`

### Fixes

- Improved thread safety.
- Fixed an issue where the allocated size for an FFT operation could be incorrect, leading to a crash.

## 0.12.0 - 2025-12-12

### New features

- Added new VAD parameter `AIC_VAD_PARAMETER_MINIMUM_SPEECH_DURATION` used to control for how long speech needs to be present in the audio signal before the VAD considers it speech.

### Breaking changes

- Replaced VAD parameter `AIC_VAD_PARAMETER_LOOKBACK_BUFFER_SIZE` with `AIC_VAD_PARAMETER_SPEECH_HOLD_DURATION`, used to control for how long the VAD continues to detect speech after the audio signal no longer contains speech.

## 0.11.0 - 2025-12-10

### New features

- Added new Quail Voice Focus STT model (`AIC_MODEL_TYPE_QUAIL_VF_STT_L16`), purpose-built to isolate and elevate the foreground speaker while suppressing both interfering speech and background noise.
- Added new variants of the Quail STT model: `AIC_MODEL_TYPE_QUAIL_STT_L8`, `AIC_MODEL_TYPE_QUAIL_STT_S16` and `AIC_MODEL_TYPE_QUAIL_STT_S8`.
- Added `aic_model_process_sequential` for sequential channel data in a single buffer

### Breaking changes

- `AIC_MODEL_TYPE_QUAIL_STT` was renamed to `AIC_MODEL_TYPE_QUAIL_STT_L16`
- `aic_vad_create` signature changed: the `model` parameter is no longer `const`

### Fixes

- VAD now works correctly when `AIC_ENHANCEMENT_PARAMETER_ENHANCEMENT_LEVEL` is set to 0 or `AIC_ENHANCEMENT_PARAMETER_BYPASS` is enabled (previously non-functional in these cases)

## 0.10.1 - 2025-12-03

### Fixes

- Remove internal library symbols that cause duplicated symbols linker errors when building Rust programs that have a dependency of the `ring` crate.

## 0.10.0 - 2025-11-20

### Features

- **Quail STT** (`AIC_MODEL_TYPE_QUAIL_STT`): Our newest speech enhancement model is optimized for human-to-machine interaction (e.g., voice agents, speech-to-text). This model operates at a native sample rate of 16 kHz and uses fixed enhancement parameters that cannot be changed during runtime. The model is also compatible with our VAD.

### Breaking Changes

- Removed **AIC_ENHANCEMENT_PARAMETER_NOISE_GATE_ENABLE** as it is now a fixed part of our VAD.
- Added new error code **AIC_ERROR_CODE_PARAMETER_FIXED** returned when attempting to modify a parameter of a model with fixed parameters.

### Fixes

- Fixed an issue where `aic_vad_is_speech_detected` always returned `true` when `AIC_VAD_PARAMETER_LOOKBACK_BUFFER_SIZE` was set to `1.0`.

## 0.9.1 - 2025-11-17

### Features

- **Internal library patching**: Static libraries are now patched internally to simplify usage from Rust, reducing integration complexity
- **Windows ARM64 support**: Added Windows ARM64 as a supported target platform

### Breaking Changes

- **Additional system library dependencies**: On macOS and Windows, the following system libraries must now be linked:
  - **Windows**: `Synchronization` and `bcryptprimitives`
  - **macOS**: `CoreFoundation`

## 0.9.0 - 2025-11-05

### Features

- **Voice Activity Detection**: This release adds a new Quail-based VAD. The VAD automatically uses the output of a Quail model to calculate a voice activity prediction.

**Added VAD functions**
```C
enum AicErrorCode aic_vad_create(struct AicVad **vad, const struct AicModel *model);
void aic_vad_destroy(struct AicVad *vad);
enum AicErrorCode aic_vad_is_speech_detected(struct AicVad *vad, bool *value);
enum AicErrorCode aic_vad_set_parameter(struct AicVad *vad,
                                        enum AicVadParameter parameter,
                                        float value);
enum AicErrorCode aic_vad_get_parameter(const struct AicVad *vad,
                                        enum AicVadParameter parameter,
                                        float *value);
```

**Added VAD parameters**
```C
typedef enum AicVadParameter {
  AIC_VAD_PARAMETER_LOOKBACK_BUFFER_SIZE = 0,
  AIC_VAD_PARAMETER_SENSITIVITY = 1,
} AicVadParameter;
```

### Breaking Changes

- `AicParameter` was renamed to `AicEnhancementParameter`.

**Old**
```C
typedef enum AicParameter {
  AIC_PARAMETER_BYPASS = 0,
  AIC_PARAMETER_ENHANCEMENT_LEVEL = 1,
  AIC_PARAMETER_VOICE_GAIN = 2,
  AIC_PARAMETER_NOISE_GATE_ENABLE = 3,
} AicParameter;
```

**New**
```C
typedef enum AicEnhancementParameter {
  AIC_ENHANCEMENT_PARAMETER_BYPASS = 0,
  AIC_ENHANCEMENT_PARAMETER_ENHANCEMENT_LEVEL = 1,
  AIC_ENHANCEMENT_PARAMETER_VOICE_GAIN = 2,
  AIC_ENHANCEMENT_PARAMETER_NOISE_GATE_ENABLE = 3,
} AicEnhancementParameter;
```

## 0.8.0 - 2025-10-27

### Features

- **Self-Service Licenses**: Starting with this release, you can use self-service licenses directly from our development portal.

- **Usage-Based Telemetry**: This release introduces a new telemetry feature that collects usage data, paving the way for future usage-based pricing models such as pay-per-minute billing.
  - **What we collect**: We collect only the processing time used and some diagnostic data
  - **Privacy**: We do not collect any information about your audio content. Your audio never leaves your device during our processing.
  - **Requirements**: Requires a constant internet connection. If the SDK cannot be activated online, enhancement will stop after 10 seconds. If telemetry data cannot be sent, enhancement will stop after 5 minutes. When enhancement is stopped an error will be returned, the audio will be bypassed and the processing delay will be still applied to ensure an uninterrupted audio stream without discontinuities.
  - **Error Handling**: When processing is bypassed because our backend cannot be reached or does not allow you to process, the process functions will return `AIC_ERROR_CODE_ENHANCEMENT_NOT_ALLOWED`. Make sure to handle this error code in your implementation.
  - **Offline Licenses**: If you cannot provide a constant internet connection, please contact us to obtain a special offline license that does not require telemetry.

### Breaking Changes

- **Updated Error Codes**: Renumbered and expanded error codes with additional license-related errors.

#### Old Error Codes

| Error Code | Value |
|---|---|
| `AIC_ERROR_CODE_SUCCESS` | 0 |
| `AIC_ERROR_CODE_NULL_POINTER` | 1 |
| `AIC_ERROR_CODE_LICENSE_INVALID` | 2 |
| `AIC_ERROR_CODE_LICENSE_EXPIRED` | 3 |
| `AIC_ERROR_CODE_UNSUPPORTED_AUDIO_CONFIG` | 4 |
| `AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH` | 5 |
| `AIC_ERROR_CODE_NOT_INITIALIZED` | 6 |
| `AIC_ERROR_CODE_PARAMETER_OUT_OF_RANGE` | 7 |
| `AIC_ERROR_CODE_SDK_ACTIVATION_ERROR` | 8 |

#### New Error Codes

| Error Code | Value | Notes |
|---|---|---|
| `AIC_ERROR_CODE_SUCCESS` | 0 | Unchanged |
| `AIC_ERROR_CODE_NULL_POINTER` | 1 | Unchanged |
| `AIC_ERROR_CODE_PARAMETER_OUT_OF_RANGE` | 2 | Renumbered from 7 |
| `AIC_ERROR_CODE_MODEL_NOT_INITIALIZED` | 3 | Renamed from `AIC_ERROR_CODE_NOT_INITIALIZED`, renumbered from 6 |
| `AIC_ERROR_CODE_AUDIO_CONFIG_UNSUPPORTED` | 4 | Renamed from `AIC_ERROR_CODE_UNSUPPORTED_AUDIO_CONFIG` |
| `AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH` | 5 | Unchanged |
| `AIC_ERROR_CODE_ENHANCEMENT_NOT_ALLOWED` | 6 | **New.** SDK key was not authorized or process failed to report usage. Check if you have internet connection. |
| `AIC_ERROR_CODE_INTERNAL_ERROR` | 7 | **New.** Internal error occurred. Contact support. |
| `AIC_ERROR_CODE_LICENSE_FORMAT_INVALID` | 50 | Renamed from `AIC_ERROR_CODE_LICENSE_INVALID`, renumbered from 2 |
| `AIC_ERROR_CODE_LICENSE_VERSION_UNSUPPORTED` | 51 | **New.** License version is not compatible with the SDK version. Update SDK or contact support. |
| `AIC_ERROR_CODE_LICENSE_EXPIRED` | 52 | Renumbered from 3 |

**Removed:** `AIC_ERROR_CODE_SDK_ACTIVATION_ERROR` has been removed and split into specific license errors.

### Fixes

- Fixed an issue where, after a successful initialization, a subsequent initialization error would not properly block processing, potentially allowing operations on a partially initialized model.
- Fixed an issue where toggling bypass mode or switching enhancement levels could produce discontinuities.

## 0.7.0 - 2025-10-14

### Breaking Changes

- **Variable number of frames supported**: The `aic_model_process` function now supports a variable number of frames per call. To enable this feature, use the new `allow_variable_frames` parameter in the `initialize` function:
    ```C
    enum AicErrorCode aic_model_initialize(struct AicModel *model,
                                        uint32_t sample_rate,
                                        uint16_t num_channels,
                                        size_t num_frames,
                                        bool allow_variable_frames);
    ```
  Set `allow_variable_frames` to `true` to enable variable frame processing, or `false` to maintain the previous fixed frame behavior. Note that enabling variable frames results in higher processing delay.
- **New bypass parameter**: A new parameter `AIC_PARAMETER_BYPASS` has been added to control audio processing bypass while preserving algorithmic delay. When enabled, the input audio passes through unmodified, but the      output is still delayed by the same amount as during normal processing. This ensures seamless transitions when toggling enhancement on/off without audible clicks or timing shifts.
- **Sample rate parameter added to `aic_get_optimal_num_frames`**: The function now takes `sample_rate` as an argument to make the dependency between sample rate and optimal frame count more explicit:
    ```C
    enum AicErrorCode aic_get_optimal_num_frames(const struct AicModel *model,
                                                 uint32_t sample_rate,
                                                 size_t *num_frames);
    ```

### Fixes

- **Model state reset during pause**: The internal model state is now automatically reset when processing is paused (e.g., when bypass is enabled or enhancement level is set to 0). This ensures a clean state when processing resumes.
- **`aic_model_reset` now resets all DSP components**: The reset operation now ensures that all internal DSP components are properly reset, providing a more thorough clean state.

## 0.6.3 - 2025-08-22

### Updates

- **Updated low-sample rate models**: 8- and 16 KHz Quail models updated with improved speech enhancement performance.

## 0.6.2 - 2025-08-19

### Bug Fixes

- **Fixed output delay issue**: Resolved reported output delay problems affecting `AIC_MODEL_QUAIL_XS` and `AIC_MODEL_QUAIL_XXS` models
- **Fixed audio quality degradation**: Corrected audio distortion that occurred when using enhancement levels below 1.0 with the affected models

## 0.6.1 - 2025-08-18

### Features
- A new resampling technique makes resampling much more robust without adding additional latency or performance overhead
- New models for different sample rates are now available (16 kHz and 8 kHz)
- The noise gate algorithm has been improved, which leads to better quality
- The output of `aic_get_optimal_num_frames` now changes with sample rate so you can always have the lowest output delay
- The model reset now works as expected and resets the internal model state correctly

### Breaking Changes
- `AIC_MODEL_QUAIL_L` is now called `AIC_MODEL_QUAIL_L48`
- `AIC_MODEL_QUAIL_S` is now called `AIC_MODEL_QUAIL_S48`
- `get_library_version` is now called `aic_get_sdk_version`
- `aic_get_processing_latency` is now called `aic_get_output_delay`
- `AIC_PARAMETER_ENHANCEMENT_LEVEL_SKEW_FACTOR` has been removed because it led to confusion.
  The enhancement level is more predictable this way, and a value of 0.0 is always a bypass.
  If you want to skew the enhancement slider, this has to be done on your end.
