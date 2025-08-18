### Features
- A new resampling technique makes resampling much more robust without adding additional latency or performance overhead
- New models for different sample rates are now available (16 kHz and 8 kHz)
- The noise gate algorithm has been improved, which leads to better quality
- The output of `aic_get_optimal_num_frames` now changes with sample rate so you can always have the lowest output delay
- The model reset now works as expected and resets the internal model state correctly

## Breaking Changes
- `AIC_MODEL_QUAIL_L` is now called `AIC_MODEL_QUAIL_L48`
- `AIC_MODEL_QUAIL_S` is now called `AIC_MODEL_QUAIL_S48`
- `get_library_version` is now called `aic_get_sdk_version`
- `aic_get_processing_latency` is now called `aic_get_output_delay`
- `AIC_PARAMETER_ENHANCEMENT_LEVEL_SKEW_FACTOR` has been removed because it led to confusion.
  The enhancement level is more predictable this way, and a value of 0.0 is always a bypass.
  If you want to skew the enhancement slider, this has to be done on your end.
