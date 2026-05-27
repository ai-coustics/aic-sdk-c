## Features

### JWT token support

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

### Voice activity detection models

VAD is no longer limited to the energy-based detector. The SDK now also
supports dedicated VAD models, which output a per-buffer speech probability.
`AIC_VAD_PARAMETER_SENSITIVITY` is interpreted differently depending on the
active model:

- VAD models: range `0.0` to `1.0`; the probability threshold above which
  speech is reported.
- Energy-based VADs: range `1.0` to `15.0`; energy threshold =
  `10 ^ (-sensitivity)`.

The default sensitivity is now model-specific.

### Configurable OpenTelemetry export interval

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
