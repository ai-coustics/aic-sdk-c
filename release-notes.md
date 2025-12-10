## New features

- Added new Quail Voice Focus STT model (`AIC_MODEL_TYPE_QUAIL_VF_STT_L16`), purpose-built to isolate and elevate the foreground speaker while suppressing both interfering speech and background noise.
- Added new variants of the Quail STT model: `AIC_MODEL_TYPE_QUAIL_STT_L8`, `AIC_MODEL_TYPE_QUAIL_STT_S16` and `AIC_MODEL_TYPE_QUAIL_STT_S8`.
- Added `aic_model_process_sequential` for sequential channel data in a single buffer

## Breaking changes

- `AIC_MODEL_TYPE_QUAIL_STT` was renamed to `AIC_MODEL_TYPE_QUAIL_STT_L16`
- `aic_vad_create` signature changed: the `model` parameter is no longer `const`

## Fixes

- VAD now works correctly when `AIC_ENHANCEMENT_PARAMETER_ENHANCEMENT_LEVEL` is set to 0 or `AIC_ENHANCEMENT_PARAMETER_BYPASS` is enabled (previously non-functional in these cases)
