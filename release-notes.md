## Features

The SDK now supports observability with [OpenTelemetry](https://opentelemetry.io/).
See the [documentation](https://docs.ai-coustics.com/guides/observability) for the complete setup guide.
`aic_processor_create` takes a new `otel_config` parameter with two usage options:

**Environment variable**: pass `NULL` and set `AIC_SDK_OTEL_ENABLE=1` to enable telemetry globally:

```c
aic_processor_create(&processor, model, license_key, NULL);
```

**Per-session**: pass an `AicOtelConfig` to enable telemetry and associate it with a specific session ID:

```c
typedef struct AicOtelConfig {
  bool enable;
  const char *session_id; // NULL = auto-generate
} AicOtelConfig;

AicOtelConfig otel = { .enable = true, .session_id = "my-session-id" };
aic_processor_create(&processor, model, license_key, &otel);
```

## Breaking Changes

`aic_processor_create` has a new `otel_config` parameter. Existing call sites must be updated to pass `NULL` as the last argument.
