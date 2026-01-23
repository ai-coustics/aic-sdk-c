## Improvements

- Increased the maximum speech hold duration of the VAD from 20 to 100x the model's window size.

## Fixes

- Fixed an issue causing the VAD's state to be reset on every `aic_processor_process_*` call.

## Breaking changes

- `AIC_ERROR_CODE_MODEL_NOT_INITIALIZED` has been renamed to `AIC_ERROR_CODE_PROCESSOR_NOT_INITIALIZED`.
