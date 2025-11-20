## Features

- **Quail STT** (`AIC_MODEL_TYPE_QUAIL_STT`): Our newest speech enhancement model is optimized for human-to-machine interaction (e.g., voice agents, speech-to-text). This model operates at a native sample rate of 16 kHz and uses fixed enhancement parameters that cannot be changed during runtime. The model is also compatible with our VAD.

## Breaking Changes

- Removed **AIC_ENHANCEMENT_PARAMETER_NOISE_GATE_ENABLE** as it is now a fixed part of our VAD.
- Added new error code **AIC_ERROR_CODE_PARAMETER_FIXED** returned when attempting to modify a parameter of a model with fixed parameters.

## Fixes

- Fixed an issue where `aic_vad_is_speech_detected` always returned `true` when `AIC_VAD_PARAMETER_LOOKBACK_BUFFER_SIZE` was set to `1.0`.
