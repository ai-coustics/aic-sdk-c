## Breaking Changes

### New model file version

This release requires model file version 7. Re-download your models so the SDK does not reject them with `AIC_ERROR_CODE_MODEL_VERSION_UNSUPPORTED`. See the [compatibility matrix](https://docs.ai-coustics.com/reference/sdk/compatibility-matrix).

## New Features

### Tyto 1.0 has been replaced by Tyto 1.1

The analysis model is now Tyto 1.1, `tyto-1.1-l-16khz`. Tyto 1.0 (`tyto-l-16khz`) is not
loadable by this SDK version any more.

### Analysis result fields changed

`AicAnalysisResult` gained a field and lost one:

- Added: `codec_degradation`, a measure of artifacts introduced by lossy speech codecs.
- Removed: `media_speech`.

## Bug Fixes

- `aic_analyzer_analyze_buffered` no longer crashes when OpenTelemetry reporting is enabled.
