## Features

- **Voice Activity Detection**: This release adds a new Quail-based VAD. The VAD automatically uses the output of a Quail model to calculate a voice activity prediction.

**Added VAD functions**
```
enum AicErrorCode aic_vad_create(struct AicVad **vad, const struct AicModel *model);
void aic_vad_destroy(struct AicVad *vad);
enum AicErrorCode aic_vad_is_speech_detected(struct AicVad *vad, bool *value);
enum AicErrorCode aic_vad_set_parameter(struct AicVad *vad,
                                        enum AicVadParameter parameter,
                                        float value);
enum AicErrorCode aic_vad_get_parameter(const struct AicVad *vad,
                                        enum AicVadParameter parameter,
                                        float *value);
```

**Added VAD parameters**
```
typedef enum AicVadParameter {
  AIC_VAD_PARAMETER_LOOKBACK_BUFFER_SIZE = 0,
  AIC_VAD_PARAMETER_SENSITIVITY = 1,
} AicVadParameter;
```

## Breaking Changes

- `AicParameter` was renamed to `AicEnhancementParameter`.

**Old**
```
typedef enum AicParameter {
  AIC_PARAMETER_BYPASS = 0,
  AIC_PARAMETER_ENHANCEMENT_LEVEL = 1,
  AIC_PARAMETER_VOICE_GAIN = 2,
  AIC_PARAMETER_NOISE_GATE_ENABLE = 3,
} AicParameter;
```

**New**
```
typedef enum AicEnhancementParameter {
  AIC_ENHANCEMENT_PARAMETER_BYPASS = 0,
  AIC_ENHANCEMENT_PARAMETER_ENHANCEMENT_LEVEL = 1,
  AIC_ENHANCEMENT_PARAMETER_VOICE_GAIN = 2,
  AIC_ENHANCEMENT_PARAMETER_NOISE_GATE_ENABLE = 3,
} AicEnhancementParameter;
``` 

