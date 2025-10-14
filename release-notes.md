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
