This release comes with a number of new features and several breaking changes. Most notably, the C library does no longer include any models, which significantly reduces the library's binary size. The models are now available separately for download at https://artifacts.ai-coustics.io.

**New license keys required**: License keys previously generated in the [developer portal](https://developers.ai-coustics.io) will no longer work. New license keys must be generated.

**Model naming changes**: Quail-STT models are now called "Quail" - These models are optimized for human-to-machine enhancement (e.g., Speech-to-Text (STT) applications). Quail models are now called "Sparrow" - These models are optimized for human-to-human enhancement (e.g., voice calls, conferencing). This naming change clarifies the distinction between STT-focused models and human-to-human communication model

**Major architectural changes**: The API has been restructured to separate model data from processing instances. What was previously called `AicModel` (which handled both model data and processing) has been split into:
- `AicModel`: Now represents only the ML model data loaded from files or memory
- `AicProcessor`: New type that performs the actual audio processing using a model
- Multiple processors can share the same model, allowing efficient resource usage across streams
- Model instances are reference-counted internally; `aic_model_destroy` can be called immediately after creating processors, and the model will be freed automatically when the last processor using it is destroyed
- To change parameters, reset the processor and get the output delay, a processor context must now be created via `aic_processor_context_create`. This context can be freely moved between threads

## New features

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

## Breaking changes

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

## Fixes

- Improved thread safety.
- Fixed an issue where the allocated size for an FFT operation could be incorrect, leading to a crash.
