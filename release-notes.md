## New features

- Added new VAD parameter `AIC_VAD_PARAMETER_MINIMUM_SPEECH_DURATION` used to control for how long speech needs to be present
in the audio signal before the VAD considers it speech.

## Breaking changes

- Replaced VAD parameter `AIC_VAD_PARAMETER_LOOKBACK_BUFFER_SIZE` with `AIC_VAD_PARAMETER_SPEECH_HOLD_DURATION`, used to control
for how long the VAD continues to detect speech after the audio signal no longer contains speech.
