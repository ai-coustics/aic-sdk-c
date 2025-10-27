## Features

- **Self-Service Licenses**: Starting with this release, you can use self-service licenses directly from our development portal.

- **Usage-Based Telemetry**: This release introduces a new telemetry feature that collects usage data, paving the way for future usage-based pricing models such as pay-per-minute billing.
  - **What we collect**: We collect only the processing time used and some diagnostic data
  - **Privacy**: We do not collect any information about your audio content. Your audio never leaves your device during our processing.
  - **Requirements**: Requires a constant internet connection. If the SDK cannot be activated online, enhancement will stop after 10 seconds. If telemetry data cannot be sent, enhancement will stop after 5 minutes. When enhancement is stopped an error will be returned, the audio will be bypassed and the processing delay will be still applied to ensure an uninterrupted audio stream without discontinuities.
  - **Error Handling**: When processing is bypassed because our backend cannot be reached or does not allow you to process, the process functions will return `AIC_ERROR_CODE_ENHANCEMENT_NOT_ALLOWED`. Make sure to handle this error code in your implementation.
  - **Offline Licenses**: If you cannot provide a constant internet connection, please contact us to obtain a special offline license that does not require telemetry.

## Breaking Changes

- **Updated Error Codes**: Renumbered and expanded error codes with additional license-related errors.

### Old Error Codes

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

### New Error Codes

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

## Fixes

- Fixed an issue where, after a successful initialization, a subsequent initialization error would not properly block processing, potentially allowing operations on a partially initialized model.
- Fixed an issue where toggling bypass mode or switching enhancement levels could produce discontinuities.
